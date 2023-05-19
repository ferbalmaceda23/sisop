# Lab: shell

### Búsqueda en $PATH
1. **¿cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?**
   
    La syscall `execve(2)` es una llamada directa al sistema operativo que permite ejecutar un programa, cambiando el espacio de memoria del proceso que la llamo, excepto por su PID. Mientras que la familia de wrappers proporcionados por la librería estándar de C (libc) `exec(3)` son funciones que permiten ejecutar un programa llamando a la syscall `execve(2)` que nos ayudan a evitar tener que pasarle a la función, por ejemplo, la busqueda del binario en el PATH o tener que manejar el entorno del proceso.

2. **¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?**

    Sí, la llamada a `exec(3)` puede fallar. En ese caso, se permanece en el mismo proceso desde donde se quizo ejecutar el nuevo proceso, se le informa al usuario del error imprimiendo por consola y la shell sigue corriendo esperando por nuevos comandos.

### Comandos built-in
1. **¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)**

    Sí, se podría implementar `cd` sin necesidad de ser built-in dado que solo imprime el directorio actual. Sin embargo se la implementa como built-in debido a que es más optimo que implementarlo como un comando normal dado que no se debe crear un proceso hijo para su ejecución.
---

### Variables de entorno adicionales
1. **¿Por qué es necesario hacerlo luego de la llamada a fork(2)?**

    Es necesario hacerlo luego de la llamada a `fork(2)` porque si no, el hijo heredaría las variables de entorno del padre, y no tendría acceso a las variables de entorno que se le asignaron en el hijo.

2. **En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un array y se lo coloca en el tercer argumento de una de las funciones de exec(3).**

    a. **¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.**

    No, el comportamiento resultante no es el mismo que en el primer caso. Esto se debe a que las variables de entorno que se le pasan al tercer argumento de una de las funciones de `exec(3)` se agregan al final de las variables de entorno del proceso padre, y no reemplazan las variables de entorno del proceso padre.

    b. **Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.**

    Una posible implementación para que el comportamiento sea el mismo es que se borren las variables de entorno del proceso padre, y luego se agreguen las variables de entorno que se le pasan al tercer argumento de una de las funciones de `exec(3)`.    
---

### Procesos en segundo plano
1.  **Detallar cuál es el mecanismo utilizado para implementar procesos en segundo plano.**

    Para implementar procesos en segundo plano, se utiliza la función `fork()` para crear un proceso hijo y se utiliza la función `execvp()` en el proceso hijo para ejecutar el comando ingresado por el usuario. Luego, se utiliza la función `waitpid()` con el flag **WHOHANG**, que a diferencia de `wait()` no es bloqueante lo que significa que el proceso padre sigue corriendo en simultaneo.
---

### Flujo estándar
1. **Investigar el significado de 2>&1, explicar cómo funciona su forma general y mostrar qué sucede con la salida de cat out.txt en el ejemplo. Luego repetirlo invertiendo el orden de las redirecciones. ¿Cambió algo?**

    `2>&1` significa que el file descriptor 2 (stderr) se redirige al file descriptor 1 (stdout). En el ejemplo:
    ```
    ls -C /home /noexiste >out.txt 2>&1
    ```
    lo que sucede es que se se escribe tanto la salida de `ls` como el error de `ls` en el archivo `out.txt`.
    
    Si se invierte el orden de las redirecciones, tanto el error de `ls` como la salida de `ls` se escriben en la consola (stdout).
---

### Tuberías simples (pipes)
1. **Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe ¿Cambia en algo? ¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con la implementación del este lab.**
   
    Si se ejecuta un pipe, el exit code reportado por la shell es el exit code del último comando del pipe (en general, `0`) incluso si algún comando falla. Si se desea que el exit code sea del ultimo pipe que fallo se lo debe setear con el comando: `set -eo pipefail`.
    
    A continuación se muestra la salida de la terminal al ejecutar comandos entre pipe en bash.
    
    a. Caso de exit code 0:
    ```
    λ fer ~ → ls | grep h
    hola
    λ fer ~ → echo $?
    0
    ```
    b. Caso de exit code 0 con comando fallido:
    ```
    λ fer ~ → ls | grep no_existe | wc -l
    0
    λ fer ~ → echo $?
    0
    ```

    A continuación se muestra la salida de la terminal al ejecutar comandos entre pipe en la implementación de este lab.
    
    a. Caso de exit code 0:
    ```
      (/home/fer) 
    $ ls | grep h
    hola
      (/home/fer) 
    $ echo $?
    0
        Program: [echo $?] exited, status: 0
    ```
    b. Caso de exit code 0 con comando fallido:
    ```
      (/home/fer) 
    $ ls | grep no_existe | wc -l
    0
      (/home/fer) 
    $ echo $?
    0
        Program: [echo $?] exited, status: 0
    ```
---

### Pseudo-variables
1. **Investigar al menos otras tres variables mágicas estándar, y describir su propósito. Incluir un ejemplo de su uso en bash (u otra terminal similar).**
   
    a. `$_`: expande el ultimo argumento de la ultima linea de comando ejecutada.
    ```
    λ fer ~ → ls -l
    total 4
    drwxrwxr-x 3 fer fer 4096 Jun 28 21:02 R
    -rw-r--r-- 1 fer fer    0 Sep 25 20:16 hola
    λ fer ~ → echo $_
    -l
    ```

    b. `$0`: expande la ruta del binario de la shell actual.
    ```
    λ fer ~ → echo $0
    /bin/bash
    ```

    c. `$$`: expande el PID de la shell actual.
    ```
    λ fer ~ → ps
    PID TTY          TIME CMD 
    10516 pts/0    00:00:00 bash
    10791 pts/0    00:00:00 ps  
    λ fer ~ → echo $$
    10516
    ```
---
