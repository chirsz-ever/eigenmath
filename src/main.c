#include "defs.h"
#include <editline/readline.h>


int
main(int argc, char *argv[])
{
	char *line;

	clear();

	if (argc > 1)
		run_script(argv[1]);

	for (;;) {
		line = readline("? ");
		if (!line)
			break;
		add_history(line);
		run(line);
		free((void*)line);
	}

	return 0;
}

void
run_script(char *filename)
{
	int fd, n;
	char *buf;

	fd = open(filename, O_RDONLY, 0);

	if (fd == -1) {
		printf("cannot open %s\n", filename);
		exit(1);
	}

	// get file size

	n = lseek(fd, 0, SEEK_END);

	if (n == -1) {
		printf("lseek err\n");
		exit(1);
	}

	lseek(fd, 0, SEEK_SET);

	buf = malloc(n + 1);

	if (buf == NULL)
		malloc_kaput();

	if (read(fd, buf, n) != n) {
		printf("read err\n");
		exit(1);
	}

	close(fd);

	buf[n] = 0;
	run(buf);
	free(buf);
}

void
malloc_kaput(void)
{
	printf("malloc kaput\n");
	exit(1);
}

void
printstr(char *s)
{
	fputs(s, stdout);
}

void
printchar(int c)
{
	fputc(c, stdout);
}

void
eval_draw(void)
{
	push_symbol(NIL);
}

void
cmdisplay(void)
{
	display();
}
