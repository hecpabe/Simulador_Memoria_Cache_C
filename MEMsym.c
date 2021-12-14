

/*
    Título: Proyecto Memoria Caché
    Nombre: Héctor Paredes Benavides
    Descripción: Creamos un programa que lea el contenido de dos ficheros, el primero contiene el conjunto de posiciones de memoria ram
                a leer y el segundo el contenido de esa memoria, se crea un programa que con la entrada de estos dos datos simule el 
                comportamiento de una memoria caché de correspondencia directa
    Fecha: 3/12/2021
*/

/* Instrucciones de Preprocesado */
// Inclusión de bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

// Definición de constantes
#define TAM_LINEA 16                            // Tamaño de línea / Número de palabras por línea
#define NUM_FILAS 8                             // Número de líneas de la caché
#define TAM_CONTENIDO_MEMORIA_RAM 4096          // Tamaño del contenido de la memoria RAM
#define TAM_DIRECCIONES_HEXADECIMAL 3           // Tamaño en nibbles de una dirección de memoria
#define LONG_PALABRA 4                          // Tamaño en bits de la palabra
#define LONG_LINEA 3                            // Tamaño en bits de la línea
#define INCREMENTO_TIEMPO_SIMULACION 1          // Número de unidades en las que aumenta el tiempo de simulación al realizar una operación de acceso
#define INCREMENTO_TIEMPO_SIMULACION_FALLO 10   // Número de unidades en las que aumenta el tiempo de simulación al obtener un fallo de caché

/* Declaraciones Globales */
// Estructura en la que almacenamos una línea de la memoria caché, se compone de la etiqueta y de las 16 palabras de la línea de la caché
typedef struct T_CACHE_LINE{
    unsigned char ETQ;
    unsigned char Data[TAM_LINEA];
}T_CACHE_LINE;

// Estructura en la que almacenamos una dirección de memoria ya parseada
typedef struct T_DIRECCION_MEMORIA{
    int ETQ;
    int palabra;
    int linea;
    int bloque;
}T_DIRECCION_MEMORIA;

// Prototipado de funciones pedidas por el enunciado de la práctica
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]);                                             // Función con la que inicializamos por defecto la caché con etiqueta 0xFF y datos 0x23 al inicio del programa
void VolcarCACHE(T_CACHE_LINE *tbl);                                                        // Función con la que mostramos el contenido de la memoria caché por pantalla
void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque);  // Función con la que transformamos una dirección de memoria en formato decimal a una dirección de memoria en formato de la estructura que hemos creado para ello
void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque);            // Función con la que tratamos un fallo de la memoria caché, cargamos en la línea correspondiente el bloque correspondiente que ha causado el fallo

// Prototipado de funciones creadas por mí para conseguir el resultado esperado de la práctica
char *leeLineaDinamica(FILE *fichero);                                                                  // Función con la que leemos dinámicamente una línea de un fichero de texto
char **leeFicheroDinamicamente(char *nombreFichero, int *numeroLineas);                                 // Función con la que leemos dinámicamnete un fichero de texto y lo guardamos en un array de cadenas de caracteres
void cargaContenidoMemoriaRAM(char memoriaRAM[TAM_CONTENIDO_MEMORIA_RAM], char *nombreFichero);         // Función con la que leemos el contenido del fichero binario del contenido de la memoria RAM y lo volcamos en el array de caracteres pasado por argumento
unsigned int transformaDireccionDeStringAEntero(char *direccionString, int tamDireccionesHexadecimal);  // Función con la que transformamos una dirección de memoria leída del fichero como cadena de caracteres a decimal
void escribeCacheEnFichero(T_CACHE_LINE *memoriaCache, char *nombreFichero);                            // Función con la que volcamos el contenido de la memoria caché en un fichero binario pasado como argumento

/* Función Principal Main */
void main(int argc, char **argv){

    // Declaración de variables
    unsigned int globalTime = 0;    // Variable en la que almacenaremos el tiempo que tarda la simulación en acceder a las posiciones de memoria
    unsigned int numeroFallos = 0;  // Variable con la que contaremos el número de fallos de caché que se dan en la simulación
    char texto[100];                // Variable en la que almacenaremos el texto que vamos leyendo de la memoria

    char **contenidoFicheroDireccionesMemoria = NULL;   // Variable en la que volcaremos el contenido del fichero de direcciones de memoria a las que acceder
    int numeroDireccionesMemoria = 0;                   // Variable en la que volcaremos el número de direcciones de memoria a las que tenemos que acceder

    char Simul_RAM[TAM_CONTENIDO_MEMORIA_RAM];  // Variable en la que almacenaremos el contenido del fichero binario que alberga el contenido de la memoria RAM

    T_CACHE_LINE memoriaCache[NUM_FILAS];   // Variable en la que almacenaremos el contenido de la memoria caché

    // Comprobamos si el usuario nos ha pasado como argumento del programa los 3 ficheros que necesita el programa, el de las direcciones de memoria, el contenido de la memoria RAM y el fichero donde volcaremos el contenido de la memoria caché (por ese orden)
    if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL){
        printf("\nERROR: No se han introducido los ficheros necesarios para la ejecución del programa (contenido de la ram, fichero con las posiciones de memoria a las que acceder y fichero binario donde volcar el contenido de la caché una vez finalizado el programa).");
        exit(-1);
    }

    // Cargamos el contenido de los 2 primeros ficheros en el programa mediante las funciones que hemos creado con tal objeto
    contenidoFicheroDireccionesMemoria = leeFicheroDinamicamente(argv[1], &numeroDireccionesMemoria);
    cargaContenidoMemoriaRAM(Simul_RAM, argv[2]);

    // Limpiamos la memoria caché
    LimpiarCACHE(memoriaCache);

    // Recorremos las direcciones de memoria recogidas del fichero
    for(int i = 0; i < numeroDireccionesMemoria; i++){

        // Incrementamos el tiempo de ejecución
        globalTime += INCREMENTO_TIEMPO_SIMULACION;

        // Parseamos la dirección
        T_DIRECCION_MEMORIA nuevaDireccion;
        int nuevaDireccionEntera = transformaDireccionDeStringAEntero(contenidoFicheroDireccionesMemoria[i], TAM_DIRECCIONES_HEXADECIMAL);
        ParsearDireccion(nuevaDireccionEntera, &nuevaDireccion.ETQ, &nuevaDireccion.palabra, &nuevaDireccion.linea, &nuevaDireccion.bloque);

        // Si la misma no se encuentra cargada en caché incrementamos los fallos, mostramos al usuario el fallo de caché y realizamos el algoritmo para resolverlo
        if(nuevaDireccion.ETQ != memoriaCache[nuevaDireccion.linea].ETQ){
            numeroFallos++;
            printf("\nT: %d, Fallo de CACHE %d, ADDR %04X Label %X linea %02X palabra %02X bloque %02X", globalTime, numeroFallos, nuevaDireccionEntera, nuevaDireccion.ETQ, nuevaDireccion.linea, nuevaDireccion.palabra, nuevaDireccion.bloque);
            TratarFallo(memoriaCache, Simul_RAM, nuevaDireccion.ETQ, nuevaDireccion.linea, nuevaDireccion.bloque);
            globalTime += INCREMENTO_TIEMPO_SIMULACION_FALLO;
        }

        // Mostramos el acierto de la caché y recogemos el caracter obtenido
        printf("\nT: %d, Acierto de CACHE, ADDR %04X Label %X linea %02X palabra %02X DATO %02X", globalTime, nuevaDireccionEntera, nuevaDireccion.ETQ, nuevaDireccion.linea, nuevaDireccion.palabra, memoriaCache[nuevaDireccion.linea].Data[nuevaDireccion.palabra]);
        texto[i] = memoriaCache[nuevaDireccion.linea].Data[nuevaDireccion.palabra];

        // Mostramos el contenido de la memoria caché
        VolcarCACHE(memoriaCache);

        printf("\n");

        sleep(1);

    }

    // Al acabar de recorrer todas las direcciones de memoria mostramos las estadísticas pedidas en la práctica
    printf("\nESTADÍSTICAS:\n-Accesos totales: %d\n-Número de fallos: %d\n-Tiempo medio de acceso: %0.2f unidades de tiempo\n-Texto leído: ", numeroDireccionesMemoria, numeroFallos, (globalTime / (float)numeroDireccionesMemoria));
    for(int i = 0; i < numeroDireccionesMemoria; i++)
        printf("%c", texto[i]);

    // Limpiamos toda la memoria reservada dinámicamente por la lectura del fichero de direcciones de memoria
    for(int i = 0; i < numeroDireccionesMemoria; i++){
        free(contenidoFicheroDireccionesMemoria[i]);
    }

    free(contenidoFicheroDireccionesMemoria[numeroDireccionesMemoria]);
    free(contenidoFicheroDireccionesMemoria);

    // Volcamos el contenido de la memoria caché en el fichero pasado por argumento al programa
    escribeCacheEnFichero(memoriaCache, argv[3]);

}

/* Codificación de Funciones */
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]){

    // Recorremos todas las líneas de la caché estableciendo como etiqueta 0xFF y valores a las palabras de la línea 0x23
    for(int i = 0; i < NUM_FILAS; i++){

        tbl[i].ETQ = 0xFF;

        for(int j = 0; j < TAM_LINEA; j++)
            tbl[i].Data[j] = 0x23;

    }

}

void VolcarCACHE(T_CACHE_LINE *tbl){

    // Recorremos todas las líneas de la caché mostrando su etiqueta y el contenido de las palabras de las mismas
    for(int i = 0; i < NUM_FILAS; i++){

        printf("\nETQ:%02X  Data", tbl[i].ETQ);
        for(int j = TAM_LINEA - 1; j >= 0; j--)
            printf(" %02X", tbl[i].Data[j]);

    }

}

void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque){

    // Declaramos las máscaras en función de los datos dados por la práctica
    unsigned int mascaraPalabra = 0b000000001111;
    unsigned int mascaraLinea = 0b000001110000;
    unsigned int mascaraEtiqueta = 0b111110000000;

    // Traducimos todos los datos de una dirección de memoria al formato de nuestra estructura
    *ETQ = (addr & mascaraEtiqueta) >> (LONG_PALABRA + LONG_LINEA);
    *palabra = addr & mascaraPalabra;
    *linea = (addr & mascaraLinea) >> LONG_PALABRA;
    *bloque = addr >> LONG_PALABRA;

}

void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque){

    // Establecemos la nueva etiqueta y las nuevas palabras en la fila que haya que tratar de la caché
    tbl[linea].ETQ = ETQ;

    for(int i = 0; i < TAM_LINEA; i++)
        tbl[linea].Data[i] = MRAM[bloque + i];

}

char *leeLineaDinamica(FILE *fichero){

    // Variables en las que se van a almacenar la cadena de caracteres (línea) leída, el caracter leído actualmente y el índice de la cadena en el que nos encontramos
    char *cadenaCaracteres = NULL;
    char caracter = '\0';
    int indice = 0;
    
    // Reservamos memoria para la cadena de caracteres
    cadenaCaracteres = (char*)malloc(sizeof(char));
    cadenaCaracteres[0] = caracter;

    // Mientras que no nos encontremos un retorno de carro o un fin de fichero leemos cada caracter del fichero
    while((caracter != '\n') && (!feof(fichero))){

            caracter = getc(fichero);

            // Si nos encontramos un un retorno de caro, un \r o el final del fichero insertamos un final de cadena
            if(caracter == '\r' || caracter == '\n' || feof(fichero))
                cadenaCaracteres[indice] = '\0';
            else{   // En caso contrario recogemos el caracter en la cadena, pasamos a la siguiente posición y reservamos más espacio
                cadenaCaracteres[indice] = caracter;
                indice++;
                cadenaCaracteres = (char*)realloc(cadenaCaracteres, sizeof(char) * (indice + 1));
                //printf("\nLEIDO CARACTER: %c", caracter);
            }

    }

    return cadenaCaracteres;

}

char **leeFicheroDinamicamente(char *nombreFichero, int *numeroLineas){

    // Variables en las que almacenaremos el fichero, la lectura que realicemos del mismo y el índice de línea por el que vamos
    FILE *fichero = NULL;
    char **lecturaFichero = (char**)malloc(sizeof(char*));
    int indiceLineaFichero = 0;

    // Abrimos el fichero
    fichero = fopen(nombreFichero, "r");

    // Si no hay fichero devolvemos un error
    if(fichero == NULL){
        free(lecturaFichero);
        printf("\nERROR: No se ha podido abrir el fichero %s", nombreFichero);
        exit(-1);
    }

    // Mientras que no nos encontremos el final del fichero leemos línea a línea reservando espacio para la siguiente
    while(!feof(fichero)){
        lecturaFichero[indiceLineaFichero] = leeLineaDinamica(fichero);
        if(lecturaFichero[indiceLineaFichero][0] != '\0')
            indiceLineaFichero++;
        lecturaFichero = (char**)realloc(lecturaFichero, sizeof(char*) * (indiceLineaFichero + 1));
    }

    // Pasamos el número de líneas que hemos leído
    *numeroLineas = indiceLineaFichero;

    // Cerramos el fichero
    fclose(fichero);

    return lecturaFichero;

}

void cargaContenidoMemoriaRAM(char memoriaRAM[TAM_CONTENIDO_MEMORIA_RAM], char *nombreFichero){

    FILE *fichero = fopen(nombreFichero, "rb");     // Fichero del que recogeremos los datos de la memoria RAM

    // Comprobamos que la apertura del fichero ha funcionado correctamente
    if(fichero == NULL){
        printf("\nERROR: Ha ocurrido un error al intentar cargar el archivo %s", nombreFichero);
        exit(-1);
    }

    // Volcamos el contenido del fichero en el array de caracteres correspondiente al almacenamiento del contenido de la memoria RAM
    fread(memoriaRAM, TAM_CONTENIDO_MEMORIA_RAM, 1, fichero);

    // Cerramos el fichero para no tenerlo bloqueado
    fclose(fichero);

}

unsigned int transformaDireccionDeStringAEntero(char *direccionString, int tamDireccionesHexadecimal){

    unsigned int resultado = 0;     // Variable en la que almacenaremos el resultado de la transformación de la dirección de memoria

    int valorNibble = 0;    // Variable auxiliar que utilizaremos para transformar a entero un nibble de la dirección de memoria
    int indiceInverso = 0;  // Índice que utilizaremos para recorrer inversamente la dirección de memoria

    // Recorremos la dirección de memoria
    for(int i = 0; i < tamDireccionesHexadecimal; i++){

        // La recorremos de forma inversa ya que los nibbles aumentan su valor significativo de derecha a izquierda
        indiceInverso = tamDireccionesHexadecimal - (i + 1);
        valorNibble = 0;

        // Traducimos el nibble de caracter a entero
        if(direccionString[indiceInverso] >= '0' && direccionString[indiceInverso] <= '9')
            valorNibble = direccionString[indiceInverso] - '0';
        else if(direccionString[indiceInverso] >= 'A' && direccionString[indiceInverso] <= 'F')
            valorNibble = (direccionString[indiceInverso] - 'A') + 10;

        // Lo multiplicamos por la base elevada a la posición en la que se encuentra para obtener su valor decimal final
        resultado += valorNibble * pow(16, i);

    }

    return resultado;

}

void escribeCacheEnFichero(T_CACHE_LINE *memoriaCache, char *nombreFichero){

    FILE *fichero = fopen(nombreFichero, "w+b");        // Fichero en el que volcaremos el contenido de la caché
    char contenidoMemoriaCache[TAM_LINEA * NUM_FILAS];  // Vector de caracteres en el que volcaremos el contenido de la caché para volcarlo a su vez en el fichero
    int indice = 0;                                     // Índice del vector anterior

    // Comprobamos que la apertura del fichero haya funcionado correctamente
    if(fichero == NULL){
        printf("\nERROR: Ha ocurrido un error al abrir el fichero para volcar el contenido de la memoria caché %s", nombreFichero);
        exit(-1);
    }

    // Recorremos todas las palabras de la memoria caché y volcamos su contenido en las posiciones pertinentes del vector de caracteres
    for(int i = 0; i < NUM_FILAS; i++){
        for(int j = 0; j < TAM_LINEA; j++){
            contenidoMemoriaCache[indice] = memoriaCache[i].Data[j];
            indice++;
        }
    }

    // Volcamos el contenido del vector de caracteres en el fichero
    fwrite(contenidoMemoriaCache, NUM_FILAS * TAM_LINEA, NUM_FILAS * TAM_LINEA, fichero);

    // Cerramos el fichero para no tenerlo bloqueado
    fclose(fichero);

}
