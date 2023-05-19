#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	cprintf("hello, world\n");
    int priority = sys_getpriority();
	cprintf("i am environment %08x and my priority is %d\n", thisenv->env_id, priority);
    sys_changepriority(1);
    priority = sys_getpriority();
    cprintf("i am environment %08x and my priority is %d\n", thisenv->env_id, priority);
    sys_changepriority(2);
    priority = sys_getpriority();
    cprintf("i am environment %08x and my priority is %d\n", thisenv->env_id, priority);
    sys_changepriority(3);
    priority = sys_getpriority();
    cprintf("i am environment %08x and my priority is %d\n", thisenv->env_id, priority);
    sys_changepriority(4);
    priority = sys_getpriority();
    cprintf("i am environment %08x and my priority is %d\n", thisenv->env_id, priority);
    sys_changepriority(5);
    priority = sys_getpriority();
    cprintf("i am environment %08x and my priority is %d\n", thisenv->env_id, priority);
    sys_changepriority(6);
    priority = sys_getpriority();
    cprintf("i am environment %08x and my priority is %d\n", thisenv->env_id, priority);
    sys_changepriority(7);
    priority = sys_getpriority();
    cprintf("i am environment %08x and my priority is %d\n", thisenv->env_id, priority);
    for(;;){
        priority = sys_getpriority();
        cprintf("i am environment %08x and my priority is %d\n", thisenv->env_id, priority);
        if (priority == 0) {
            break;
        }
    }
}
