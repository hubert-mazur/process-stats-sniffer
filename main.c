#include "process.h"

int main(int argc, char **argv)
{
	if (geteuid() != 0)
	{
		perror("Not enough privileges to run program, (Hint: are you root?) \n");
		exit(-1);
	}

	read_parameters(argc, argv);

//	initscr();
	char *buff;
	clock_t time;
	time = clock();
	unsigned counter = 0;
	while (True)
	{
		if (counter == 600)
			break;
		if ((clock() - time) < 1 * CLOCKS_PER_SEC)
			continue;
		struct process *p = get_processes_info();
		buff = print_header(p);
		printf("%s\n", buff);
		list_process_info(p);
		refresh();
		clear();
		free(buff);
		free_process_fields_mem(p);
		free(p);
		time = clock();
		printf("%d\n", counter);
		counter++;
//		break;
	}

	endwin();

	return 0;
}