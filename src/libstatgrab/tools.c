#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>

char *f_read_line(FILE *f, const char *string){
	/* Max line length. 8k should be more than enough */
	static char line[8192];

	while((fgets(line, sizeof(line), f))!=NULL){
		if(strncmp(string, line, strlen(string))==0){
			return line;
		}
	}

	return NULL;
}

char *get_string_match(char *line, regmatch_t *match){
	int len=match->rm_eo - match->rm_so;
	char *match_string=malloc(len+1);

	match_string=strncpy(match_string, line+match->rm_so, len);
	match_string[len]='\0';

	return match_string;
}

long long get_ll_match(char *line, regmatch_t *match){
	char *ptr;
	long long num;

	ptr=line+match->rm_so;
	num=atoll(ptr);

	return num;
}
