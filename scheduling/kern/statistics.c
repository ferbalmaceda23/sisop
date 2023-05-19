#include <kern/statistics.h>

int p_count = 0;
int run_count = 0;
int boost_count = 0;
int run_count_by_priority[MAX_ENV_PRIORITY];

void init_stats(void) {
    p_count = 0;
    run_count = 0;
    boost_count = 0;
    for (int i = 0; i < MAX_ENV_PRIORITY; i++) {
        run_count_by_priority[i] = 0;
    }
}

void print_statistics(void) {
    cprintf("Processes: %d\n", p_count);
    cprintf("Run count: %d\n", run_count);
    cprintf("Average run count per process: %d\n", run_count / p_count);
    cprintf("Amount of boosts: %d\n", boost_count);

    for (int i = 0; i < MAX_ENV_PRIORITY; i++) {
        cprintf("Priority %d ran %d times\n", i, run_count_by_priority[i]);
    }
}