#include "builtin.h"
#include "utils.h"


// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	return (strcmp(cmd, "exit") == 0);
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
	char *directory;

	if (strcmp(cmd, "cd") == 0 || strcmp(cmd, "cd ") == 0) {
		directory = getenv("HOME");
	} else if (strncmp(cmd, "cd ", 3) == 0) {
		directory = cmd + 3;
	} else {
		return 0;
	}

	if (chdir(directory) == -1) {
		printf_debug("Error, no se pudo cambiar de directorio.\n");
		return -1;
	} else {
		char buf[PRMTLEN];
		snprintf(prompt, sizeof(prompt), "(%s)", getcwd(buf, PRMTLEN));
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
	if (strcmp(cmd, "pwd") != 0)
		return 0;

	char buf[PRMTLEN];
	printf_debug("%s\n", getcwd(buf, PRMTLEN));

	return 1;
}
