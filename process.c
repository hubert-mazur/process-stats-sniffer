//
// Created by hubert on 07.04.20.
//

#include "process.h"

static int processes_count = 0;
conditions limits;

/**
 * Function that opens a directory, where processes are stored - /proc
 * In case of failure prints error and exits
 * @return DIR directory
 */
DIR *get_processes_dir()
{
	char dir[] = "/proc/";
	DIR *directory = opendir(dir);
	if (!directory)
	{
		char err_msg[100];
		sprintf(err_msg, "Can't open directory /proc/");
		perror(err_msg);
		exit(-1);
	}
	return directory;
}

/**
 * Function that checks whether given directory name  is directory
 * Returns logical value
 * @param string name of directory
 * @return boolean
 */
boolean is_dir_process_dir(char *string)
{
	char *c = string;
	while (*c)
	{
		if (!isdigit(*c))
			return False;
		c++;
	}
	return True;
}

/**
 * Function which iterates through /proc directory to find all process directories
 * Returns 2D array of chars - names of process directories
 * @return char**
 */
char **get_processes_dirs_list()
{
	processes_count = 0;
	DIR *dir = get_processes_dir();
	char **processes = NULL;
	struct dirent *current_dir;
	char name_buffer[200];
	char buff[100];
	while ((current_dir = readdir(dir)))
	{
		memcpy(buff, current_dir->d_name, current_dir->d_reclen * sizeof(char));
		buff[current_dir->d_reclen] = 0;
		if (is_dir_process_dir(buff))
		{
			sprintf(name_buffer, "/proc/%s/", buff);
			processes = (char **) realloc(processes, (processes_count + 1) * sizeof(char *));
			*(processes + processes_count) = (char *) malloc(current_dir->d_reclen * sizeof(char) + 7);
			memcpy(*(processes + processes_count), name_buffer, current_dir->d_reclen * sizeof(char) + 7);
			processes_count++;
		}
	}
	closedir(dir);
	return processes;
}

/**
 * Function that reads processes files to fill statistics tables.
 * Files read: stat, statm, io
 * Uses locks to prevent io operations on these files for time of reading
 * Returns struct process with full information about process
 * @return process
 */
process *get_processes_info()
{
	static const char *statistics_files[] = {"stat", "statm", "io"};
	char **processes_list = get_processes_dirs_list();
	process *info = (process *) malloc(processes_count * sizeof(process));
	FILE *stat;
	FILE *statm;
	FILE *io;
	char fn_stat[100];
	char fn_statm[100];
	char fn_io[100];

	for (int i = 0; i < processes_count; i++)
	{
		sprintf(fn_stat, "%s%s", *(processes_list + i), statistics_files[0]);
		sprintf(fn_statm, "%s%s", *(processes_list + i), statistics_files[1]);
		sprintf(fn_io, "%s%s", *(processes_list + i), statistics_files[2]);

		int fd_stat = open(fn_stat, O_RDONLY);
		int fd_statm = open(fn_statm, O_RDONLY);
		int fd_io = open(fn_io, O_RDONLY);

		if (fd_io < 0 || fd_stat < 0 || fd_statm < 0)
		{
			info[i].command = NULL;
			continue;
		}

		flock(fd_stat, LOCK_EX);
		flock(fd_statm, LOCK_EX);
		flock(fd_io, LOCK_EX);

		stat = fdopen(fd_stat, "r");
		statm = fdopen(fd_statm, "r");
		io = fdopen(fd_io, "r");

		char buffer[10000] = {'\0'};
		get_file_content(buffer, stat);
		int size = 0;
		char **res = split_str(buffer, ' ', &size);

		info[i].pid = strtol(res[0], NULL, 0);
		info[i].ppid = strtol(res[3], NULL, 0);
		info[i].command = (char *) malloc(strlen(res[1]) + 1);
		strcpy(info[i].command, res[1]);
		info[i].status = state_rewrite(*res[2]);
		info[i].rss = strtoull(res[23], NULL, 0);
		info[i].vsize = strtoul(res[22], NULL, 0);
		info[i].utime = strtoul(res[13], NULL, 0) / sysconf(_SC_CLK_TCK);
		info[i].stime = strtoul(res[14], NULL, 0) / sysconf(_SC_CLK_TCK);
		info[i].nice = strtol(res[18], NULL, 0);

		free_string_array(res, size);
		get_file_content(buffer, statm);
		size = 0;
		res = split_str(buffer, ' ', &size);

		info[i].shared = strtoull(res[2], NULL, 0);
		info[i].size = strtoull(res[0], NULL, 0);

		free_string_array(res, size);

		get_file_content(buffer, io);
		size = 0;
		res = split_str(buffer, ' ', &size);
		info[i].chars_read = strtoull(res[1], NULL, 0);
		info[i].chars_written = strtoull(res[3], NULL, 0);
		info[i].bytes_read = strtoull(res[9], NULL, 0);
		info[i].bytes_written = strtoull(res[11], NULL, 0);

		free_string_array(res, size);

		fclose(stat);
		fclose(statm);
		fclose(io);

		flock(fd_stat, LOCK_UN);
		flock(fd_statm, LOCK_UN);
		flock(fd_io, LOCK_UN);
	}

	free_string_array(processes_list, processes_count);

	return info;
}

/**
 * Function for string splitting with given delimiter
 * @param string string to split
 * @param delimiter delimiter used for splitting
 * @param size number of strings after split
 * @return
 */
char **split_str(char *string, const char delimiter, int *size)
{
	char **string_arr = NULL;
	char *pointer = string;
	char buffer[1000] = {'\0'};
	short counter = 0;
	short tokens = 0;

	while (True)
	{
		if (*pointer == delimiter || *pointer == '\0')
		{
			buffer[strlen(buffer)] = '\0';
			string_arr = (char **) realloc(string_arr, (tokens + 1) * sizeof(char *));
			string_arr[tokens] = (char *) malloc(strlen(buffer) + 2);
			strcpy(string_arr[tokens], buffer);
			for (int i = 0; i < 1000; i++)
				buffer[i] = '\0';
			tokens++;
			(*size)++;
			counter = 0;
			if (*pointer == '\0')
				break;
			else
			{
				pointer++;
				continue;
			}
		}

		buffer[counter] = *pointer;
		counter++;
		pointer++;
	}
	return string_arr;
}

/**
 *
 * @param c character indicating state of process
 * @return returns one of process state
 */
process_state state_rewrite(char c)
{
	switch (c)
	{
		case 'R':
			return Running;
		case 'S':
			return Sleeping;
		case 'D':
			return Waiting;
		case 'Z':
			return Zombie;
		case 'T':
			return Stopped;
		case 't':
			return Tracing_stop;
		case 'X':
			return Dead;
		case 'x':
			return Dead;
		case 'K':
			return Wakekill;
		case 'W':
			return Waking;
		case 'P':
			return Parked;
		case 'I':
			return Idle;
		default:
			return Idle;
	}
}

/**
 * Function that frees unused information about process
 * @param s pointer to array of type struct process
 */
void free_process_fields_mem(struct process *s)
{
	for (int i = 0; i < processes_count; i++)
	{
		if (s[i].command != NULL)
			free(s[i].command);
	}
}

/**
 * Function that reads data from file to buffer
 * Uses flockfile for proper io operations
 * @param buffer char array, storage for file content
 * @param file pointer to opened file
 */
void get_file_content(char *buffer, FILE *file)
{
	int counter = 0;
	flockfile(file);

	while (!feof(file))
	{
		buffer[counter] = (char) fgetc(file);
		counter++;
	}
	buffer[strlen(buffer) - 1] = '\0';
	funlockfile(file);
}

/**
 * Function tht frees 2D char array
 * @param arr pointer to 2D char array
 * @param size size of array to free
 */
void free_string_array(char **arr, int size)
{
	for (int j = 0; j < size; j++)
		free(*(arr + j));
	free(arr);
}

/**
 * Function that initializes limits
 */
void init_limits()
{
	limits.b_read = ULLONG_MAX;
	limits.b_written = ULLONG_MAX;
	limits.ch_read_limit = ULLONG_MAX;
	limits.ch_written_limit = ULLONG_MAX;
	limits.metric_prefix = 1;
	limits.refresh_freq = 1;
	strcpy(limits.sort_by, "pid");
	limits.rss_limit = ULLONG_MAX;
	limits.shared_limit = ULLONG_MAX;
	limits.size_limit = ULLONG_MAX;
	limits.stime_limit = ULLONG_MAX;
	limits.utime_limit = ULLONG_MAX;
	limits.vsize_limit = ULLONG_MAX;
	limits.human_readable = False;
}

/**
 * Function that reads input parameters
 * @param argc number of arguments
 * @param argv 2D char array of arguments
 */
void read_parameters(int argc, char **argv)
{
	init_limits();

	for (int i = 1; i < argc; i++)
	{
		int size = 0;
		char **key = split_str(argv[i], '=', &size);
		if (size > 2)
		{
			perror("Syntax error!");
			exit(-1);
		}

		if (strcmp(key[0], "--rsslim") == 0)
		{
			limits.metric_prefix = set_prefix(key[1][strlen(key[1]) - 1]);
			key[1][strlen(key[1]) - 1] = '\0';
			limits.rss_limit = strtoull(key[1], NULL, 0);
		}
		else if (strcmp(key[0], "--vsize") == 0)
		{
			limits.metric_prefix = set_prefix(key[1][strlen(key[1]) - 1]);
			key[1][strlen(key[1]) - 1] = '\0';
			limits.vsize_limit = strtoull(key[1], NULL, 0);
		}
		else if (strcmp(key[0], "--size") == 0)
		{
			limits.metric_prefix = set_prefix(key[1][strlen(key[1]) - 1]);
			key[1][strlen(key[1]) - 1] = '\0';
			limits.size_limit = strtoull(key[1], NULL, 0);
		}
		else if (strcmp(key[0], "--shared") == 0)
		{
			limits.metric_prefix = set_prefix(key[1][strlen(key[1]) - 1]);
			key[1][strlen(key[1]) - 1] = '\0';
			limits.shared_limit = strtoull(key[1], NULL, 0);
		}
		else if (strcmp(key[0], "--stime") == 0)
		{
			key[1][strlen(key[1]) - 1] = '\0';
			limits.stime_limit = strtoull(key[1], NULL, 0);
		}
		else if (strcmp(key[0], "--utime") == 0)
		{
			key[1][strlen(key[1]) - 1] = '\0';
			limits.utime_limit = strtoull(key[1], NULL, 0);
		}
		else if (strcmp(key[0], "-h") == 0)
		{
			limits.human_readable = True;
		}
		else if (strcmp(key[0], "--sortby") == 0)
		{
			strcpy(limits.sort_by, key[1]);
		}
		else if (strcmp(key[0], "--watch") == 0)
		{
			limits.refresh_freq = (int) strtol(key[1], NULL, 0);
		}
		else
		{
			perror("Unsupported option in __FILE__ at __LINE\n");
			exit(-1);
		}

		for (int j = 0; j < size; j++)
			free(*(key + j));
		free(key);
	}

}

/**
 * Function that sets proper metric prefix
 * @param c character with given prefix
 * @return
 */
prefix set_prefix(char c)
{
	if (isdigit(c))
		return D;

	switch (c)
	{
		case 'k':
			return k;
		case 'M':
			return M;
		case 'G':
			return G;
		default:
		{
			perror("Unsupported metric prefix!\n");
			exit(-1);
		}
	}
}

/**
 * Function that prints out to screen overall number of processes, and their state-classification
 * @param p pointer to array of type struct process
 * @return
 */
char *print_header(process *p)
{
	int working = 0;
	int sleeping = 0;
	int idle = 0;
	int zombie = 0;
	int stopped = 0;

	for (int i = 0; i < processes_count; i++)
	{
		if (p[i].command == NULL)
			continue;

		switch (p[i].status)
		{
			case Running:
				working++;
				break;
			case Sleeping:
				sleeping++;
				break;
			case Idle:
				idle++;
				break;
			case Zombie:
				zombie++;
				break;
			case Stopped:
				stopped++;
				break;
			default:
				break;
		}
	}
	char *buffer = malloc(10000 * sizeof(char));
	sprintf(buffer, "All: %d, running: %d, sleeping: %d, zombie: %d, stopped: %d, idle: %d\n", processes_count, working,
			sleeping, zombie, stopped, idle);
	return buffer;
}

/**
 * function that sets refresh frequency
 * NOT IN USE
 * @return int refresh frequency in seconds
 */
int get_refresh_freq()
{
	return limits.refresh_freq;
}

/**
 * Function that prints to screen process information
 * Checks whether process is exceeding any limit, if so lists it on red
 * @param p pointer to array of type struct process
 */
void list_process_info(process *p)
{
	wattron(window, COLOR_PAIR(HEADER_COLOR));
	wprintw(window, "%5s %5s %10s %10s %10s %10s %10s %10s %10s %10s\n", "PID", "PPID", "vSize", "RSS",
			"uTime",
			"sTime", "size", "shared", "status", "command");
	wattroff(window, COLOR_PAIR(HEADER_COLOR));
	qsort_r(p, processes_count, sizeof(struct process), comparator, limits.sort_by);

	for (int i = 0; i < processes_count; i++)
	{
		if (p[i].command == NULL)
			continue;
		if (check_exceeding_limit(p + i))
			wattron(window, COLOR_PAIR(EXCEED_LIM_COLOR));
		if (limits.human_readable)
		{
			p[i].vsize /= M;
			p[i].rss /= M;
			p[i].size /= M;
			p[i].shared /= M;
			wprintw(window, "%5d %5d %10luM %10lluM %10lu %10lu %10lluM %10lluM %10s %10s\n", p[i].pid, p[i].ppid,
					p[i].vsize,
					p[i].rss,
					p[i].utime,
					p[i].stime, p[i].size,
					p[i].shared, get_state_name(p[i].status), p[i].command);
		}
		else
			wprintw(window, "%5d %5d %10lu %10llu %10lu %10lu %10llu %10llu %10s %10s\n", p[i].pid, p[i].ppid,
					p[i].vsize,
					p[i].rss,
					p[i].utime,
					p[i].stime, p[i].size,
					p[i].shared, get_state_name(p[i].status), p[i].command);

		wattroff(window, COLOR_PAIR(EXCEED_LIM_COLOR));
		wattron(window, COLOR_PAIR(NORMAL_LIM_COLOR));

	}
}

/**
 * Comparator used in qsort function for sorting processes
 * @param v1 first process to compare
 * @param v2 second process to compare
 * @param arg additional argument, tells which parameter to sort by
 * @return int order of processes
 */
int comparator(const void *v1, const void *v2, void *arg)
{
	process *pv_1 = (process *) (v1);
	process *pv_2 = (process *) (v2);

	if (pv_1->command == NULL || pv_2->command == NULL)
		return 1;


	if (!strcmp(arg, "pid"))
	{
		if (pv_1->pid <= pv_2->pid)
			return -1;
		else
			return 1;
	}

	else if (!strcmp(arg, "ppid"))
	{
		if (pv_1->ppid <= pv_2->ppid)
			return -1;
		else
			return 1;
	}

	else if (!strcmp(arg, "vsize"))
	{
		if (pv_1->vsize >= pv_2->vsize)
			return -1;
		else
			return 1;
	}
	else if (!strcmp(arg, "rss"))
	{
		if (pv_1->rss >= pv_2->rss)
			return -1;
		else
			return 1;
	}
	else if (!strcmp(arg, "utime"))
	{
		if (pv_1->utime >= pv_2->utime)
			return -1;
		else
			return 1;
	}
	else if (!strcmp(arg, "stime"))
	{
		if (pv_1->stime >= pv_2->stime)
			return -1;
		else
			return 1;
	}
	else if (!strcmp(arg, "size"))
	{
		if (pv_1->size >= pv_2->size)
			return -1;
		else return 1;
	}
	else if (!strcmp(arg, "shared"))
	{
		if (pv_1->shared >= pv_2->shared)
			return -1;
		else
			return 1;
	}
}

/**
 * Function that checks, whether process is exceeding any of limits
 * @param p pointer to struct process
 * @return boolean logical value, True if excceding limit
 */
boolean check_exceeding_limit(process *p)
{
	if (
			p->vsize / limits.metric_prefix >= limits.vsize_limit ||
			p->rss / limits.metric_prefix >= limits.rss_limit ||
			p->utime / limits.metric_prefix >= limits.utime_limit ||
			p->stime / limits.metric_prefix >= limits.stime_limit ||
			p->size / limits.metric_prefix >= limits.size_limit ||
			p->size / limits.metric_prefix >= limits.shared_limit
			)
		return True;

	return False;
}

/**
 * Function that is responsible for screen scrolling
 * Run under thread
 * @param pad_pos pointer to current position in window
 */
void screen_scroll(int *pad_pos)
{
	int ch;
	while ((ch = wgetch(window)) != 'q')
	{
		switch (ch)
		{
			case 'w':
			{
				if (*pad_pos >= 0)
				{
					(*pad_pos)--;
				}
				wclear(window);
				prefresh(window, *pad_pos, 0, 0, 0, MAX_NUMBER_OF_LISTED_PROCESSES - 1, COLS);
				break;
			}
			case 's':
			{
				if (*pad_pos <= MAX_NUMBER_OF_LISTED_PROCESSES + 1)
				{
					(*pad_pos)++;
				}
				wclear(window);
				prefresh(window, *pad_pos, 0, 0, 0, MAX_NUMBER_OF_LISTED_PROCESSES - 1, COLS);
				break;
			}
			default:
				continue;
		}
	}
	pthread_mutex_lock(&mutex);
	ON_FLAG = False;
	pthread_mutex_unlock(&mutex);
}

/**
 * Function that prints out state of process in full, human readable name
 * @param state
 * @return pointer to character, name of current process state
 */
char *get_state_name(int state)
{
	switch (state)
	{
		case 0:
			return "Running";

		case 1:
			return "Sleeping";

		case 2:
			return "Waiting";

		case 3:
			return "Zombie";

		case 4:
			return "Stopped";

		case 5:
			return "Tracing_stop";

		case 6:
			return "Dead";

		case 7:
			return "Wakekill";

		case 8:
			return "Waking";

		case 9:
			return "Parked";

		case 10:
			return "Idle";

		default:
			perror("No such process state\n");
			break;
	}
}
