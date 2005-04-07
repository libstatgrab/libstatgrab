#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <pwd.h>
#include "statgrab.h"

char usage(char *progname){
	fprintf(stderr," Usage: %s [-efAnh] [-u user] [-o sort]\n\n", progname);
	fprintf(stderr, "\t-h\tDisplays this help\n");
	fprintf(stderr, "\t-e\tAll processes\n");
	fprintf(stderr, "\t-f\tFull listing\n");
	fprintf(stderr, "\t-a\tAll processes (same as -e)\n");
	fprintf(stderr, "\t-n\tDont look up usernames\n");
	fprintf(stderr, "\t-u\tLook for just this user\n");
	fprintf(stderr, "\t-o\tSorting method. Valid are cpu, mem, pid, uid, gid and size\n");
	fprintf(stderr, "\n");
	exit(1);
}

char *size_conv(long long number){
        char type[] = {'B', 'K', 'M', 'G', 'T'};
        int x=0;
        static char string[10];

        for(;x<5;x++){
                if( (number/1024) < (100)) {
                        break;
                }
                number = (number/1024);
        }

        snprintf(string, 10, "%lld%c", number, type[x]);        
        return string;
        
}

char *username(uid_t uid){
	static struct passwd *pass;

	pass = getpwuid(uid);
	if(pass == NULL){
		return "NULL";
	}

	return pass->pw_name;
}

char *proc_state(sg_process_state state){
	static char *pstat = "Unk";

	if(state == SG_PROCESS_STATE_RUNNING){
		pstat="Run";
	}

	if(state == SG_PROCESS_STATE_SLEEPING){
		pstat="Sleep";
	}

	if(state == SG_PROCESS_STATE_STOPPED){
		pstat="Stop";
	}

	if(state == SG_PROCESS_STATE_ZOMBIE){
		pstat="Zomb";
	}

	if(state == SG_PROCESS_STATE_UNKNOWN){
		pstat="Unk";
	}
	
	return pstat;
}

int main(int argc, char **argv){
	extern char *optarg;
	int c;
	int x;
	int full=0;
	int detailed=0;
	int nolookup=0;
	struct passwd *pwd_ent;
	uid_t uid = -1;
 
	int (*sortby_ptr)(const void *va, const void *vb);

	int entries;
	extern char *optarg;


	sg_process_stats *sg_proc = NULL;
	/* Default behaviour - sort by pid */
	sortby_ptr = sg_process_compare_pid;

	while ((c = getopt(argc, argv, "hneAfU:u:o:")) != -1){
		switch (c){
			case 'e':
			case 'A':
				full=1;
				break;
			case 'f':
				detailed=1;
				break;
			case 'n':
				nolookup=1;
				break;
			case 'U':
			case 'u':
				pwd_ent = getpwnam(optarg);
				if (pwd_ent == NULL){
					fprintf(stderr, "Error: user %s does not exist\n", optarg);
					exit(1);
				}
				uid = pwd_ent->pw_uid;
				break;
			case 'o':
				if(!strncasecmp(optarg, "cpu", 3)){
					sortby_ptr = sg_process_compare_cpu;
					break;
				}
				if(!strncasecmp(optarg, "size", 3)){
					sortby_ptr = sg_process_compare_size;
					break;
				}
				if(!strncasecmp(optarg, "pid", 3)){
					sortby_ptr = sg_process_compare_pid;
					break;
				}
				if(!strncasecmp(optarg, "mem", 3)){
					sortby_ptr = sg_process_compare_res;
					break;
				}
				if(!strncasecmp(optarg, "res", 3)){
					sortby_ptr = sg_process_compare_res;
					break;
				}
				if(!strncasecmp(optarg, "uid", 3)){
					sortby_ptr = sg_process_compare_uid;
					break;
				}
				if(!strncasecmp(optarg, "gid", 3)){
					sortby_ptr = sg_process_compare_gid;
					break;
				}
			case 'h':
			default:
				usage(argv[0]);
				break;
				exit(1);
		}
	}

	/* Get the procs - and sort them */
	sg_proc = sg_get_process_stats(&entries);
	qsort(sg_proc, entries, sizeof *sg_proc, sortby_ptr);

	/* Print the headers out on stderr, so should someone want to use this in a script, they wont need to
	 * strip out the first line.
	 */
	if (full){ 
		fprintf(stderr, "%9s %6s %6s %6s %7s %7s %4s %s\n", "User", "pid", "parent", "state", "size", "res", "CPU", "Process");
	}else{
		fprintf(stderr, "%6s %7s %4s %s\n", "pid", "size", "CPU", "Process");
	}

	for(x=0; x<entries; x++){
		char *proc_out=NULL;
		if (detailed) {
			proc_out = sg_proc->proctitle;
		}else{
			proc_out = sg_proc->process_name;
		}

		if((uid == -1) || (uid == sg_proc->uid)){
			if(full){
				printf("%9s %6d %6d %6s %7s %7s %2.2f %s\n", username(sg_proc->uid), (int)sg_proc->pid, (int)sg_proc->parent, proc_state(sg_proc->state), size_conv(sg_proc->proc_size), size_conv(sg_proc->proc_resident), sg_proc->cpu_percent, proc_out);
			}else{
				printf("%6d %7s %2.2f %s\n", (int)sg_proc->pid, size_conv(sg_proc->proc_size), sg_proc->cpu_percent, proc_out);
			}
		}

		sg_proc++;
	}

	return 0;
}
