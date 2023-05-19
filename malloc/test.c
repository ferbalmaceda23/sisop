#include "malloc.h"
#include "printfmt.h"
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include "pa2mm.h"

extern int amount_of_mallocs;
extern int amount_of_frees;
extern int requested_memory;

#define TEST_STRING "aaaaa"

void set_up(void);
void pruebas_malloc(void);
void pruebas_free(void);
void pruebas_realloc(void);
void pruebas_calloc(void);

void
set_up()
{
	amount_of_mallocs = 0;
	amount_of_frees = 0;
	requested_memory = 0;
	errno = 0;
}

void
pruebas_malloc()
{
	pa2m_nuevo_grupo("PRUEBAS MALLOC Y FREE");

	set_up();
	char *var1 = malloc(100);
	strcpy(var1, "PRUEBA");
	pa2m_afirmar(errno != ENOMEM && amount_of_mallocs == 1 &&
	                     amount_of_frees == 0 && strcmp(var1, "PRUEBA") == 0,
	             "Un malloc se ejecuta correctamente");
	free(var1);

	set_up();
	int *var2 = malloc(sizeof(int));
	free(var2);
	pa2m_afirmar(requested_memory == 32,
	             "Si se pide menos que el minimo, se reserva el minimo");

	set_up();
	int *var3 = malloc(sizeof(int) * 10);
	free(var3);
	pa2m_afirmar(requested_memory == sizeof(int) * 10,
	             "Si se pide mas que el minimo, se reserva lo pedido");

	set_up();
	int *var15 = malloc(4195200);
	pa2m_afirmar(var15 == NULL && errno == ENOMEM,
	             "Si se pide mas que el tamanio de un bloque grande, se "
	             "devuelve NULL y errno = ENOMEM");
	set_up();
	int *var21 = malloc(0);
	pa2m_afirmar(var21 == NULL, "Malloc de 0 devuelve null");

	set_up();
	int *var16 = malloc(100);
	int *var17 = malloc(100);
	int *var18 = malloc(80);
	int *var19 = malloc(100);
	free(var16);
	free(var18);
	int *var20 = malloc(80);
#ifdef BEST_FIT
	pa2m_afirmar(var20 == var18,
	             "Best Fit devuelve el bloque de memoria mas chico");
#endif
#ifdef FIRST_FIT
	pa2m_afirmar(
	        var20 == var16,
	        "First Fit devuelve el primer bloque de memoria que encuentre");
#endif
	free(var20);
	free(var17);
	free(var19);

	set_up();
	int *var4 = malloc(4190200);
	int *var5 = malloc(4190200);
	int *var6 = malloc(4190200);
	int *var7 = malloc(4190200);
	int *var8 = malloc(4190200);
	int *var9 = malloc(4190200);
	int *var10 = malloc(4190200);
	int *var11 = malloc(4190200);
	int *var12 = malloc(4190200);
	int *var13 = malloc(4190200);
	int *var14 = malloc(4190200);
	pa2m_afirmar(var13 != NULL && var14 == NULL && errno == ENOMEM,
	             "Si se pide mas que el limite de memoria, se devuelve "
	             "NULL y errno = ENOMEM");
	free(var4);
	free(var5);
	free(var6);
	free(var7);
	free(var8);
	free(var9);
	free(var10);
	free(var11);
	free(var12);
	free(var13);
	pa2m_afirmar(amount_of_mallocs == 10 && amount_of_frees == 10,
	             "Se liberan todos los bloques de memoria");

	pa2m_terminar_grupo();
}

void
pruebas_free()
{
	pa2m_nuevo_grupo("PRUEBAS BORDE FREE");
	set_up();
	int *var1 = NULL;
	free(var1);
	pa2m_afirmar(amount_of_frees == 0,
	             "Si se pasa NULL a free, no se hace nada");

	set_up();
	int in_stack = 5;
	int *var2 = &in_stack;
	free(var2);
	pa2m_afirmar(amount_of_frees == 0, "Si se pasa algo que no es un puntero al heap a free, no se hace nada");

	set_up();
	int *var3 = malloc(50);
	free(var3);
	free(var3);
	pa2m_afirmar(
	        amount_of_frees == 1,
	        "Si se pasa un puntero que ya fue liberado, no se hace nada");

	pa2m_terminar_grupo();
}

void
pruebas_calloc()
{
	pa2m_nuevo_grupo("PRUEBAS CALLOC");

	set_up();
	char *var1 = malloc(200 * sizeof(char));
	memcpy(var1, TEST_STRING, strlen(TEST_STRING));
	char *var2 = malloc(200 * sizeof(char));
	memcpy(var2, TEST_STRING, strlen(TEST_STRING));
	free(var2);
	char *var3 = calloc(200, sizeof(char));
	bool is_all_zero = true;
	for (int i = 0; i < 200; i++) {
		if (var3[i] != 0)
			is_all_zero = false;
	}
	pa2m_afirmar(var3 != NULL && is_all_zero && amount_of_mallocs == 3,
	             "Calloc inicializa todos los valores en 0");
	free(var3);
	free(var1);

	pa2m_terminar_grupo();
}

void
pruebas_realloc()
{
	pa2m_nuevo_grupo("PRUEBAS REALLOC");
	set_up();
	char *var1 = malloc(100);
	strcpy(var1, "PRUEBA");
	char *var2 = realloc(var1, 200);
	pa2m_afirmar(var2 == var1 && amount_of_mallocs == 1 &&
	                     amount_of_frees == 0 && strcmp(var2, "PRUEBA") == 0,
	             "Realloc agranda la region de memoria y devuelve el mismo "
	             "puntero");
	free(var2);

	set_up();
	char *var3 = malloc(100);
	strcpy(var3, "PRUEBA");
	char *var4 = realloc(var3, 4046);
	pa2m_afirmar(var3 != var4 && amount_of_mallocs == 2 &&
	                     amount_of_frees == 1 && strcmp(var4, "PRUEBA") == 0,
	             "Realloc agranda la region de memoria y devuelve un "
	             "puntero distinto");
	free(var4);

	set_up();
	char *var5 = malloc(100);
	strcpy(var5, "PRUEBA");
	char *var6 = realloc(var5, 0);
	pa2m_afirmar(var6 == NULL && amount_of_mallocs == 1 && amount_of_frees == 1,
	             "Realloc libera la memoria si el nuevo tamanio es 0");

	set_up();
	char *var7 = malloc(100);
	strcpy(var7, "PRUEBA");
	char *var8 = realloc(var7, 1000000000);
	pa2m_afirmar(
	        var8 == NULL && amount_of_frees == 0 && strcmp(var7, "PRUEBA") == 0,
	        "Si se pide mas que el limite de memoria, se devuelve NULL y "
	        "errno = ENOMEM, el puntero viejo sigue funcionando");
	free(var7);

	pa2m_terminar_grupo();
}

int
main(void)
{
#ifndef BEST_FIT
#ifndef FIRST_FIT
	printfmt("Error, no se definio BEST_FIT o FIRST_FIT\n");
	return 0;
#endif
#endif
	pa2m_iniciar();
	pruebas_malloc();
	pruebas_free();
	pruebas_calloc();
	pruebas_realloc();
	pa2m_mostrar_reporte();
	set_up();
	return 0;
}