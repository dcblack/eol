/*************************************************************
 *	Echo2 - writes characters to standard error
 *
 *	Options:	-n	do not append newline
 *
 *************************************************************/

#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[])
{
	short nl = 1;

	/* Check for options */
	if (argc > 1 && strcmp(argv[1],"-n") == 0) {
		nl = 0;
		argc--;
		argv++;
	}

	/* Output the arguments */
	while (--argc) {
		fputs(*++argv,stderr);
		if (argc > 1) putc(' ',stderr);
	}

	/* Newline if allowed */
	if (nl) putc('\n',stderr);
 }
