# sched.md

Lugar para respuestas en prosa, seguimientos con GDB y documentacion del TP

# Sección 1

## Visualizaremos el context switch con GDB

En la primer imagen, realizamos el break en la funcion `context switch` y vemos que antes de ejecutar la primer instrucción, sus registros tienen determinados valores iniciales. Lo que realizamos aquí es una reubicación del stack pointer para que apunte al parametro que se le pasó a la función context switch.
![img1](/assets/s1gdb1.png)

En la segunda imagen vemos que efectivamente el stack pointer fue reubicado.
![img2](/assets/s1gdb2.png)

La siguiente acción ejecutada fue popa, que llenará los registros pushable del struct trapframe. En la siguiente imagen vemos que los registros fueron llenados con los valores que se encuentran en el stack.

![img3](/assets/s1gdb3.png)

Luego de esto, se realiza un pop y se llena el registro es, vemos en la siguiente imagen que ha cambiado.

![img4](/assets/s1gdb4.png)

Luego de esto, se realiza un pop y se llena el registro ds, vemos en la siguiente imagen que ha cambiado.

![img5](/assets/s1gdb5.png)

Por ultimo, mostramos el valor de los registros que han cambiado al ejecutar la instrucción privilegiada iret.

![img6](/assets/s1gdb6.png)

Observamos que cambió el registro eip, cs, eflags, esp y ss.

## Ejecución de un unico proceso

Luego de ejecutarse el context switch, se ha ejecutado un proceso "user_hello", como lo vemos en la siguiente imagen.

![img7](/assets/s1qemu.png)

Este programa utilizó syscalls para poder mostrar un mensaje por pantalla. Con esto se puede ver que el kernel es capaz de ejecutar un proceso y este de ejecutar syscalls. El proceso finalizó satisfactoriamente.


## Nuestro scheduler de prioridades

Para poder implementar nuestro scheduler de prioridades, primero debemos modificar la estructura de datos que contiene la información de los procesos. Para esto, agregamos un campo llamado `env_priority`. Este campo es un entero que representa la prioridad del proceso, donde 0 es la prioridad más alta y 8 la más baja.

Nuestro scheduler consiste en una versión modificada de Multi Level Feedback Queue.

En el scheduler, primero se recorre la lista de procesos para ver si hay alguno que esté listo para ejecutarse, partiendo del siguiente al actual siendo ejecutado, y fijandonos que tenga una prioridad igual o mejor al actual. Si hay alguno, se corre.

Las reglas de éste scheduler son las siguientes:

1. Para un conjunto de procesos, se ejecutará el que tenga mejor priodidad (más cercana a 0).
2. Si hay 2 o más procesos con la mejor misma prioridad se ejecutarán en round robin.
3. Cuando se crea un nuevo proceso, se le asigna la prioridad de su padre. La prioridad propia de un proceso puede ser modificada(empeorada) con la syscall `set_priority`.
4. Luego de llegar a una cantidad fija de ejecuciones en su prioridad actual (2500), se empeorará su prioridad (se sumará 1 o permanecerá en 7 si ya lo estaba). Este cambio de prioridad se hace justo antes de ejecutar el proceso, y luego se ejecutará. A partir del proximo cambio de contexto a modo kernel, su ejecución dependerá del scheduler.
5. Luego de que se produzcan 1500 interrupciones en el sistema (se contabilizan a la hora de hacerles acknowledge), se mejorará la prioridad de todos los procesos. (Si ya estaban en la mejor prioridad, no se modifica)

## Ejemplos incluidos en el repositorio

En el repositorio se incluyen 3 ejemplos de ejecución de procesos. Estos se encuentran en la carpeta `user`. Los 3 ejemplos son:

1. `priorityescalator`: Este programa utiliza la syscall `set_priority` para ir empeorando su prioridad hasta llegar a 7. Una vez allí se bloquea en un loop infinito imprimiendo su propia prioridad hasta volver a tener prioridad 0. Este programa es útil para probar que es posible empeorar la prioridad de un mismo proceso, y ver que luego de un determinado tiempo se obtiene un boost..
1. `priorityforker`: Este programa creará un nuevo proceso hijo (a través de un fork) y luego se empeorará su priodidad. Se verá como primero se ejecuta su hijo, luego ambos en round robin, y luego que finalice el hijo solo se ejecutará el padre. Este programa es útil para probar que se puede crear un proceso con la misma prioridad que su padre y la ejecución de varios procesos con la misma prioridad en round robin.
1. `priorityspammer`: Este programa hará flood en la consola y mostrará que tras un tiempo empeora sola su prioridad. Este ejemplo sirve para ver que la priodidad de un proceso se ve afectada tras correr por cierto tiempo.