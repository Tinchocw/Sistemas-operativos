#include "builtin.h"
#include "parsing.h"
// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	if (strcmp(cmd, "exit") == 0) {
		return EXIT_SHELL;
	}

	return 0;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	if (cmd[0] != 'c' || cmd[1] != 'd') {
		return 0;
	}

	char *dir;

	if (strlen(cmd) == 2) {
		dir = getenv("HOME");

	} else if (cmd[2] == ' ' && cmd[3] == '$') {
		dir = cmd + 4;  // Apuntar dir a partir de cmd[4]

	} else {
		dir = cmd + 3;  // Apuntar dir a partir de cmd[3]
	}

	char buf[BUFLEN] = { 0 };

	if (chdir(dir) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", dir);
		perror(buf);

	} else {
		char *cwd = getcwd(NULL, 0);

		snprintf(prompt, sizeof prompt, "(%s)", cwd);

		free(cwd);
	}

	return 1;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	// Your code here
	if (strcmp(cmd, "pwd") == 0) {
		char abs_dir[BUFLEN] = { 0 };
		printf("%s\n", getcwd(abs_dir, BUFLEN));
		return 1;
	}
	return 0;
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	// Your code here

	return 0;
}
