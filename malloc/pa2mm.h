#ifndef __PA2M_H_
#define __PA2M_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "printfmt.h"

#define BLANCO "\x1b[37;1m"
#define VERDE "\x1b[32;1m"
#define ROJO "\x1b[31;1m"
#define AMARILLO "\x1b[33;1m"
#define NORMAL "\x1b[0m"

#define TILDE "✓"
#define CRUZ "✗"

#define CARACTERES_PRUEBA 130
#define UNUSED(x) (void)(x)

int __pa2m_cantidad_de_pruebas_corridas = 0;
int __pa2m_cantidad_de_pruebas_fallidas = 0;
const char* __pa2m_prueba_actual = NULL;

void cuadro_inicio(void);
void cuadro_fin(void);
void cuadro_techo(void);
void cuadro_piso(void);
void cuadro_separacion(void);
void pa2m_nuevo_grupo(const char* nombre);
void pa2m_nueva_prueba(const char* nombre);
int pa2m_mostrar_reporte(void);
void pa2m_separar(void);
void pa2m_iniciar(void);
void pa2m_terminar_grupo(void);
void __pa2m_atajarse(void (*handler)(int));
void __pa2m_morir(int signum);

void cuadro_inicio() {
  printfmt(AMARILLO"╔");
  for(int i = 0; i < CARACTERES_PRUEBA + 6; i++)
    printfmt("═");
  printfmt("╗\n"BLANCO);
}

void cuadro_fin() {
  printfmt(AMARILLO"╚");
  for(int i = 0; i < CARACTERES_PRUEBA + 6; i++)
    printfmt("═");
  printfmt("╝\n"BLANCO);
}

void cuadro_techo() {
  printfmt(AMARILLO"╠═══╦");
  for(int i = 0; i < CARACTERES_PRUEBA + 2; i++)
    printfmt("═");
  printfmt("╣\n"BLANCO);
}

void cuadro_piso() {
  printfmt(AMARILLO"╠═══╩");
  for(int i = 0; i < CARACTERES_PRUEBA + 2; i++)
    printfmt("═");
  printfmt("╣\n"BLANCO);
}

void cuadro_separacion() {
  printfmt(AMARILLO"╠═══╬");
  for(int i = 0; i < CARACTERES_PRUEBA + 2; i++)
    printfmt("═");
  printfmt("╣\n"BLANCO);
}

void __pa2m_atajarse(void (*handler)(int)) {
  signal(SIGABRT, handler);
  signal(SIGSEGV, handler);
  signal(SIGBUS, handler);
  signal(SIGILL, handler);
  signal(SIGFPE, handler);
}

void __pa2m_morir(int signum) {
  UNUSED(signum);
  printfmt(BLANCO
     "         _nnnn_                        \n"
     "        dGGGGMMb     ,''''''''''''''.  \n"
     "       @p~qp~~qMb    |    ERROR!    |  \n"
     "       M|@||@) M|   _;..............'  \n"
     "       @,----.JM| -'                   \n"
     "      JS^\\__/  qKL                    \n"
     "     dZP        qKRb                   \n"
     "    dZP          qKKb                  \n"
     "   fZP            SMMb                 \n"
     "   HZM            MMMM                 \n"
     "   FqM            MMMM                 \n"
     " __| '.        |\\dS'qML               \n"
     " |    `.       | `' \\Zq               \n"
     "_)      \\.___.,|     .'               \n"
     "\\____   )MMMMMM|   .'                 \n"
     "     `-'       `--'                    \n"
     "\n");
  if(__pa2m_prueba_actual)
    printfmt(ROJO "\n\nERROR MIENTRAS SE EJECUTABA LA PRUEBA: " BLANCO "'%s'\n\n" BLANCO, __pa2m_prueba_actual);
  else printfmt(ROJO "\n\nFINALIZACION ANORMAL DE LAS PRUEBAS\n\n"BLANCO);
  __pa2m_atajarse(SIG_DFL);
}

#define pa2m_afirmar(afirmacion, descripcion) do {                                             \
    __pa2m_prueba_actual = descripcion;                                                        \
    __pa2m_atajarse(__pa2m_morir);                                                             \
    if (afirmacion) {                                                                          \
      printfmt(AMARILLO "║ " VERDE TILDE AMARILLO " ║" BLANCO);                                  \
    } else {                                                                                   \
      __pa2m_cantidad_de_pruebas_fallidas++;                                                   \
      printfmt(AMARILLO "║ " ROJO CRUZ AMARILLO " ║" BLANCO);                                    \
    }                                                                                          \
    printfmt(BLANCO " %-*s "AMARILLO "║\n" BLANCO, CARACTERES_PRUEBA, __pa2m_prueba_actual);     \
    __pa2m_prueba_actual = NULL;                                                               \
    __pa2m_cantidad_de_pruebas_corridas++;                                                     \
  }while(0);

void pa2m_nuevo_grupo(const char* descripcion) {
  printfmt(AMARILLO "║ "BLANCO "%-*s " AMARILLO "║\n" BLANCO, CARACTERES_PRUEBA + 4, descripcion);
  cuadro_techo();
}

void pa2m_iniciar() {
  cuadro_inicio();
}

void pa2m_terminar_grupo() {
  cuadro_piso();
}

void pa2m_separar() {
  cuadro_separacion();
}

int pa2m_mostrar_reporte() {
  printfmt(AMARILLO "║ " BLANCO
         "%.3i pruebas corridas, %.3i errores - %*s " AMARILLO "║\n" BLANCO NORMAL,
         __pa2m_cantidad_de_pruebas_corridas,
         __pa2m_cantidad_de_pruebas_fallidas,
         -(CARACTERES_PRUEBA-32),
         __pa2m_cantidad_de_pruebas_fallidas == 0 ? "OK" : "D:");
  cuadro_fin();
  return __pa2m_cantidad_de_pruebas_fallidas;
}

#endif // __PA2M_H_
