#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/*
 * Función que escribe en el pipe recibido el valor recibido. En
 * caso de error sale con valor 1, imprimiendo por consola el error.
 */
void escribir_valor_en_el_pipe(int valor, int fd[2]);

/*
 * Función que lee del pipe recibido para guardarlo en la dirección
 * recibida. Devuelve la cantidad leida. En caso de error sale con
 * valor 1, imprimiendo por consola el error.
 */
int leer_valor_del_pipe(int *valor, int fd[2]);

/*
 * Función que imprime por consola los numeros primos de la lista
 * de numeros recibida en el pipe izquierdo. En caso de error sale
 * con valor 1, imprimiendo por consola el error.
 */
void obtener_primos(int fd_izquierdo[2]);


void
escribir_valor_en_el_pipe(int valor, int fd[2])
{
	if (write(fd[1], &valor, sizeof(int)) == -1) {
		perror("Error en write del pipe\n");
		exit(1);
	}
}

int
leer_valor_del_pipe(int *valor, int fd[2])
{
	int leido = read(fd[0], valor, sizeof(int));
	if (leido == -1) {
		perror("Error en el read del pipe\n");
		exit(1);
	}
	return leido;
}

void
obtener_primos(int fd_izquierdo[2])
{
	close(fd_izquierdo[1]);

	int fd_derecho[2];
	if (pipe(fd_derecho) == -1) {
		perror("Error en creacion del pipe derecho\n");
		close(fd_izquierdo[0]);
		exit(1);
	}

	int valor_1 = 0;
	int leido = leer_valor_del_pipe(&valor_1, fd_izquierdo);
	if (leido == 0) {
		close(fd_izquierdo[0]);
		close(fd_derecho[0]);
		close(fd_derecho[1]);
		exit(0);
	}

	printf("primo %d\n", valor_1);

	int pid = fork();
	if (pid == -1) {
		perror("Error en creacion del fork\n");
		close(fd_izquierdo[0]);
		close(fd_derecho[0]);
		close(fd_derecho[1]);
		exit(1);
	}

	if (pid > 0) {
		close(fd_derecho[0]);

		int valor_2 = 0;
		leido = leer_valor_del_pipe(&valor_2, fd_izquierdo);
		while (leido > 0) {
			if (valor_2 % valor_1 != 0)
				escribir_valor_en_el_pipe(valor_2, fd_derecho);

			leido = leer_valor_del_pipe(&valor_2, fd_izquierdo);
		}

		close(fd_izquierdo[0]);
		close(fd_derecho[1]);

		wait(NULL);

		exit(0);
	}

	close(fd_izquierdo[0]);
	obtener_primos(fd_derecho);
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		perror("Error en la cantidad de argumentos\n");
		exit(1);
	}

	int valor = atoi(argv[1]);

	if (valor < 2) {
		perror("Error en el valor ingresado, debe ser mayor o igual a "
		       "2\n");
		exit(1);
	}

	int fd_izquierdo[2];
	if (pipe(fd_izquierdo) == -1) {
		perror("Error creación del pipe izquierdo\n");
		exit(1);
	}

	int pid = fork();
	if (pid == -1) {
		perror("Error creación del fork\n");
		close(fd_izquierdo[0]);
		close(fd_izquierdo[1]);
		exit(1);
	}

	if (pid == 0) {
		obtener_primos(fd_izquierdo);
	} else {
		close(fd_izquierdo[0]);

		for (int i = 2; i <= valor; i++)
			escribir_valor_en_el_pipe(i, fd_izquierdo);

		close(fd_izquierdo[1]);

		wait(NULL);

		exit(0);
	}
}