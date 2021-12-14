# Simulador de Memoria Caché en C
Programa en C que simula el comportamiento de una memoria caché

## Índice:
1) Información.
2) Requisitos.
3) Instalación y Ejecución.
4) Documentación de Funcionamiento del Programa
5) Créditos.

### Información:
Este programa se ha creado en C y compilado con GCC en la distribución Cygwin, se ha probado a compilarse y ejecutarse en ArchLinux, Ubuntu 20.04 y KaliLinux. En otras distribuciones no se asegura que funcione.

### Requisitos:
- Sistema Operativo Linux (probado en Arch, Kali, Ubuntu y Cygwin).
- GCC para compilar el archivo.

### Instalación y Ejecución:
1) Descargar o clonar el proyecto en un directorio de local.
2) Compilar el archivo MEMsym.c.
3) Ejecutar el programa pasándole como argumentos los siguientes ficheros:
- Ficheros:
  1) Fichero en el que se ven contenidas las direcciones de memoria las que se accede.
  2) Fichero en el que se encuentra el contenido de la memoria RAM.  
  3) Fichero en el que se quiere volcar el contenido de la memoria caché al finalizar la ejecución.  
  - Ejemplo para los ficheros del proyecto: ./MEMsym.exe accesos_memoria.txt CONTENTS_RAM.bin CONTENTS_CACHE.bin

4- Si se quiere volcar el resultado de la ejecución a modo de log se puede realizar con el siguiente comando ./MEMsym.exe accesos_memoria.txt CONTENTS_RAM.bin CONTENTS_CACHE.bin > FICHERO.txt

### Documentación de Funcionamiento del Programa:
El programa recibirá los nombres de los archivos por argumento al ejecutarse, en caso de no recibir alguno da error y finaliza la ejecución de forma segura. Después lee las direcciones de memoria redactadas en el primer fichero y las vuelca sobre un puntero doble de caracteres. Acto seguido vuelca el contenido del fichero de la memoria RAM en un array de 4096 caracteres y limpia la memoria caché. Después el programa recorre el puntero donde están almacenadas las direcciones de memoria traduciéndolas a nuestra estructura, comprueba si esa dirección está cargada en la caché, si no lo está trata la excepción, y una vez que la dirección se encuentre en la caché leemos el dato y editamos nuestras estadísticas. Finalmente por cada iteración mostramos el contenido de la memoria caché y antes de liberar la memoria y finaliza el programa mostramos las estadísticas obtenidas.

### Créditos:
**Autor: *Héctor Paredes Benavides*** *INSO 2C*
