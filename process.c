//
// Created by hubert on 07.04.20.
//

#include "process.h"

static int processes_count = 0;
conditions limits;

DIR *get_processes_dir()
{
	char dir[] = "/proc/";
	DIR *directory = opendir(dir);
	if (!directory)
	{
		char err_msg[100];
		sprintf(err_msg, "Can't open directory %s, in __FILE__ at __LINE__", dir);
		perror(err_msg);
		exit(-1);
	}
	return directory;
}

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


char **get_processes_dirs_list()
{
	processes_count = 0;
	DIR *dir = get_processes_dir();
	char **processes = NULL;
	struct dirent *current_dir;
	char name_buffer[100];
	char buff[100];
	while ((current_dir = readdir(dir)))
	{
		memcpy(buff, current_dir->d_name, current_dir->d_reclen * sizeof(char));
		buff[current_dir->d_reclen] = 0;
		if (is_dir_process_dir(buff))
		{
			strcpy(name_buffer, "/proc/");
			strcat(name_buffer, buff);
			strcat(name_buffer, "/");
			processes = (char **) realloc(processes, (processes_count + 1) * sizeof(char *));
			*(processes + processes_count) = (char *) malloc(current_dir->d_reclen * sizeof(char) + 7);
			memcpy(*(processes + processes_count), name_buffer, current_dir->d_reclen * sizeof(char) + 7);
			processes_count++;
		}
	}
	closedir(dir);
	return processes;
}

process *get_processes_info()
{
	static const char *statistics_files[] = {"stat", "statm", "io"};
	char **processes_list = get_processes_dirs_list();
	process *info = (process *) malloc(processes_count * sizeof(process));

	FILE *stat;
	FILE *statm;
	FILE *io;
	char *fn_stat = NULL;
	char *fn_statm = NULL;
	char *fn_io = NULL;

	for (int i = 0; i < processes_count; i++)
	{
		fn_stat = (char *) malloc(strlen(*(processes_list + i)) + strlen(statistics_files[0]) + 1);
		strcpy(fn_stat, *(processes_list + i));
		strcat(fn_stat, statistics_files[0]);

		fn_statm = (char *) malloc(strlen(*(processes_list + i)) + strlen(statistics_files[1]) + 1);
		strcpy(fn_statm, *(processes_list + i));
		strcat(fn_statm, statistics_files[1]);

		fn_io = (char *) malloc(strlen(*(processes_list + i)) + strlen(statistics_files[2]) + 1);
		strcpy(fn_io, *(processes_list + i));
		strcat(fn_io, statistics_files[2]);

//		int stat_s = open(fn_stat, O_RDONLY);
//		if (stat_s < 0)
//		{
//			perror("Cannot open file! leaving");
//		}
//		flock(stat_s, LOCK_EX);
		// experimetnal !

		int fd_stat = open(fn_stat, O_RDONLY);
		int fd_statm = open(fn_statm, O_RDONLY);
		int fd_io = open(fn_io, O_RDONLY);

		if (fd_io < 0 || fd_stat < 0 || fd_statm < 0)
		{
			perror("fd < 0\n");
			continue;
		}

		flock(fd_stat, LOCK_EX);
		flock(fd_statm, LOCK_EX);
		flock(fd_io, LOCK_EX);


		stat = fdopen(fd_stat, "r");
		statm = fdopen(fd_statm, "r");
		io = fdopen(fd_io, "r");
		 //

//		stat = fopen(fn_stat, "r");
//		statm = fopen(fn_statm, "r");
//		io = fopen(fn_io, "r");

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

		free(fn_stat);
		free(fn_statm);
		free(fn_io);

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

char **split_str(char *string, const char delimiter, int *size)
{
	char **string_arr = NULL;
	char *pointer = string;
	char buffer[1000] = {'\0'};
	short counter = 0;
	short tokens = 0;

	while (True)
	{
//		printf("%c\n", *pointer);
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
		{
			printf("%c\n", c);
			perror("Unsupported process state!\n");
			return Idle; // DEBUG ONLY, TODO: IMPLEMENT PROPER STATE
//			exit(-1);
		}
	}
}

void free_process_fields_mem(struct process *s)
{
	for (int i = 0; i < processes_count; i++)
	{
		free(s[i].command);
	}
}

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

void free_string_array(char **arr, int size)
{
	for (int j = 0; j < size; j++)
		free(*(arr + j));
	free(arr);
}

void init_limits()
{
	limits.b_read = ULLONG_MAX;
	limits.b_written = ULLONG_MAX;
	limits.ch_read_limit = ULLONG_MAX;
	limits.ch_written_limit = ULLONG_MAX;
	limits.metric_prefix = 0;
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
		else if (strcmp(key[0], "h") == 0)
		{
			limits.human_readable = True;
		}
		else if (strcmp(key[0], "sortby") == 0)
		{
			strcpy(limits.sort_by, key[1]);
		}
		else if (strcmp(key[0], "watch") == 0)
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
			perror("Unsupported metric prefix in __FILE__ at __LINE__\n");
			exit(-1);
		}
	}
}

char* print_header(process *p)
{
	int working = 0;
	int sleeping = 0;
	int idle = 0;
	int zombie = 0;
	int stopped = 0;

	for (int i = 0; i < processes_count; i++)
	{
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
	char* buffer = malloc(10000 * sizeof(char));
	sprintf(buffer, "All: %d, running: %d, sleeping: %d, zombie: %d, stopped: %d, idle: %d\n", processes_count, working,
			sleeping, zombie, stopped, idle);
	return buffer;
}

int get_refresh_freq()
{

}
