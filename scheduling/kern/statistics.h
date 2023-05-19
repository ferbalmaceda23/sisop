#ifndef STATISTICS_H
#define STATISTICS_H

#include <inc/stdio.h>
#include <inc/env.h>

extern int p_count;
extern int run_count;
extern int boost_count;
extern int run_count_by_priority[MAX_ENV_PRIORITY];

void init_stats(void);
void print_statistics(void);

#endif