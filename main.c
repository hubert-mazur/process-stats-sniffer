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
	char *buff;

	while (True)
	{
		struct process *p = get_processes_info();
		buff = print_header(p);
		printw("%s\n", buff);
		refresh();
		free(buff);
		clear();
		free_process_fields_mem(p);
		free(p);
//		if (feof(stdin))
//			continue;
//		else
//		{
//			if (getc(stdin) == 'q')
//			break;
//		}
		printw("ON\n");
	}

	endwin();


	return 0;
}