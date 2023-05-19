#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	cprintf("hello, world\n");
    int priority;
    for(;;){
        priority = sys_getpriority();
        cprintf("i am environment %08x and my priority is %d\n", thisenv->env_id, priority);
        if (priority == 7) {
            break;
        }
    }
}
