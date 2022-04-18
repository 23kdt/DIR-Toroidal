/****************************************
* 
* - Autor: Diego Dorado Galán
* - Diseño de infraestructura de red
* - P1: Diseño de una red toroide
* - Fecha: 17/04/2022
*   (Todas las fechas se refieren a la última modificación)
* 
**************************************/

/* ********************* BIBLIOTECAS ************************** */

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* *********** CONSTANTES Y VARIABLES GLOBALES***************** */

#define FICHERO "datos.dat"
#define MAX_SIZE 1024
#define L 4
#define n (L*L)
#define TRUE 1
#define FALSE 0

/**************** FUNCIONES AUXILIARES ************************/

int leerdatos(double numeros[]);
void getVecinos(int rank, int vecinos[]);
double getMinimo(int rank, double buffer, int vecinos[]);
void enviarDatos(double *numeros);

/**************** FUNCION PRINCIPAL ***************************/

/* Intentar modularizar más */

int main(int argc, char *argv[]){

    int rank, size;

    MPI_Status status;

    /* Nodos (rank) vecinos de un rank */
    /* ¡¡¡NOTA!!! Intentar implementar con vectores para mayor claridad */
    int vecinoNorte, vecinoSur, vecinoIzq, vecinoDch;
    int vecinos[L];
    /* Por ejemplo, vecinos[0]-> vecinoNorte 
                    vecinos[1]-> vecinoSur
                    vecinos[2]-> vecinoIzq
                    vecinos[3]-> vecinoDch */

    /* Mínimo */
    double min;
    /* Buffer que almacenará el número de cada nodo */
    double buffer;

    /* Numeros leidos que compararemos con el tamaño para comprobar si se ha leído completamente o existen suficientes datos */
    int leidos;
    /* Array que almacenará los numeros */
    double numeros[n];

    int condicion = TRUE;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* Primero debemos comprobar si es el rank 0 */
    if(rank == 0){

        /* Se debe comprobar si se lanza un número de procesos adecuado para un toroide de lado L */
        if(size == n){

            /* (Borrar)IMPORTANTE!! Errores en los procesos (signal 6) por no ponerlo */
            /* Asignamos un espacio de memoria al array */
            double *numeros = malloc(size * sizeof(double));
            
            /* Leemos el fichero y almacenamos el tamaño del vector */
            leidos = leerdatos(numeros); 

            /* Debemos comprobar además si existen suficientes datos parar satisfacer los nodos del toroide
               Enviamos los datos del fichero a los procesos */
            if(n == size){
                
                /*  El rank 0 pasa 1 entero (condicion) a través del comunicador */
                MPI_Bcast(&condicion,1,MPI_INT,0,MPI_COMM_WORLD);
                
                /* Función por la que el rank 0 envía los datos al resto de nodos */
                enviarDatos(numeros);
                
                condicion=TRUE; 
                free(numeros);
            }
            else {

                condicion = FALSE;
                fprintf(stderr, "El número de datos leídos del fichero (%d) no es suficiente. Se necesitan %d\n", leidos, n);
                
                /* Realizamos un proceso similar al caso anterior */
                MPI_Bcast(&condicion,1,MPI_INT,0,MPI_COMM_WORLD);
            }
        
        }

        else {
            
            /* Realizaremos un broadcast para notificar al resto de procesos que aborten la ejecución */
            condicion = FALSE;
            fprintf(stderr,"Tenemos que ejecutar %d procesos para un toroide de lado %d \n",n,L);

            /*  El rank 0 pasa 1 entero (condicion) a través del comunicador */
            MPI_Bcast(&condicion,1,MPI_INT,0,MPI_COMM_WORLD);
            
        }
    }

    /* Con una llamada colectiva nos aseguramos un punto de ejecución común que indique a los nodos que pueden continuar */
    MPI_Bcast(&condicion,1,MPI_INT,0,MPI_COMM_WORLD);

    if(condicion==TRUE){

        MPI_Recv(&buffer,1,MPI_DOUBLE,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);

        getVecinos(rank, vecinos);

        min = getMinimo(rank,buffer, vecinos);

        if(rank == 0)
            printf("El menor número encontrado en la red es %.5lf\n",min);

    }

    MPI_Finalize();
    return 0;
}


/************************ FUNCIONES AUXILIARES *************************/


void enviarDatos(double *numeros){
    int i;
    double buffer;

    for(i=0;i<n;i++){
        buffer = numeros[i];
        /* El rank 0 envía al resto de nodos (y a sí mismo) los datos */
        /* PREGUNTAR: Por qué no deja poner MPI_ANY_TAG */
        MPI_Send(&buffer,1,MPI_DOUBLE,i,0,MPI_COMM_WORLD);
        printf("%5.lf enviado al nodo %d \n",buffer,i);
    }
}


/**************
 * 
 * Autor: Diego Dorado Galán
 * 
 * Fecha: 13/04/2022
 * 
 * Fuentes: 
 *  - https:/* www.delftstack.com/es/howto/c/strtok-in-c/ (guía strtok)
 *  - https:/* stackoverflow.com/questions/3889992/how-does-strtok-split-the-string-into-tokens-in-c
 *  - https:/* docs.microsoft.com/es-es/cpp/c-runtime-library/reference/atof-atof-l-wtof-wtof-l?view=msvc-170
 * 
 * Aclaración: Gran parte de el código ha sido reutilizado de prácticas anteriores de SSOO
 *             La parte final ha sido extraída de un foro
 * ************/


int leerdatos(double numeros[]){

    /* Creamos un buffer con una memoria igual al tamaño de un char * maximo de caracteres para garantizar espacio condicion */
    char *buffer = malloc(MAX_SIZE * sizeof(char));

    /* Variable para el control de errores */
    errno = 0;
    int contador = 0;
    int size = 0;
    char *aux;
    
    /*Abrir el fichero*/

    FILE *fp = fopen(FICHERO,"r");
    if(errno != 0){
        fprintf(stderr, "Se ha detectado un error (%d) al intentar abrir el fichero\n",errno);
        return 0;    
    }

    /*Copiamos los valores del fichero en el buffer*/
    fscanf(fp,"%s",buffer);

    /*Asignamos a cada indice del vector el double convertido del buffer, a la vez que incrementamos el valor del índice
    "Tokenizamos" el vector buffer tomando como separador la , */
    numeros[size++] = atof(strtok(buffer,","));

    while ((aux = strtok(NULL,",")) != NULL) {
        numeros[size++]=atof(aux);
    }

/* Liberamos la zona de memoria reservada para el array números */
free(buffer);

/* Cerramos correctamente el fichero */
fclose(fp);

return size;

}

/* *************
 * 
 * Autor: Diego Dorado Galán
 * 
 * Última edición: 16/04/2022
 *
 * Función encargada de buscar los vecinos de cada nodo
 * 
 ************ */

/* Mirar si se puede realizar de una forma más tecnica */

void getVecinos(int rank, int vecinos[]){

    int fila = rank/L;
    int columna = rank%L;

    /* En primer lugar averiguaremos los vecinos de la izquierda y derecha */

    switch(columna){

        /* Si esta en la columna de la izquierda */
        case 0:
            vecinos[2] = rank + (L-1);
            vecinos[3] = rank + 1;
            break;
        /* Si está en la columna de la derecha */
        case L-1:
            vecinos[2] = rank -1;
            vecinos[3] = rank - (L-1);
            break;
        default:
            vecinos[2] = rank-1;
            vecinos[3] = rank+1;
            break;
    }

    /* Para calcular los vecinos norte y sur */

    switch(fila){

        /* Si está en la fila de abajo */
        case 0:
            vecinos[0] = rank+L;
            vecinos[1] = (L * (L-1))+ rank;
            break;
        case L-1:
            vecinos[0] = columna;
            vecinos[1] = (rank - L);
            break;
        default:
            vecinos[0] = rank +L;
            vecinos[1] = rank - L;
            break;
    }
}


/* *************
 * 
 * Autor: Diego Dorado Galán
 * 
 * Última edición: 16/04/2022
 *
 * Función encargada de encontrar el mínimo
 * Un nodo debe envíar al nodo de la derecha y recibir de la izquierda
 * Un nodo debe envíar al nodo sur y recibir del nodo norte
 * 
 * 
 ************ */

double getMinimo(int rank, double buffer, int vecinos[]){
    int i;

    MPI_Status status;
    
    double min;
    min = buffer;
    
    /* Envio del rank a su vecino de la derecha tantas veces como elementos haya en la fila */
    for(i=0;i<L;i++){
        
        /* Envio al vecino de la derecha */
        MPI_Send(&min,1,MPI_DOUBLE, vecinos[3],i,MPI_COMM_WORLD);

        /*Recibo del vecino de la izquierda */
        MPI_Recv(&buffer,1, MPI_DOUBLE, vecinos[2],i,MPI_COMM_WORLD,&status);

        if(buffer < min){
            min = buffer;
        }
    }

    /* Repetimos la operación enviando datos al nodo inferior y leyendo del superior*/
    for(i=0;i<L;i++){
        
        /* Envio al vecino de la derecha */
        MPI_Send(&min,1,MPI_DOUBLE, vecinos[1],i,MPI_COMM_WORLD);

        /*Recibo del vecino de la izquierda */
        MPI_Recv(&buffer,1, MPI_DOUBLE, vecinos[0],i,MPI_COMM_WORLD,&status);

        if(buffer < min){
            min = buffer;
        }
    }

    return min;
}


/*Funcionamiento del programa para obtener el mínimo --> Pseudocodigo

double getMinimo() {
    int i;


    /* Envío a nodo este y recibo de oeste
    for(i=0;i<L;i++){

        if(num < min){
            min = num;
        }

        send(e);
        recv(o);

        if(num<min){
            min = num;
        }
    }

    /* Envio a nodo sur y recibo de norte
    for(i=0;i<L;i++){

        if(num < min){
            min = num;
        }

        send(e);
        recv(o);

        if(num<min){
            min = num;
        }
    }

}*/
