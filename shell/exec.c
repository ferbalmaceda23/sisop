#include "exec.h"

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
		int sep = block_contains(eargv[i], '=');
		if (sep == -1) {
			printf_debug("No se recibió una variable de entorno a "
			             "setear.\n");
			continue;
		}

		int arg_len = strlen(eargv[i]);

		if (sep > 0 && arg_len > sep) {
			char key[sep + 1];
			char value[arg_len - sep];

			get_environ_key(eargv[i], key);
			get_environ_value(eargv[i], value, sep);

			// '1' as third parameter means to overwrite env var
			if (setenv(key, value, 1) == -1) {
				printf_debug("Error al setear variable de "
				             "entorno.\n");
			}
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
	return (flags & O_CREAT) ? open(file, flags, S_IWUSR | S_IRUSR)
	                         : open(file, flags);
}

// redirects to specific fd
// returns 0 on success, -1 on error
static int
redir_fd(char *cmd_file, int fd, char *fd_name, int flags)
{
	int fd_aux = open_redir_fd(cmd_file, flags);
	if (fd_aux == -1) {
		printf_debug("Error al abrir archivo de redireccionamiento.\n");
		return -1;
	}

	if (dup2(fd_aux, fd) == -1) {
		printf_debug("Error al redireccionar %s.\n", fd_name);
		close(fd_aux);
		return -1;
	}

	return 0;
}

// redirects the stdin/stdout/stderr of the process
// returns 0 on success, -1 on error
static int
redir_fds(struct execcmd *cmd)
{
	if (strlen(cmd->out_file) > 0)
		if (redir_fd(cmd->out_file,
		             STDOUT_FD,
		             "stdout",
		             O_CREAT | O_TRUNC | O_RDWR | O_CLOEXEC) == -1)
			return -1;

	if (strlen(cmd->in_file) > 0)
		if (redir_fd(cmd->in_file, STDIN_FD, "stdin", O_RDWR | O_CLOEXEC) ==
		    -1)
			return -1;

	if (strlen(cmd->err_file) > 0) {
		if (strcmp(cmd->err_file, "&1") == 0) {
			if (dup2(STDOUT_FD, STDERR_FD) == -1) {
				printf_debug("Error al redireccionar stdout al "
				             "stderr.\n");
				return -1;
			}
		} else {
			if (redir_fd(cmd->err_file,
			             STDERR_FD,
			             "stderr",
			             O_CREAT | O_TRUNC | O_RDWR | O_CLOEXEC) == -1)
				return -1;
		}
	}

	return 0;
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
	case EXEC: {
		e = (struct execcmd *) cmd;
		if (e->argc == 0) {
			printf_debug(
			        "Error, se recibió un comando invalido.\n");
			exit(1);
		}

		set_environ_vars(e->eargv, e->eargc);
		execvp(e->argv[0], e->argv);

		printf_debug("Error al ejecutar comando.\n");
		exit(1);
		break;
	}

	case BACK: {
		b = (struct backcmd *) cmd;
		exec_cmd(b->c);
		exit(1);
		break;
	}

	case REDIR: {
		r = (struct execcmd *) cmd;
		if (r->argc == 0) {
			printf_debug(
			        "Error, se recibió un comando invalido.\n");
			exit(1);
		}

		if (redir_fds(r) == -1)
			exit(1);

		r->type = EXEC;
		exec_cmd(cmd);

		exit(1);
		break;
	}

	case PIPE: {
		p = (struct pipecmd *) cmd;

		if (!p->rightcmd || !p->leftcmd) {
			printf_debug("Error, nose recibieron comandos.\n");
			exit(1);
		}

		int fd[2];

		if (pipe(fd) == -1) {
			printf_debug("Error en la creación de pipe.\n");
			exit(1);
		}

		int pid_left = fork();
		if (pid_left == -1) {
			printf_debug("Error en la creación de fork del hijo "
			             "izquierdo.\n");
			exit(1);
		}
		if (pid_left == 0) {
			close(fd[0]);
			dup2(fd[1], STDOUT_FD);
			close(fd[1]);
			exec_cmd(p->leftcmd);
			break;
		}

		int pid_right = fork();
		if (pid_right == -1) {
			printf_debug("Error en la creación de fork del hijo "
			             "derecho.\n");
			exit(1);
		}
		if (pid_right == 0) {
			close(fd[1]);
			dup2(fd[0], STDIN_FD);
			close(fd[0]);
			exec_cmd(p->rightcmd);
			break;
		}

		close(fd[0]);
		close(fd[1]);

		wait(NULL);
		wait(NULL);

		break;
	}
	}
}