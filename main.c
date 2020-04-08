#include <stdio.h>
#include <ncurses.h>
#include "process.h"

int main()
{
	get_processes_info();
//	initscr();
	printf("Hello, World!\n");
//	getch();
//	endwin();
	return 0;
}