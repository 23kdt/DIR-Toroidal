all: 

clean:
	-rm -f toroide

	-rm -f hipercubo

compilar:

	mpicc toroidal.c -o toroide

	mpicc hipercubo.c -o hipercubo

ejecutarToroide:
	mpirun --hostfile my-hostfile -n 16 ./toroide

ejecutarHipercubo:
	mpirun --hostfile my-hostfile -n 16 ./hipercubo