#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "futils.h"

static inline void print_ext_2(int a, int b)
{
	putchar(a);
	putchar(b);
	putchar(' ');
}

static int print_missing_parts_2(void)
{
	int a,b;
	int count, exists;
	char filename[1024];

	count = 0;
	for(a=97; a<=122; a++) {
		for(b=97; b<=122; b++) {
			snprintf(filename, sizeof(filename), "%s.%c%c", "mpartfile", a, b);
			exists = is_regfile(filename, 0);
			if(exists == -1) { count++; print_ext_2(a, b); }
		}
	}
	return count;
}

static inline void print_ext_3(int a, int b, int c)
{
	putchar(a);
	putchar(b);
	putchar(c);
	putchar(' ');
}

static int print_missing_parts_3(void)
{
	int a,b,c;
	int count, exists;
	char filename[1024];

	count = 0;
	for(a=97; a<=122; a++) {
		for(b=97; b<=122; b++) {
			for(c=97; c<=122; c++) {
				snprintf(filename, sizeof(filename), "%s.%c%c%c", "mpartfile", a, b, c);
				exists = is_regfile(filename, 0);
				if(exists == -1) { count++; print_ext_3(a, b, c); }
			}
		}
	}
	return count;
}

static void usage(char *basename)
{
	fprintf(stderr, "%s: <2|3>\n", basename);
	exit(1);
}

int main(int argc, char *argv[])
{
	int type, count;

	if(argc != 2) { usage(argv[0]); }

	type = atoi(argv[1]);
	if(type == 2) { count = print_missing_parts_2(); }
	else if(type == 3) { count = print_missing_parts_3(); }
	else { usage(argv[0]); }

	if(count == 0) { printf("No missing peices!"); }

	putchar('\n');

	return 0;
}
