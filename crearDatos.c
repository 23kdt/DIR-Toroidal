#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


double numAleatorio(int i, int f);

int main(int argc, char *argv[]) {
    int i;
    int numero = atoi(argv[1]);
    FILE *fp;
    char filename[] = "data.txt";

    if((fp = fopen(filename,"w+"))==NULL){
        printf("Ha ocurrido un error en la apertura del archivo");
    }

    srand(time(NULL));

    for(i=0;i<numero;i++){
        fprintf(fp,"%lf,", numAleatorio(-500,500));
    }

    fclose(fp);
    return 0;
    
}

double numAleatorio (int i, int f){
    double num;

    num = (rand()%(f + i+1) +i);

    return num;
}