# Trabajo Práctico - File System

Documentación de diseño

    Se deben explicar los distintos aspectos de diseño:
        Las estructuras en memoria que almacenarán los archivos, directorios y sus metadatos
        Cómo el sistema de archivos encuentra un archivo específico dado un path
        Todo tipo de estructuras auxiliares utilizadas
        El formato de serialización del sistema de archivos en disco (ver siguiente sección)
        Cualquier otra decisión/información que crean relevante



## Estructuras de memoria y Estructuras auxiliares

Para realizar el file system implementamos una estructura unix-like, con archivos y directorios. 
Para esto, creamos las siguientes estructuras:
1. Superblock:
```c
struct superblock {
	size_t block_size;
	size_t inode_size;
	uint8_t inodes_count;
	uint8_t data_count;
};
```
El Superblock consta principalmente de Metadata acerca del file system. Los campos del struct corresponden a:
- block_size: Tamaño de los bloques en bytes.
- inode_size: Tamaño de los inodos en bytes.
- inodes_count: Cantidad de inodos.
- data_count: Cantidad de bloques de datos.
2. Bitmap:
```c
struct bitmap {
    uint8_t *bit;
    size_t bound;
};
```
El bitmap es una estructura que nos permite llevar un registro de los bloques de datos e inodos que están ocupados y cuáles no.
- bit: Puntero a un array de bytes. Cada bit dentro de cada byte representa si el bloque está libre (0) o no (1).
- bound: Cantidad de bytes que tiene el bitmap.

3. Inode:
```c
struct inode {
	uid_t uid;
	gid_t gid;
	size_t content_size;
	char path[MAX_PATH_LENGTH];
	mode_t mode;
	uint32_t link_count;
	time_t time;  
	time_t ctime; 
	time_t mtime; 
	int64_t block_id;
};
```
El Inode es una estructura que contiene la metadata de un archivo o directorio junto con el bloque donde se almacena su información.
- uid: User ID del propietario del archivo.
- gid: Group ID del propietario del archivo.
- content_size: Tamaño del contenido del archivo.
- path: Path del archivo.
- mode: Permisos del archivo.
- link_count: Cantidad de enlaces duros al archivo.
- time: Último acceso del archivo.
- ctime: Tiempo de creación del archivo.
- mtime: Tiempo de modificación del archivo.
- block_id: ID del bloque donde se almacena la información del archivo.

4. FileSystem:
```c
struct file_system {
	struct superblock sb;
	struct bmap imap;
	struct bmap dmap;
	struct inode *inodes[INODE_BLOCKS];
	char **data_blocks;
};
```
Wrapper de todas las estructuras mencionadas anteriormente. Se apuntó a que todo el FS esté empaquetado de manera lineal en memoria.

## Búsqueda de archivos

Para buscar un archivo se recorren todos los inodos del FS y se compara el path del archivo con el path asignado al inodo. 
Si se encuentra un inodo con el mismo path, se retorna un puntero a la estructura inodo correspondiente. 
En caso de no encontrarlo, se retorna NULL.

## Serialización/Persistencia de datos

Para la serialización de los datos en disco se realiza la siguiente dinámica (En orden):
1. El Superblock no se escribe en disco, ya que su metadata es constante.
2. Los bitmaps se escriben en el archivo a partir de los bytes consecutivos que los componen. Esto permite recuperar la cadena de bits.
3. Los inodos se escriben en disco de una manera similar, se concatena todo el array de inodos de manera consecutiva al archivo.
4. Finalmente, los bloques de datos se recorren y escriben uno por uno en el archivo.

La deserialización se realiza de la misma manera, ya que se lee el archivo en el mismo orden que se escribió.

