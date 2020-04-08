//
// Created by hubert on 07.04.20.
//

#include "process.h"

static int processes_count = 0;

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

void free_processes_dirs_list(char **list)
{
	for (int i = 0; i < processes_count; i++)
		free(*(list + i));
	free(list);
}

char **get_processes_dirs_list()
{
	DIR *dir = get_processes_dir();
	char **processes = NULL;
	struct dirent *current_dir;
	char name_buffer[100];

	while ((current_dir = readdir(dir)))
	{
		memcpy(name_buffer, current_dir->d_name, current_dir->d_reclen * sizeof(char));
		name_buffer[current_dir->d_reclen] = 0;
		if (is_dir_process_dir(name_buffer))
		{
//			printf("%s\n", name_buffer);
			processes = (char **) realloc(processes, (processes_count + 1) * sizeof(char *));
			*(processes + processes_count) = (char *) malloc(current_dir->d_reclen * sizeof(char));
			memcpy(*(processes + processes_count), name_buffer, current_dir->d_reclen * sizeof(char));
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

	free_processes_dirs_list(processes_list);
	// TODO: implement information gathering
	free(info);
}
