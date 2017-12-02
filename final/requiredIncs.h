#ifndef REQUIREDINCS_H
#define REQUIREDINCS_H

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> 		/*tolower()*/
#include <string.h>     /* strcmp()*/
#include <unistd.h>     /*fork() pid_t */
#include <sys/wait.h>	/* wait()*/
#include <errno.h>		/* for errors */
#include <dirent.h>		/*for directory*/
#include <fcntl.h>		/*open */
#include <sys/stat.h>   /*mkfifo*/
#include <pthread.h>	/* thread functions*/
#include <semaphore.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <math.h>

#define FIFO_PERM (S_IRUSR | S_IWUSR)
#define FIFO_MODES 0666		/*open permission read-write */
#define miliSecond 1000.0
#define MILLION 1000000L
#define NUM_THREADS	300

/**
*	threadVar (thread Variables)
*/
typedef struct threadVar
{
	int numOfStrings;
	int numOfDir;
	int numOfFiles;
	int numOfLines;
	int numOfThreads;
	int numOfProcess;
	int maxOfThread;	
}threadVar;

typedef struct sharedVar
{
	int numOfStrings;
	int numOfLines;
	char path[PATH_MAX];
}sharedVar;

typedef struct 
{
	double matrix[20][20];
	int row,col;
}Matrix;

typedef struct 
{
	Matrix A,B,X;
	int generatedA,generatedB,generatedX;
	double normErr;
	
}results;


void generateMatrix(Matrix* matrix,int row,int col);
Matrix matrixCopy(Matrix a);
Matrix matrixTranspose(Matrix matrix);
Matrix matrixMul(Matrix x, Matrix y);
Matrix matrixSub(Matrix x, Matrix y);
double determinant(double a[20][20],int n);
Matrix inversMatrix(Matrix matrix);
void coFactor(double a[20][20],int n,double b[20][20]);
void householder(Matrix m, Matrix *R, Matrix *Q);
Matrix QRfactorization(Matrix A, Matrix *R, Matrix *Q, Matrix B);
Matrix pseudoInverse(Matrix A, Matrix B);

void writeToFile(FILE* log,Matrix matrix,char* name);
void writeAvgStd();


void matrixShow(Matrix m);

#endif
