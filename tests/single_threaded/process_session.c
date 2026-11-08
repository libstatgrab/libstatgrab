/* Regression for the Linux /proc/PID/stat session field. */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(LINUX)
#include <statgrab.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int
check_process(pid_t pid, pid_t parent, pid_t group, pid_t session)
{
	size_t entries, i;
	sg_process_stats *stats = sg_get_process_stats(&entries);

	if (stats == NULL) {
		fprintf(stderr, "sg_get_process_stats failed\n");
		return 1;
	}
	for (i = 0; i < entries; ++i) {
		if (stats[i].pid == pid) {
			if (stats[i].parent != parent || stats[i].pgid != group ||
			    stats[i].sessid != session || stats[i].uid != getuid() ||
			    stats[i].gid != getgid() || stats[i].process_name == NULL ||
			    stats[i].proc_size == 0) {
				fprintf(stderr, "pid %ld: parent %ld/%ld, group %ld/%ld, session %ld/%ld\n",
				        (long)pid, (long)stats[i].parent, (long)parent,
				        (long)stats[i].pgid, (long)group,
				        (long)stats[i].sessid, (long)session);
				return 1;
			}
			return 0;
		}
	}
	fprintf(stderr, "pid %ld missing from process statistics\n", (long)pid);
	return 1;
}
#endif

int
main(void)
{
#if defined(LINUX)
	int ready[2], release[2], status, failed;
	pid_t child;
	char result;

	if (sg_init(1) != SG_ERROR_NONE || pipe(ready) || pipe(release))
		return 1;
	child = fork();
	if (child == -1)
		return 1;
	if (child == 0) {
		close(ready[0]);
		close(release[1]);
		result = setsid() == -1 ? 'F' : 'R';
		if (write(ready[1], &result, 1) != 1)
			_exit(1);
		close(ready[1]);
		if (read(release[0], &result, 1) != 1)
			_exit(1);
		_exit(0);
	}
	close(ready[1]);
	close(release[0]);
	failed = read(ready[0], &result, 1) != 1 || result != 'R';
	close(ready[0]);
	if (!failed) {
		failed |= check_process(getpid(), getppid(), getpgrp(), getsid(0));
		failed |= check_process(child, getpid(), child, child);
		/* A second collection must preserve both the inherited and new session. */
		failed |= check_process(getpid(), getppid(), getpgrp(), getsid(0));
		failed |= check_process(child, getpid(), child, child);
	}
	result = 'Q';
	if (write(release[1], &result, 1) != 1)
		failed = 1;
	close(release[1]);
	if (waitpid(child, &status, 0) != child || !WIFEXITED(status) || WEXITSTATUS(status))
		failed = 1;
	sg_shutdown();
	return failed;
#else
	return 77;
#endif
}
