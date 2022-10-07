#include <string.h>
#include <unistd.h>

// Часть контекста процесса
extern /* const */char *optarg;
extern int opterr, optind, optopt;

int getopt(int argc, char * const argv[], const char *optstring)
{
	const char * s = argv[optind];
	if (s==NULL) return -1;
	if (s[0]!='-') return -1;
	optopt = s[0];
	if (optopt=='\0') return -1;
	if (s[2]=='\0'){ 
		optind++;
		if (optopt=='-') return -1;
		s = argv[optind];
		if (s==NULL || optind >= argc) return ':';
	} else 
		s+= 2;
	
	const char* p = strchr(optstring, optopt);
	if (p==NULL) return '?';// unrecognized option
	if (p[1]==':') {
		optind++;
		optarg = (char *)s;
	}
	
	return p[0];
}
#ifdef TEST_GETOPT
/* Tests
This code accepts any of the following as equivalent:
cmd −ao arg path path
cmd −a −o arg path path
cmd −o arg −a path path
cmd −a −o arg − − path path
cmd −a −oarg path path
cmd −aoarg path path
*/
/*! Parsing Command Line Options
The following code fragment shows how you might process the arguments for a utility that can
take the mutually-exclusive options a and b and the options f and o, both of which require
arguments: */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(int argc, char *argv[])
{
	int c;
	int bflg = 0, aflg = 0, errflg = 0;
	char *ifile;
	char *ofile;
	// . . .
	while ((c = getopt(argc, argv, ":abf:o:")) != -1) {
		switch(c) {
		case 'a':
			if (bflg)
				errflg++;
			else
				aflg++;
			break;
		case 'b':
			if (aflg)
				errflg++;
			else
				bflg++;
			break;
		case 'f':
			ifile = optarg;
			break;
		case 'o':
			ofile = optarg;
			break;
		case ':': /* -f or -o without operand */
			fprintf(stderr, "Option -%c requires an operand\n", optopt);
			errflg++;
			break;
		case '?':
			fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
			errflg++;
			break;
		}
	}
	if (errflg) {
		fprintf(stderr, "usage: . . . \n");
		exit(2);
	}
	for ( ; optind < argc; optind++) {
		if (access(argv[optind], R_OK)) {
		// . . .
		}
	}
}
#endif
