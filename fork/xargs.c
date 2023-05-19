#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

#ifndef NARGS
#define NARGS 4
#endif

/*
 * Función que libera la memorua utilizada
 * para guardar los argumentos
 */
void liberar_argumentos(char **argumentos, int leido);

/*
 * Función que lee NARGS argumentos recibidos
 * en el stdin, guardandolos en el vector que
 * recibe como parámetro.
 */
int leer_argumentos(char *argumentos[NARGS + 1]);

/*
 * Función que ejecuta el comando recibido con sus
 * argumentos, creando un proceso hijo y llamando
 * a execvp. En caso de error, libera los argumentos
 * y sale del proceso.
 */
void ejecutar_comando(char *argumentos[NARGS + 1], char *comando, int leido);

/*
 * Función que chequea que la cantidad de argumentos
 * ingresados por consola sea la correcta, y que la
 * constante NARGS sea correcta.
 */
bool check_argumentos(int argc);


void
liberar_argumentos(char *argumentos[NARGS + 1], int leido)
{
	for (int i = 0; i < leido; i++) {
		free(argumentos[i]);
	}
}

void
ejecutar_comando(char *argumentos[NARGS + 1], char *comando, int leido)
{
	int pid = fork();
	if (pid == -1) {
		liberar_argumentos(argumentos, leido);
		perror("Error en la creación del fork\n");
		exit(1);
	}

	if (pid == 0) {
		execvp(comando, argumentos - 1);
		liberar_argumentos(argumentos, leido);
		perror("Error con excev");
		exit(0);
	} else {
		int estado;
		wait(&estado);
		if (!WIFEXITED(estado)) {
			liberar_argumentos(argumentos, leido);
			perror("Error en la ejecución del comando\n");
			exit(1);
		}
	}

	liberar_argumentos(argumentos, leido);
}

int
leer_argumentos(char *argumentos[NARGS + 1])
{
	int cont = 0;
	size_t n = 0;
	int leido = 0;
	char *linea = NULL;

	leido = getline(&linea, &n, stdin);

	while (cont < NARGS && leido > 0) {
		if (linea[leido - 1] == '\n')
			linea[leido - 1] = '\0';
		argumentos[cont] = malloc(sizeof(char) * (leido + 1));

		if (argumentos[cont] == NULL) {
			liberar_argumentos(argumentos, cont);
			free(linea);
			perror("Se produjo un error con malloc\n");
			return -1;
		}

		if (linea)
			strcpy(argumentos[cont], linea);
		cont++;
		if (cont < NARGS)
			leido = getline(&linea, &n, stdin);
	}

	argumentos[cont] = NULL;
	free(linea);

	return cont;
}

bool
check_argumentos(int argc)
{
	if (argc < 2) {
		perror("Error en la cantidad de argumentos ingresados.\n");
		return true;
	}
	if (NARGS <= 0) {
		perror("Error con NARGS. Debe ser mayor o igual a 1.\n");
		return true;
	}
	return false;
}

int
main(int argc, char *argv[])
{
	check_argumentos(argc) ? exit(1) : NULL;
	int leido = 0;
	do {
		char *argumentos[NARGS + 1];
		leido = leer_argumentos(argumentos);
		leido > 0 ? ejecutar_comando(argumentos, argv[1], leido) : NULL;
	} while (leido > 0);
	exit(0);
}