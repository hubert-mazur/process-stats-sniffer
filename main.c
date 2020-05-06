#include "process.h"

int main(int argc, char **argv)
{
	if (geteuid() != 0)
	{
		perror("Not enough privileges to run program, (Hint: are you root?) \n");
		exit(-1);
	}

	read_parameters(argc, argv);
	initscr();
	clear();
	keypad(stdscr, True);
	start_color();
	init_pair(EXCEED_LIM_COLOR, COLOR_RED, COLOR_BLACK);
	init_pair(NORMAL_LIM_COLOR, COLOR_WHITE, COLOR_BLACK);
	init_pair(HEADER_COLOR, COLOR_GREEN, COLOR_BLACK);

	window = newpad(MAX_NUMBER_OF_LISTED_PROCESSES, COLS);


	char *buff;
	volatile int pad_pos = 0;
	ON_FLAG = True;
	pthread_t screen_mg_thread;
	int thread_status = pthread_create(&screen_mg_thread, NULL, (void *(*)(void *)) screen_scroll, (void *) &pad_pos);
	if (thread_status != 0)
	{
		perror("Thread creation error\n");
		exit(-1);
	}
	while (ON_FLAG)
	{
		struct process *p = get_processes_info();
		buff = print_header(p);
		wattron(window, COLOR_PAIR(HEADER_COLOR));
		wprintw(window, "%s\n", buff);
		wattroff(window, COLOR_PAIR(HEADER_COLOR));
		list_process_info(p);
		prefresh(window, pad_pos, 0, 0, 0, LINES - 1, COLS);
		wclear(window);
		free(buff);
		free_process_fields_mem(p);
		free(p);

	}
	pthread_join(screen_mg_thread, NULL);
	endwin();
	return 0;
}