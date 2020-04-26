//
// Created by hubert on 07.04.20.
//

#ifndef PROCESS_STATS_SNIFFER_PROCESS_H
#define PROCESS_STATS_SNIFFER_PROCESS_H

#include <stdio.h>
#include <ncurses.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <zconf.h>
#include <fcntl.h>
#include <sys/file.h>
#include <time.h>

typedef enum boolean { True = 1, False = 0 } boolean;

typedef enum prefix { k = 1000, M = 1000000, G = 1000000000, D = 1 } prefix;

typedef enum process_state {Running, Sleeping, Waiting, Zombie, Stopped, Tracing_stop, Dead, Wakekill, Waking, Parked, Idle} process_state;

typedef struct process {
	pid_t pid;
	pid_t ppid;
	unsigned long vsize;
	unsigned long long rss;
	unsigned long utime;
	unsigned long stime;
	unsigned long long size;
	unsigned long long shared;
	process_state status;
	char *command;
	unsigned long long chars_read;
	unsigned long long chars_written;
	unsigned long long bytes_read;
	unsigned long long bytes_written;
	long nice;
} process;

typedef struct conditions {
	unsigned long vsize_limit;
	unsigned long long rss_limit;
	unsigned long utime_limit;
	unsigned long stime_limit;
	unsigned long long size_limit;
	unsigned long long shared_limit;
	unsigned long long ch_read_limit;
	unsigned long long ch_written_limit;
	unsigned long long b_read;
	unsigned long long b_written;
	prefix metric_prefix;
	int refresh_freq;
	char sort_by[15];
	boolean human_readable;
} conditions;


char **get_processes_dirs_list();

char **split_str(char *string, char delimiter, int *size);

process_state state_rewrite(char c);

void free_process_fields_mem(struct process *s);

void get_file_content(char *buffer, FILE *file);

void free_string_array(char **arr, int size);

void read_parameters(int argc, char **argv);

void init_limits();

prefix set_prefix(char c);

process *get_processes_info();

DIR *get_processes_dir();

char* print_header(process *p);

int get_refresh_freq();

void list_process_info(process *p);


#endif //PROCESS_STATS_SNIFFER_PROCESS_H
