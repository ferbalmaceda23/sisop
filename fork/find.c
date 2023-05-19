#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdbool.h>

#define FLAG "-i"

/*
 * Abstracción de las funciones de comparación "strcmp" y "strcasecmp",
 * para evitar repetición de codigo.
 */
typedef char *(*buscar_substring)(const char *, const char *);

/*
 * Funcion que chequea si la cantidad de argumentos recibida es la correcta
 */
bool check_argumentos(int argc);

/*
 * Funcion que chequea que no se este accediendo a un directorio invalido
 * como "." o ".."
 */
bool directorio_valido(char *dir_name);

/*
 * Funcion que concatena los strings recibidos para utilizarlos como ruta
 */
char *concatenar(char *string_1, char *string_2);

/*
 * Función que recorre recursivamente los directorios buscando los archivos
 * que contengan el string recibido.
 */
void recorrer_recursivo(char *string,
                        int fd,
                        char ruta[PATH_MAX],
                        buscar_substring buscador);


void
recorrer_recursivo(char *string, int fd, char ruta[PATH_MAX], buscar_substring buscador)
{
	DIR *directorio_actual = fdopendir(fd);
	if (directorio_actual == NULL) {
		perror("Error en fdopendir");
		return;
	}

	struct dirent *entry = readdir(directorio_actual);

	while (entry != NULL) {
		char *dir_name = (*entry).d_name;
		char ruta_actual[PATH_MAX];
		strcpy(ruta_actual, ruta);

		if ((*entry).d_type == DT_DIR) {
			if (directorio_valido(dir_name)) {
				concatenar(ruta_actual, dir_name);
				if (buscador(dir_name, string) != NULL)
					printf("%s\n", ruta_actual);
				int fd_directorio =
				        openat(dirfd(directorio_actual),
				               dir_name,
				               O_DIRECTORY);
				recorrer_recursivo(string,
				                   fd_directorio,
				                   ruta_actual,
				                   buscador);
			}
		} else {
			if (buscador(dir_name, string) != NULL) {
				char *ruta_final =
				        concatenar(ruta_actual, dir_name);
				printf("%s\n", ruta_final);
			}
		}
		entry = readdir(directorio_actual);
	}
	closedir(directorio_actual);
}

char *
concatenar(char *string_1, char *string_2)
{
	(strlen(string_1) > 0) ? strcat(string_1, "/") : NULL;
	return strcat(string_1, string_2);
}

bool
directorio_valido(char *dir_name)
{
	return (strcmp(dir_name, ".") != 0) && (strcmp(dir_name, "..") != 0);
}

bool
check_argumentos(int argc)
{
	return (argc < 2 || argc > 3);
}

int
main(int argc, char *argv[])
{
	if (check_argumentos(argc)) {
		perror("Error en la cantidad de argumentos. Debe ingresarse un "
		       "string con o sin el flag -i.\n");
		exit(1);
	}

	buscar_substring buscador = strstr;
	char *string = argv[1];
	char ruta[PATH_MAX];
	strcpy(ruta, "");

	if (argc == 3 && strcmp(argv[1], FLAG) == 0) {
		buscador = strcasestr;
		string = argv[2];
	}

	DIR *directorio_actual = opendir(".");
	if (directorio_actual == NULL) {
		perror("Error en el llamado a opendir");
		exit(1);
	}

	recorrer_recursivo(string, dirfd(directorio_actual), ruta, buscador);

	closedir(directorio_actual);

	exit(0);
}