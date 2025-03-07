#include "exec.h"
#include "defs.h"

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	for (int i = 0; i < eargc; i++) {
		int idx = block_contains(eargv[i], '=');

		if (idx < 0) {
			continue;
		}

		char key[idx + 1];
		char *value =
		        (char *) calloc(strlen(eargv[i]) - idx, sizeof(char));
		get_environ_key(eargv[i], key);
		get_environ_value(eargv[i], value, idx);

		int ret = setenv(key, value, REPLACE_ENV);
		free(value);

		if (ret < 0) {
			char buf[BUFLEN] = { 0 };
			snprintf(buf,
			         sizeof buf,
			         "cannot set environment variable %s ",
			         key);
			perror(buf);

			exit(ERROR_STATUS);
		}
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	int fd = open(file, flags, S_IWUSR | S_IRUSR);

	if (fd < 0) {
		fprintf(stderr, "Error opening file %s: %s\n", file, strerror(errno));
		exit(ERROR_STATUS);
	}

	return fd;
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC:
		e = (struct execcmd *) cmd;
		set_environ_vars(e->eargv, e->eargc);

		if (e->argv[0]) {
			if (execvp(e->argv[0], e->argv) < 0) {
				char buf[BUFLEN] = { 0 };
				snprintf(buf,
				         sizeof buf,
				         "cannot exec file %s ",
				         e->argv[0]);
				perror(buf);

				free_command(cmd);
				exit(ERROR_STATUS);
			}
		}

		break;

	case BACK: {
		b = (struct backcmd *) cmd;

		setpgid(0, 0);
		exec_cmd(b->c);

		break;
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// file name's length
		r = (struct execcmd *) cmd;
		set_environ_vars(r->eargv, r->eargc);

		int fd;

		/*
		 * ls /usr >out1.txt
		 * Lo que tengo que hacer en este caso es duplicar lo que tengo
		 * en antes del > en el outfile
		 * */

		if (strlen(r->in_file) > 0) {  //<
			// Tengo que chequear que no sea -1
			fd = open_redir_fd(r->in_file, O_RDONLY | O_CLOEXEC);
			dup2(fd, STDIN_FILENO);
			// wc -w <out1.txt
		}
		if (strlen(r->out_file) > 0) {  //>
			fd = open_redir_fd(r->out_file,
			                   O_WRONLY | O_CREAT | O_TRUNC |
			                           O_CLOEXEC);
			dup2(fd, STDOUT_FILENO);
			// ls /usr >out1.txt
		}
		if (strlen(r->err_file) > 0) {
			// Verificar si se necesita la redirección 2>&1
			if (strlen(r->err_file) > 0 &&
			    strcmp(r->err_file, "&1") == 0) {
				dup2(STDOUT_FILENO, STDERR_FILENO);
			} else {
				fd = open_redir_fd(r->err_file,
				                   O_WRONLY | O_CREAT |
				                           O_TRUNC | O_CLOEXEC);
				dup2(fd, STDERR_FILENO);
			}
		}

		if (r->argv[0]) {
			if (execvp(r->argv[0], r->argv) < 0) {
				char buf[BUFLEN] = { 0 };
				snprintf(buf,
				         sizeof buf,
				         "cannot exec file %s ",
				         r->argv[0]);
				perror(buf);

				exit(ERROR_STATUS);
			}
		}

		break;
	}

	case PIPE: {
		// pipes two commands
		//
		p = (struct pipecmd *) cmd;
		// set_environ_vars(p->leftcmd->eargv, p->leftcmd->eargc);
		// set_environ_vars(p->rightcmd->eargv, p->rightcmd->eargc);
		// Casteo a exec?

		int pipefd[2];
		if (pipe(pipefd) == -1) {
			perror("pipe");
			exit(ERROR_STATUS);
		}

		pid_t pid_left = fork();
		if (pid_left == -1) {
			perror("fork");
			exit(ERROR_STATUS);
		}

		if (pid_left == 0) {
			close(pipefd[READ]);
			dup2(pipefd[WRITE], STDOUT_FILENO);
			close(pipefd[WRITE]);
			exec_cmd(p->leftcmd);
		}

		pid_t pid_right = fork();
		if (pid_right == -1) {
			perror("fork");
			exit(ERROR_STATUS);
		}

		if (pid_right == 0) {
			close(pipefd[WRITE]);
			dup2(pipefd[READ], STDIN_FILENO);
			close(pipefd[READ]);
			exec_cmd(p->rightcmd);
		}

		close(pipefd[READ]);
		close(pipefd[WRITE]);

		free_command(parsed_pipe);

		waitpid(pid_left, NULL, 0);

		int right_status;
		if (waitpid(pid_right, &right_status, 0) < 0) {
			perror("waitpid");
			exit(ERROR_STATUS);
		}

		if (WIFEXITED(right_status)) {
			exit(WEXITSTATUS(right_status));
		} else {
			exit(ERROR_STATUS);
		}
	}
	}
}
