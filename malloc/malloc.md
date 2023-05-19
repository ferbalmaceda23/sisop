# TP: malloc

En esta implementación de malloc utilizamos la syscall mmap para pedirle al sistema operativo un bloque de memoria en el heap. Luego, dividimos ese bloque en regiones de tamaño variable donde cada una tiene un puntero a la siguiente como una lista enlazada. De ser necesario se pueden pedir más bloques (cada uno con sus propias regiones y un tamaño predeterminado Chico/Mediano/Grande), y estos también estarán como una lista enlazada. Cuando se pide memoria, se busca dentro de los bloques una región disponible (ya sea mediante FIRST FIT o BEST FIT), y cuando se libera memoria, se realiza un coalesce entre todas las regiones libres. En caso de que un bloque haya quedado con una región libre que ocupe todo el bloque se libera el bloque completo.

La estructura de datos que utilizamos para representar un bloque es la siguiente:

```c
struct block {
	size_t size;
	struct block *next;
	struct region *region;
};
```

La estructura de datos que utilizamos para representar una región es la siguiente:

```c
struct region {
	bool free;
	size_t size;
	struct region *next;
};
```

Como se observa, la región tiene un puntero a la siguiente región, y la estructura block tiene un puntero a la primera región que le pertenece. Esto nos permite recorrer las regiones de un bloque facilmente, parandonos en la primera y accediendo a la siguiente mediante el puntero next de la región.

Tenemos como variable global un puntero al primer bloque reservado. Esto nos permite recorrer todos los bloques, de la misma manera que lo hacemos con las regiones.

En la memoria, la estructura que tendría un bloque vacío es la siguiente

|==========================================================|
| Block Header | Region Header | Region Space              |
|==========================================================|

La parte en la que debimos realizar un coalesce la implementamos de la siguiente manera:

Para cada bloque, para cada región, iniciando por la primera. Si la región está libre, se fija si la siguiente región también está libre. Si es así, se unen ambas regiones en una sola, y se actualiza el puntero next de la región anterior a la siguiente de la región que se acaba de unir. Si la siguiente región no está libre, se pasa a la siguiente región. De esta manera todas las regiones libres contiguas quedarán unidas.

La parte en la que debiamos realizar splitting la implementamos de la siguiente manera:

Para una región libre, si el tamaño de la región es mayor al tamaño que se pide, se crea una nueva región con el tamaño que sobra, y se actualiza el puntero next de la región libre para que apunte a la nueva región (contenida en lo que antes era parte de la región original). Luego, se inicializa este nuevo header y se actualiza el tamaño de su región para que sea el tamaño que se pide.

## Ejecución

Para compilar el programa, ejecutar `make` indicando el flag USE_FF o USE_BF. Para ejecutar el programa, ejecutar `./malloc`. Para limpiar los archivos generados por el programa, ejecutar `make clean`.

## Análisis

Para analizar el comportamiento de la implementación, se realizaron pruebas con distintos tamaños de memoria pedidos, y distintos tamaños de bloques. Se verificó que las estadisticas sean correctas, y que el programa no tenga memory leaks.
