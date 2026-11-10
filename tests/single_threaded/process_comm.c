/* Linux process names must not change the alignment of /proc/PID/stat. */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(LINUX)
#include <statgrab.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int
check_process(pid_t pid, const char *name, const char *alternative, int reentrant)
{
	size_t entries, i;
	sg_process_stats *stats = reentrant ? sg_get_process_stats_r(&entries) : sg_get_process_stats(&entries);
	int failed = 1;

	if (stats == NULL) {
		fprintf(stderr, "sg_get_process_stats%s failed\n", reentrant ? "_r" : "");
		return 1;
	}
	for (i = 0; i < entries; ++i) {
		if (stats[i].pid != pid)
			continue;
		failed = stats[i].parent != (pid == getpid() ? getppid() : getpid()) ||
		         stats[i].pgid != getpgrp() || stats[i].uid != getuid() ||
		         stats[i].euid != geteuid() || stats[i].gid != getgid() ||
		         stats[i].egid != getegid() || stats[i].proc_size == 0 ||
		         stats[i].state != SG_PROCESS_STATE_RUNNING ||
		         stats[i].process_name == NULL;
		if (!failed)
			failed = strcmp(stats[i].process_name, name) != 0 &&
			         (alternative == NULL || strcmp(stats[i].process_name, alternative) != 0);
		if (failed)
			fprintf(stderr, "pid %ld: incorrect name or process fields "
			        "(api=%s, name=[%s], expected=[%s]/[%s], "
			        "parent=%ld/%ld, pgid=%ld/%ld, uid=%ld/%ld, "
			        "euid=%ld/%ld, gid=%ld/%ld, egid=%ld/%ld, "
			        "size=%llu, state=%d/%d)\n",
			        (long)pid, reentrant ? "reentrant" : "static",
			        stats[i].process_name == NULL ? "(null)" : stats[i].process_name,
			        name, alternative == NULL ? "(none)" : alternative,
			        (long)stats[i].parent, (long)(pid == getpid() ? getppid() : getpid()),
			        (long)stats[i].pgid, (long)getpgrp(),
			        (long)stats[i].uid, (long)getuid(),
			        (long)stats[i].euid, (long)geteuid(),
			        (long)stats[i].gid, (long)getgid(),
			        (long)stats[i].egid, (long)getegid(),
			        (unsigned long long)stats[i].proc_size,
			        (int)stats[i].state, (int)SG_PROCESS_STATE_RUNNING);
		break;
	}
	if (i == entries)
		fprintf(stderr, "pid %ld missing from process statistics\n", (long)pid);
	if (reentrant)
		sg_free_process_stats(stats);
	return failed;
}

static int
check_changing_name(void)
{
	int ready[2], status, i, failed = 0;
	pid_t child;
	char result;

	if (pipe(ready)) {
		perror("pipe");
		return 1;
	}
	child = fork();
	if (child == -1) {
		perror("fork");
		close(ready[0]);
		close(ready[1]);
		return 1;
	}
	if (child == 0) {
		close(ready[0]);
		if (prctl(PR_SET_NAME, "a b") || write(ready[1], "R", 1) != 1) {
			perror("child initialization");
			_exit(1);
		}
		close(ready[1]);
		for (;;) {
			if (prctl(PR_SET_NAME, "ab") || prctl(PR_SET_NAME, "a b")) {
				perror("child PR_SET_NAME");
				_exit(1);
			}
		}
	}
	close(ready[1]);
	if (read(ready[0], &result, 1) != 1 || result != 'R') {
		fprintf(stderr, "child did not signal readiness\n");
		failed = 1;
	}
	close(ready[0]);
	if (!failed) {
		for (i = 0; i < 20; ++i)
			failed |= check_process(child, "ab", "a b", i % 2);
	}
	if (kill(child, SIGTERM) || waitpid(child, &status, 0) != child ||
	    !WIFSIGNALED(status) || WTERMSIG(status) != SIGTERM) {
		fprintf(stderr, "child termination failed\n");
		failed = 1;
	}
	return failed;
}
#endif

int
main(void)
{
#if defined(LINUX)
	const char *names[] = {
		"ordinary", "two words", " white space ", "a\tb", "line\nbreak",
		" a b c d e f g ", "(nested)", "two)parts", ")end)", "", "   "
	};
	char original[16];
	size_t i;
	int failed = 0;

	if (prctl(PR_GET_NAME, original)) {
		perror("PR_GET_NAME");
		return 1;
	}
	if (sg_init(1) != SG_ERROR_NONE) {
		fprintf(stderr, "sg_init failed\n");
		return 1;
	}
	for (i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
		if (prctl(PR_SET_NAME, names[i])) {
			perror("PR_SET_NAME");
			failed = 1;
			break;
		}
		failed |= check_process(getpid(), names[i], NULL, 0);
		failed |= check_process(getpid(), names[i], NULL, 1);
	}
	if (prctl(PR_SET_NAME, original)) {
		perror("restore PR_SET_NAME");
		failed = 1;
	}
	failed |= check_changing_name();
	sg_shutdown();
	return failed;
#else
	return 77;
#endif
}
