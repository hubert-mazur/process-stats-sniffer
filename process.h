//
// Created by hubert on 07.04.20.
//

#ifndef PROCESS_STATS_SNIFFER_PROCESS_H
#define PROCESS_STATS_SNIFFER_PROCESS_H

#define _GNU_SOURCE
#define EXCEED_LIM_COLOR 1
#define NORMAL_LIM_COLOR 2
#define HEADER_COLOR 3
#define MAX_NUMBER_OF_LISTED_PROCESSES 1000

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

#include <pthread.h>

typedef enum boolean { True = 1, False = 0 } boolean;

typedef enum prefix { k = 1000, M = 1000000, G = 1000000000, D = 1 } prefix;

typedef enum process_state {
	Running,
	Sleeping,
	Waiting,
	Zombie,
	Stopped,
	Tracing_stop,
	Dead,
	Wakekill,
	Waking,
	Parked,
	Idle
} process_state;

typedef struct process {
	// process id
	pid_t pid;
	// process parent id
	pid_t ppid;
	// virtual memory size in  bytes
	unsigned long vsize;
	// resident set size in bytes
	unsigned long long rss;
	// time spent in user mode in seconds
	unsigned long utime;
	// time spent in kernel mode in seconds
	unsigned long stime;
	// size of program in bytes
	unsigned long long size;
	// shared memory in bytes
	unsigned long long shared;
	// process status
	process_state status;
	// command of process
	char *command;
	// characters read
	unsigned long long chars_read;
	// characters written
	unsigned long long chars_written;
	// bytes read
	unsigned long long bytes_read;
	// bytes written
	unsigned long long bytes_written;
	long nice;
} process;

typedef struct conditions {
	// limit for virtual memory in bytes
	unsigned long vsize_limit;
	// limit for rss in bytes
	unsigned long long rss_limit;
	// limit for time spent in user mode in seconds
	unsigned long utime_limit;
	// limit for time spent in kernel mode in seconds
	unsigned long stime_limit;
	// limit for size of program in bytes
	unsigned long long size_limit;
	// limit for shared memory in bytes
	unsigned long long shared_limit;
	// limit for read characters
	unsigned long long ch_read_limit;
	// limit for written characters
	unsigned long long ch_written_limit;
	// limit for read bytes
	unsigned long long b_read;
	// limit for written bytes
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

char *print_header(process *p);

int get_refresh_freq();

void list_process_info(process *p);

int comparator(const void *v1, const void *v2, void *arg);

boolean check_exceeding_limit(process *p);

void screen_scroll(int *pad_pos);

char *get_state_name(int state);


boolean volatile ON_FLAG;
WINDOW *window;
pthread_mutex_t mutex;


#endif //PROCESS_STATS_SNIFFER_PROCESS_H
