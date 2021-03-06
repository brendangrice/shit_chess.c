#include "argp.h"
/*
 * Very rushed and pale imitation of <argp.h> for portability
 * Made with a few assumptions on my part and most elements taken from <argp.c> and <argp.h>
 */

error_t 
argp_parse (const struct argp *_argp, int _argc, char **_argv, unsigned _flags, int *argv, void *_input) 
{
	// iterate through arguments and try to parse with _argp->parser
	char *str;
	struct argp_state _argp_state;
	_argp_state.argc = _argc;
	_argp_state.argv = _argv;
	_argp_state.input = _input;
	_argp_state.arg_num = 1; // only handling one at a time
	_argp_state.root_argp = _argp;

	for (int i = 1; i<_argc; i++) {
		_argp_state.next = i;
		if(_argv[i][0] == '-' && strlen(_argv[i])>1) { // some sort of argument
			if(_argv[i][1] == '-') { // some sort of argument string
				// iterate through and match with name
				str = _argv[i]+2; // str after --
				int match = 1;
				for (int j=0; _argp->options[j].name != NULL || _argp->options[j].key != 0; j++) {
					if (_argp->options[j].name != NULL && strcmp(str, _argp->options[j].name)==0) { // try to find a match
						_argp->parser( _argp->options[j].key, _argv[i+1], &_argp_state );
						match = 0;
					}
				}
				if (match) argp_usage(&_argp_state, stderr); // if no match was found
			} else { // else there's a flag
				int pos = 1;
				// while there are flags to parse
				while (_argv[i][pos]) _argp->parser( _argv[i][pos++], _argv[i+1], &_argp_state );
			}
		} else { //loose
			_argp->parser( ARGP_KEY_ARG, _argv[i], &_argp_state );
		}
		if (_argp_state.next>i) i = _argp_state.next;
	}
	return 0;
}

void
argp_usage (const struct argp_state *__restrict __state, FILE *s) 
{
	// Usage: {EXE} [-{FLAGS}] [--{ARGUMENT}]... {ARGS_DOC}
	const struct argp *__argp = __state->root_argp;
	fprintf(s, "Usage: %s [-", basename(__state->argv[0])); // print out just the program name
	for (int i=0; __argp->options[i].key != 0 || __argp->options[i].name != NULL; i++) 
		if (__argp->options[i].key>' ' && !__argp->options[i].arg) fputc(__argp->options[i].key, s); // list all the flags
	fputs("] ", s);
	for (int i=0; __argp->options[i].key != 0; i++) if (__argp->options[i].key>' ' && __argp->options[i].arg) fprintf(s, "[-%c %s] ", (char) __argp->options[i].key, __argp->options[i].arg); // list all the flags
	for (int i=0; __argp->options[i].name != NULL; i++) {
		if (__argp->options[i].arg) {
			fprintf(s, "[--%s=%s] ", __argp->options[i].name, __argp->options[i].arg);
		} else	fprintf(s, "[--%s] ", __argp->options[i].name); // list all the flags
	}
	fputs(__argp->args_doc, s);
	if (s==stderr) exit(1); // maybe a more descriptive error code could be used
	exit(0);
}

void 
argp_help (const struct argp_state *__restrict __state, FILE *s)
{	
	/* 
	 * Usage: {EXE} [OPTIONS...] {ARGS_DOC}
	 * {DOC}
	 * -{FLAG}, {ARGUMENT}, {ARGUMENT DESCRIPTION} // with tabs for spacing
	 * {\v split on DOC}
	 * Usage: chess [OPTIONS...] [FILE]
	 *
	 
	 * Usage: chess [OPTIONS...] [FILE]
	 * Plays Chess.
	 * With FILE try to interpret FILE as PGN. When FILE is - read standard input as PGN
	 * 
	 *   -s, --step                 Steps through the moves when viewing a PGN file
	 *   -o, --output=FILE          Output PGN notation to specified FILE instead of standard date notation
	 *   -?, --help                 Give this help list
	 *       --usage                Give a short usage message
	 *   -V, --version              Print program version
	 * 
	 * more docs
	 *
	 */
	const struct argp *__argp = __state->root_argp;
	char doc[strlen(__argp->doc)]; // string that can be manipulated
	strcpy(doc, __argp->doc);
	if (__state->argv[0][0]=='.'&&__state->argv[0][1]=='/') { // formatting
		fprintf(s, "Usage: %s [OPTIONS...] %s\n", __state->argv[0]+2, __argp->args_doc);
	} else 	fprintf(s, "Usage: %s [OPTIONS...] %s\n", __state->argv[0], __argp->args_doc);
	
	//split up doc
	
	char *secondhalf = strchr(doc, '\v');
	if (secondhalf!=NULL) (secondhalf++)[0] = '\0'; // find and replace '\v' to make two strings

	char key[4] = {'-', 0, ',', 0};
	char name[30];

	fputs(doc, s);
	fputc('\n',s);
	for (int i=0; __argp->options[i].name != NULL || __argp->options[i].key != 0; i++) {
		key[0] = '-';
		key[1] = __argp->options[i].key;
		if (key[1]<=' ') key[0] = 0; // proper spacing

		if (__argp->options[i].name != NULL) {
			name[0] = '-';
			name[1] = '-';
			strncpy(name+2, __argp->options[i].name, 20);
		} else	name[0]=0; // proper spacing

		if (__argp->options[i].arg!=NULL) { // --output=FILE
			strncat(name, "=", 2);
			strncat(name, __argp->options[i].arg, 9);
		}
		fprintf(s, "%5s %-23s%s\n", key, name, __argp->options[i].doc);	
	}
	fputc('\n',s);

	fputs(secondhalf, s);

	
	exit(0);
}

void
argp_version ()
{
	puts(argp_program_version);
	exit(0);
}
