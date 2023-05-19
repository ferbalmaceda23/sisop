#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

int
main()
{
	srand((unsigned int) time(NULL));
	int fd_padre[2];
	int fd_hijo[2];

	if (pipe(fd_padre) == -1) {
		perror("Error en creación del pipe del padre.\n");
		exit(1);
	}

	if (pipe(fd_hijo) == -1) {
		perror("Error en creación del pipe del hijo.\n");
		exit(1);
	}

	printf("Hola, soy PID %d\n", getpid());
	printf("\t- primer pipe me devuelve: [%d, %d]\n", fd_padre[0], fd_padre[1]);
	printf("\t- segundo pipe me devuelve: [%d, %d]\n\n",
	       fd_hijo[0],
	       fd_hijo[1]);

	int pid = fork();
	if (pid == -1) {
		perror("Error en creación del fork.");
		close(fd_padre[0]);
		close(fd_padre[1]);
		close(fd_hijo[0]);
		close(fd_hijo[1]);
		exit(1);
	}

	if (pid > 0) {
		close(fd_padre[0]);

		int random = rand() % 100;

		printf("Donde fork me devuelve %d:\n", pid);
		printf("\t- getpid me devuelve: %d\n", getpid());
		printf("\t- getppid me devuelve: %d\n", getppid());
		printf("\t- random me devuelve: %d\n", random);
		printf("\t- envío valor %d a través de fd=%d\n\n",
		       random,
		       fd_padre[1]);

		if (write(fd_padre[1], &random, sizeof(int)) == -1) {
			perror("Error en write del pipe del padre.\n");
			close(fd_padre[1]);
			close(fd_hijo[0]);
			close(fd_hijo[1]);
			exit(1);
		}
		close(fd_padre[1]);
	} else {
		close(fd_padre[1]);
		close(fd_hijo[0]);

		int valor = 0;

		if (read(fd_padre[0], &valor, sizeof(int)) == -1) {
			perror("Error en read del pipe del padre.\n");
			close(fd_padre[0]);
			close(fd_hijo[1]);
			exit(1);
		}

		printf("Donde fork me devuelve 0:\n");
		printf("\t- getpid me devuelve: %d\n", getpid());
		printf("\t- getppid me devuelve: %d\n", getppid());
		printf("\t- recibo valor %d via fd=%d\n", valor, fd_padre[0]);
		printf("\t- reenvío valor en fd=%d y termino\n\n", fd_hijo[1]);

		if (write(fd_hijo[1], &valor, sizeof(int)) == -1) {
			perror("Error en write del pipe del hijo.\n");
			close(fd_hijo[1]);
			exit(1);
		}
		close(fd_hijo[1]);
		close(fd_padre[0]);

		exit(0);
	}

	close(fd_hijo[1]);

	int valor = 0;

	if (read(fd_hijo[0], &valor, sizeof(int)) == -1) {
		perror("Error en read del pipe del hijo.\n");
		close(fd_hijo[0]);
		exit(1);
	}

	printf("Hola, de nuevo PID %d:\n", getpid());
	printf("\t- recibí valor %d vía fd=%d\n\n", valor, fd_hijo[0]);

	close(fd_hijo[0]);

	exit(0);
}