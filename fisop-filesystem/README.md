# fisopfs

Sistema de archivos tipo FUSE.

## Estructuras de datos

Las estructuras creadas por nosotros para este sistema de archivos se componen por:

Estructura file_t, representa un archivo, tiene datos de tamaño estático sobre el archivo y un puntero a un contenido (array de caracteres).

Estructura dir_t, representa un directorio, tiene datos de tamaño estático sobre el directorio y un array de punteros a un tipo file_t.

Estructura requestPath_t, representa un path requerido por el usuario de hasta una profundidad de 2 (un archivo dentro de un directorio) que facilita la lectura del mismo.

El filesystem contiene un directorio raíz(de tipo dir_t) que contiene todos los archivos en la raíz, y un array de punteros a directorios que representan los directorios que se encuentran dentro del directorio raíz.

## Como se encuentra un archivo dado un path

En primer lugar creamos una función auxiliar que determina si el path tiene un nivel de recursión acorde a nuestro filesystem (es un archivo o directorio en la raíz, o un archivo en la raíz). Si es así se "parsea" a un tipo requestPath_t que contiene el nombre de uno o dos niveles de profundidad. También contiene un bool indicando si el path está en el directorio raíz.

Una vez que tenemos el path parseado, se busca el directorio solicitado. En caso de que sea la raíz ya tenemos el dir_t correspondiente, sino se buscará en el array de directorios. Una vez que tenemos el dir_t iteraremos por todo su array de archivos buscando aquel que corresponda al pedido.
En caso de no encontrar un directorio o archivo que corresponda al pedido se devuelve un error ENOENT.

## Statelessness

Como desición para el sistema de archivos optamos por que sea stateless, lo cual significa que no se guarda información sobre el estado de las acciones del usuario. Por ejemplo, si opta por abrir el archivo, realmente no está abriendo nada, solo verifica que existe y posterior a esto se pierde el seguimiento de esta acción. Si opta por escribir sobre un archivo, se usará el path para determinar de que archivo se trata y se escribirá sobre el mismo, no se usarán punteros especiales almacenados en memoria que representen un archivo abierto. Esto nos permite que el sistema de archivos sea más simple y que no se requiera de un estado para el mismo.

## Serialización

Al inicial el sistema de archivos se intentará acceder al archivo indicado por el usuario. Importante: es nuestro sistema el que agrega la extensión del mismo. En caso de no ser indicado se optará por el archivo por defecto. En caso de existir se leerán las estructuras de tamaño estático en un orden especifico (directorio raíz, array de directorios en la raíz y por último el tope del array). Una vez hecho esto se iterarán las estructuras que contienen punteros, se calculará el tamaño de los mismos(los archivos almacenan su tamaño y el file_t tiene un tamaño fijo) y se leerán los datos correspondientes. Esta información se irá almacenando en el heap, y se colocarán estos nuevos punteros en las estructuras correspondientes.

Al finalizar el sistema de archivos se serializarán las estructuras de tamaño estático en el mismo orden que se leyeron. Luego se iterarán las estructuras que contienen punteros, se serializarán los datos correspondientes en el mismo orden que deben ser leídas.

## Pruebas

En todas las pruebas se asume que la carpeta está igual que en el repositorio.

### Prueba 1 (Creación de archivos, Lectura de archivos, Escritura de archivos, Persistencia)

```
make
mkdir prueba
./fisopfs prueba/ -f
```

En otra terminal:
```
cd prueba
touch prueba.txt
echo "probando" > prueba.txt
cd ..
sudo umount prueba/
cat prueba.txt
```

Volver a la primera terminal:
```
./fisopfs prueba/ -f
```


En otra terminal:
```
cd prueba
cat prueba.txt
cd ..
sudo umount prueba/
```

Esperado en primera terminal:
```
[debug] Error al abrir el archivo file.fisopfs
[debug] Guardando el sistema de archivos file.fisopfs
[debug] Guardando el sistema de archivos file.fisopfs
[debug] truncate(/prueba.txt, 0)
[debug] Guardando el sistema de archivos file.fisopfs
[debug] Writing Buff /prueba.txt, offset 0
[debug] Guardando el sistema de archivos file.fisopfs
[debug] Guardando el sistema de archivos file.fisopfs
[debug] Destroying filesystem in memory

[debug] Cargando el sistema de archivos file.fisopfs
[debug] fisopfs_read_buf(/prueba.txt, 0, 4096)
[debug] Guardando el sistema de archivos file.fisopfs
[debug] Guardando el sistema de archivos file.fisopfs
[debug] Destroying filesystem in memory
```

Esperado en segunda terminal:
```
at prueba.txt
cat: prueba.txt: No such file or directory
at prueba.txt 
probando
```

### Prueba 2 (Creación de directorios, Lectura de directorios)

```
make
mkdir prueba
./fisopfs prueba/ -f
```

En otra terminal:
```
cd prueba
mkdir directorio
cd ..
ls prueba
sudo umount prueba/
ls prueba
```


Esperado en primera terminal:
```
[debug] Error al abrir el archivo file.fisopfs
[debug] fisopfs_read_buf(/prueba.txt, 0, 4096)
[debug] Guardando el sistema de archivos file.fisopfs
[debug] Guardando el sistema de archivos file.fisopfs
[debug] Destroying filesystem in memory
```

Esperado en segunda terminal:
```
directorio
```


### Prueba 3 (Borrado de un archivo, Borrado de un directorio)

```
make
mkdir prueba
./fisopfs prueba/ -f
```

En otra terminal:
```
cd prueba
mkdir directorio
touch directorio/prueba.txt
cd ..
ls prueba/directorio
cd prueba
rm -f directorio/prueba.txt
cd ..
ls prueba/directorio
rmdir prueba/directorio
ls prueba
sudo umount prueba/
```

Esperado en primera terminal:
```
[debug] Error al abrir el archivo file.fisopfs
[debug] Guardando el sistema de archivos file.fisopfs
[debug] Guardando el sistema de archivos file.fisopfs
[debug] Guardando el sistema de archivos file.fisopfs
[debug] Destroying filesystem in memory
```

Esperado en segunda terminal:
```
prueba.txt
```

### Prueba 4 (Estadísticas)

```
make
mkdir prueba
./fisopfs prueba/ -f
```

En otra terminal:
```
cd prueba
touch prueba.txt
stat prueba.txt
cd ..
sudo umount prueba/
```

Esperado en primera terminal:
```
[debug] Cargando el sistema de archivos file.fisopfs
[debug] Guardando el sistema de archivos file.fisopfs
[debug] Guardando el sistema de archivos file.fisopfs
[debug] Guardando el sistema de archivos file.fisopfs
[debug] Destroying filesystem in memory
```

Esperado en segunda terminal:
```
  File: prueba.txt
  Size: 1         	Blocks: 0          IO Block: 4096   regular file
Device: 2ch/44d	Inode: 2           Links: 1
Access: (0664/-rw-rw-r--)  Uid: ( 1000/   user)   Gid: (    0/    root)
Access: 2022-12-11 20:47:21.000000000 -0300
Modify: 2022-12-11 20:47:21.000000000 -0300
Change: 2022-12-11 20:47:21.000000000 -0300
 Birth: -
 ```
 (Va a variar según donde/cuando se haya corrido la prueba)
