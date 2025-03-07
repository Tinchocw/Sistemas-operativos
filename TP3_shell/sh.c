#include "defs.h"
#include "types.h"
#include "readline.h"
#include "freecmd.h"
#include "runcmd.h"
#include "sh.h"

char prompt[PRMTLEN] = { 0 };

void
sigchild_handler(int signum)
{
	if (back != NULL && (signum = waitpid(back->pid, &status, WNOHANG)) > 0) {
		print_status_info(back);
		free_command(back);
	}
}

// runs a shell command
static void
run_shell()
{
	char *cmd;

	while ((cmd = read_line(prompt)) != NULL)
		if (run_cmd(cmd) == EXIT_SHELL)
			return;
}

// initializes the shell
// with the "HOME" directory
static void
init_shell()
{
	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}

	signal(SIGCHLD, sigchild_handler);
}

int
main(void)
{
	init_shell();

	run_shell();

	return 0;
}
