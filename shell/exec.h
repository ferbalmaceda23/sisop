#ifndef EXEC_H
#define EXEC_H

#include "defs.h"
#include "types.h"
#include "utils.h"
#include "freecmd.h"

#define STDIN_FD 0
#define STDOUT_FD 1
#define STDERR_FD 2

extern struct cmd *parsed_pipe;

void exec_cmd(struct cmd *c);

#endif  // EXEC_H
