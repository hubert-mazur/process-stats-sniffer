//
// Created by hubert on 07.04.20.
//

#ifndef PROCESS_STATS_SNIFFER_PROCESS_H
#define PROCESS_STATS_SNIFFER_PROCESS_H

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

DIR *get_processes_dir();


typedef enum boolean { True = 1, False = 0 } boolean;

typedef struct process {
	pid_t process_id;
	pid_t parent_process_id;
	char *command;
	char *status;
	long vsize;
	long resident_set_size;
	long current_rss_soft_limit;
	char *current_security_attribute;
	char *chars_read;
	char *chars_written;
	char *number_of_read_operations;
	char *number_of_write_operations;
	char *bytes_read;
	char *bytes_written;
} process;

char **get_processes_dirs_list();

process *get_processes_info();

#endif //PROCESS_STATS_SNIFFER_PROCESS_H
