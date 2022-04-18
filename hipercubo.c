/****************************************
* 
* - Autor: Diego Dorado Galán
* - Diseño de infraestructura de red
* - P1: Diseño de una red hipercubo
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
#include <math.h>

/* *********** CONSTANTES Y VARIABLES GLOBALES***************** */

#define FICHERO "datos.dat"

/* Definimos la operación XOR que utilizaremos para encontrar vecinos */
#define XOR(a,b) (a^b)

#define MAX_SIZE 1024
#define L 4
#define dim (int) pow(2,L)
#define TRUE 1
#define FALSE 0

/**************** FUNCIONES AUXILIARES ************************/

int leerdatos(double numeros[]);
void getVecinos(int rank, int *vecinos);
double getMax(int rank, double buffer, int *vecinos);
void enviarDatos(double *numeros);

/**************** FUNCION PRINCIPAL ***************************/

int main(int argc, char *argv[]){

    int rank, size;

    MPI_Status status;

    /* Nodos (rank) vecinos de un rank */
    int vecinos[L];

    /* Maximo */
    double max;
    /* Buffer que almacenará el número de cada nodo */
    double buffer;

    /* Numeros leidos que compararemos con el tamaño para comprobar si se ha leído completamente o existen suficientes datos */
    int leidos;
    /* Array que almacenará los numeros */
    double numeros[dim];

    int condicion = TRUE;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* Primero debemos comprobar si es el rank 0 */
    if(rank == 0){

        /* Se debe comprobar si se lanza un número de procesos adecuado para un hipercubo de lado L */
        if(size == dim) {

            double *numeros = malloc(size * sizeof(double));
            /* Leemos el fichero y almacenamos el tamaño del vector */
            leidos = leerdatos(numeros); 

            /* Debemos comprobar además si existen suficientes datos parar satisfacer los nodos del toroide
               Enviamos los datos del fichero a los procesos */
            if(dim == size){
                MPI_Bcast(&condicion,1,MPI_INT,0,MPI_COMM_WORLD);
                
                enviarDatos(numeros);

                condicion=TRUE; /* Esto no es necesario ya que no se debe haber cambiado su valor */
                free(numeros);
            }
            else {
                condicion = FALSE;
                fprintf(stderr, "El número de datos leídos del fichero (%d) no es suficiente. Se necesitan %d\n", leidos, dim);
                
                /* Realizamos un proceso similar al caso anterior */
                MPI_Bcast(&condicion,1,MPI_INT,0,MPI_COMM_WORLD);
            }
        
        }

        else {
            
            /* Realizaremos un broadcast para notificar al resto de procesos que aborten la ejecución */
            condicion = FALSE;
            fprintf(stderr,"Tenemos que ejecutar %d procesos para un hipercubo de lado %d \n",dim,L);


            /*  El rank 0 pasa 1 entero (condicion) a través del comunicador */
            MPI_Bcast(&condicion,1,MPI_INT,0,MPI_COMM_WORLD);
            
        }
    }

    /* Con una llamada colectiva nos aseguramos un punto de ejecución común que indique a los nodos que pueden continuar */
    MPI_Bcast(&condicion,1,MPI_INT,0,MPI_COMM_WORLD);

    if(condicion==TRUE){

        MPI_Recv(&buffer,1,MPI_DOUBLE,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);

        getVecinos(rank,vecinos);

        max = getMax(rank,buffer, vecinos);

        if(rank == 0)
            printf("El mayor número encontrado en la red es %.5lf\n",max);

    }

    MPI_Finalize();
    return 0;
}


/* ********************* FUNCIONES AUXILIARES ******************************/

void enviarDatos(double *numeros){
    int i;
    double buffer;

    for(i=0;i<dim;i++){
        buffer = numeros[i];
        /* El rank 0 envía al resto de nodos (y a sí mismo) los datos */
        /* PREGUNTAR: Por qué no deja poner MPI_ANY_TAG */
        MPI_Send(&buffer,1,MPI_DOUBLE,i,0,MPI_COMM_WORLD);
        printf("%5.lf enviado al nodo %d \n",buffer,i);
    }
}


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

    while ((aux = strtok(NULL,","))!=NULL) {
        numeros[size++]=atof(aux);
    }

/* Liberamos la zona de memoria reservada para el array números */
free(buffer);

/* Cerramos correctamente el fichero */
fclose(fp);

return size;

}

/* *****************************
 *
 * Una página que me ha ayudado a encontrar la solución es la siguiente:
 * https:/* hackernoon.com/how-to-solve-the-hamming-distance-problem-in-c
 * 
 * También: https://en.wikipedia.org/wiki/Hypercube_(communication_pattern)
 *  
 * Otra opción que creo que podría funcionar sería realizar un desplazamiento a la izquierda
 * 
 ***************************** */


void getVecinos(int rank, int vecinos[]){

    int i;
    int aux;

    for(i=0;i<L;i++){
        vecinos[i]= XOR(rank,(int)pow(2,1));
    }
}

double getMax(int rank, double buffer, int vecinos[]){

    int i;

    double max;

    max = buffer;
    
    for(i=0;i<L;i++){

        if(buffer > max){
            max = buffer;
        }
        
        MPI_Send(&max, 1, MPI_DOUBLE, vecinos[i],i,MPI_COMM_WORLD);

        MPI_Recv(&buffer, 1, MPI_DOUBLE, vecinos[i],i, MPI_COMM_WORLD,NULL);

        if(buffer > max){
            max = buffer;
        }

    }

    return max;

}
