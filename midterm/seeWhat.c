#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <math.h>
#include <wait.h>


#define miliSecond 1000.0
#define MILLION 1000000L

char myfifo[BUFSIZ];
char tempPath[BUFSIZ]; /* temp directory */
char serverPid[BUFSIZ];

int  server_to_client;
int fd,flag=0;
int static pidNo=1;
int static flag2=0;

typedef struct 
{
	double matrix[20][20];
	int n;
}Matrix;


void catchSigInt(int signo) {
	printf("catchSigInt\n");
	
	
	if(signo == SIGINT){
		kill(atoi(serverPid),SIGINT);
		unlink(tempPath);
		unlink(myfifo);
		close(server_to_client);
		close(fd);
				
		exit(0);
		
	}
	


}

void catchSigUsr1(int signo) {
	
	
	
	if(signo == SIGUSR1){
		
		
		unlink(tempPath);
		unlink(myfifo);
		close(server_to_client);
		close(fd);
				
		exit(0);
		
	}
}
void catchSigUsr2(int signo) {
	
	if(signo == SIGUSR2){
		
		flag = 1;
	}
}
double determinant(double f[20][20],int x);
void coFactor(double a[20][20],int n,double b[20][20]);
void transpose(double a[20][20],int n);
void convolution(double in[20][20],int x,int kernel[3][3],int y,double out[20][20]);
void shifted(Matrix* matrix);


int main(int argc, char *argv[])
{

	if(argc != 2){
		printf("%s\n","Usage: ./seeWhat <mainpipename>" );
	 	exit(-1);
	}   

	signal(SIGINT, catchSigInt);
	signal(SIGUSR1, catchSigUsr1);
	signal(SIGUSR2, catchSigUsr2);
	
	

	FILE* log;
	long timedif;
    struct timeval tpend;
    struct timeval tpstart;
	sprintf(myfifo,"%s",argv[1]);
	
	if(flag2 ==0)
	{
		printf("%s\n","Pid     result1                          result2" );
		flag2=1;
	}	
	
	
	/*********************************************************************************************************************************************************/
	int len;
	mkfifo(myfifo, 0666);
	server_to_client = open(myfifo, O_RDONLY);
	len = read(server_to_client,serverPid, BUFSIZ);
	
	close(server_to_client);
	unlink(myfifo);	


	
	/****************************************************************************************************************************************************/	
	
	while(1)
	{   	
		
		if(flag ==1)
		{
			int i,j,fdes[2],currentPid; 
			double result1,result2;
			Matrix matrix;
			char logName[BUFSIZ];
			pid_t pid;

			sprintf(tempPath,"%d",(int)getpid());  /* fifo name */
			
			mkfifo(tempPath, 0666);

			fd = open(tempPath, O_RDONLY);

			len = read(fd,&matrix,sizeof(Matrix) );
			
			
			close(fd);
			unlink(tempPath);

			sprintf(logName,"logs/log_%d_%d",(int)getpid(),pidNo);
			log= fopen(logName,"a");
			
			fprintf(log, "%s","orjinal=[" );
			fflush(log);

			for ( i = 0; i < matrix.n; ++i)
			{
				for ( j = 0; j < matrix.n; ++j)
				{
					fprintf(log, "%.2f ", matrix.matrix[i][j] );
					fflush(log);
				}
				if(i != matrix.n-1){
					fprintf(log, "%c", ';' );
					fflush(log);
				}
				else{
					fprintf(log,  "%s", "]\n" );
					fflush(log);	
				}	
			}
			fclose(log);
			currentPid = (int)getpid();
			pipe(fdes);
			pid=fork();
			if(pid >= 0)
			{
				if(pid == 0)
				{
					double temp;
					char pipes[BUFSIZ];
					signal(SIGINT, catchSigInt);
					signal(SIGUSR1, catchSigUsr1);
					signal(SIGUSR2, catchSigUsr2);

					gettimeofday(&tpstart, NULL);
					temp=determinant(matrix.matrix, matrix.n);
					shifted(&matrix);
					result1 = temp-determinant(matrix.matrix, matrix.n);
					gettimeofday(&tpend, NULL);

					timedif = MILLION*(tpend.tv_sec - tpstart.tv_sec) + tpend.tv_usec - tpstart.tv_usec;
					timedif /= miliSecond; /* mili saniye yapıldı*/
					
					printf("%d   %.2f    ",currentPid, result1 );
					
					log= fopen(logName,"a");
					fprintf(log, "%s","shifted=[" );
					fflush(log);

					for ( i = 0; i < matrix.n; ++i)
					{
						for ( j = 0; j < matrix.n; ++j)
						{
							fprintf(log, "%.2f ", matrix.matrix[i][j] );
							fflush(log);
						}
						if(i != matrix.n-1){
							fprintf(log, "%c", ';' );
							fflush(log);
						}
						else{
							fprintf(log,  "%s", "]\n" );
							fflush(log);	
						}	
					}
					fclose(log);

					
					sprintf(pipes,"%.2f  %.2f",result1,(double)timedif);
					close(fdes[0]);
					write(fdes[1],pipes,sizeof(pipes));						
					exit(0);

				}
				else
				{
					
					while(1)
					{
						waitpid(-1,NULL,0);
						if(errno == ECHILD)
						{
							break;
						}
						
					}
					FILE* file;
					double tempDet;
					char pipes[BUFSIZ],loger[BUFSIZ];	
					
					
					Matrix temp;
					int kernel[3][3];


					for ( i = 0; i < 3; ++i)
					{
						for ( j = 0; j < 3; ++j)
						{
							kernel[i][j] = 0;
						}
					}

					kernel[1][1]=1;

					
					gettimeofday(&tpstart, NULL);
					tempDet = determinant(matrix.matrix, matrix.n);
					convolution(matrix.matrix,matrix.n,kernel,3,temp.matrix);
					
					result2 = tempDet - determinant(temp.matrix, matrix.n);
					gettimeofday(&tpend, NULL);
					printf("%.2f\n",result2 );
					timedif = MILLION*(tpend.tv_sec - tpstart.tv_sec) + tpend.tv_usec - tpstart.tv_usec;
					timedif /= miliSecond; /* mili saniye yapıldı*/
					
					
					close(fdes[1]);
					read(fdes[0],pipes,sizeof(pipes)); 

					
					sprintf(loger,"m%d    %d\n%s\n%.2f   %.2f\n",pidNo,(int)getpid(),pipes,result2,(double)timedif);
					file = fopen("logs/result.txt","a");
					fprintf(file, "%s", loger );
					fflush(file);
					fclose(file);

					log= fopen(logName,"a");
					fprintf(log, "%s","convolution=[" );
					fflush(log);

					for ( i = 0; i < matrix.n; ++i)
					{
						for ( j = 0; j < matrix.n; ++j)
						{
							fprintf(log, "%.2f ", matrix.matrix[i][j] );
							fflush(log);
						}
						if(i != matrix.n-1){
							fprintf(log, "%c", ';' );
							fflush(log);
						}
						else{
							fprintf(log, "%s", "]\n" );
							fflush(log);	
						}	
					}
					fclose(log);
					++pidNo;
					flag =0;
				} 
			}
			
		

			
		}	
		else
		{
			kill(atoi(serverPid),SIGUSR2);
		}	
		
		
		
   }
   /* remove the FIFO */

   
   return 0;
}

void shifted(Matrix* matrix){
	Matrix temp,temp2;
	double det;
	int i,j,k,l;
	int count=0,n = (*matrix).n;

	while( count < 4){
		if(count == 0)
		{	
			
			k=0;l=0;
			for(i=0;i<n/2;++i){

				for(j=0;j<n/2;++j){
					temp.matrix[k][l]=(*matrix).matrix[i][j];
					++l;
				}
				l=0;
				++k;

			}
			det = determinant(temp.matrix, n/2);

			coFactor(temp.matrix , n/2, temp2.matrix);			/*cofactor */
			transpose(temp2.matrix, n/2);


			k=0;l=0;
			for(i=0;i<n/2;++i){

				for(j=0;j<n/2;++j){
					(*matrix).matrix[i][j] = temp2.matrix[k][l]*(1/det);
					++l;
				}
				l=0;
				++k;

			}
			


			++count;
		}

		else if(count == 1)
		{	k=0;l=0;
			for(i=n/2;i<n;++i){

				for(j=0;j<n/2;++j){
					temp.matrix[k][l] = (*matrix).matrix[i][j];
					++l;
				}
				l=0;
				++k;

			}
			det = determinant(temp.matrix, n/2);

			coFactor(temp.matrix , n/2, temp2.matrix);			/*cofactor */
			transpose(temp2.matrix, n/2);

			k=0;l=0;
			for(i=n/2;i<n;++i){

				for(j=0;j<n/2;++j){
					(*matrix).matrix[i][j] = temp2.matrix[k][l]*(1/det);
					++l;
				}
				l=0;
				++k;

			}
			++count;
		}
		else if(count ==2)
		{	k=0;l=0;
			for(i=0;i<n/2;++i){

				for(j=n/2;j<n;++j){
					temp.matrix[k][l]=(*matrix).matrix[i][j] ;
					++l;
				}
				l=0;
				++k;

			}

			det = determinant(temp.matrix, n/2);

			coFactor(temp.matrix , n/2, temp2.matrix);			/*cofactor */
			transpose(temp2.matrix, n/2);

			k=0;l=0;
			for(i=0;i<n/2;++i){

				for(j=n/2;j<n;++j){
					(*matrix).matrix[i][j]= temp2.matrix[k][l]*(1/det);
					++l;
				}
				l=0;
				++k;

			}


			++count;
		}
		else
		{	k=0;l=0;
			for(i=n/2;i<n;++i){

				for(j=n/2;j<n;++j){
					temp.matrix[k][l] = (*matrix).matrix[i][j] ;
					++l;
				}
				l=0;
				++k;
			}

			det = determinant(temp.matrix, n/2);

			coFactor(temp.matrix , n/2, temp2.matrix);			/*cofactor */
			transpose(temp2.matrix, n/2);

			k=0;l=0;
			for(i=n/2;i<n;++i){

				for(j=n/2;j<n;++j){
					(*matrix).matrix[i][j]= temp2.matrix[k][l]*(1/det) ;
					++l;
				}
				l=0;
				++k;
			}
			++count;
		}

	}
	return;	
}

void convolution(double in[20][20],int x,int kernel[3][3],int y,double out[20][20]){
	
	int i=0,j=0,m=0,mm=0,n=0,nn=0,ii=0,jj=0,kCenterX=0,kCenterY=0;

	/* find center position of kernel (half of kernel size)*/
	kCenterX = (int)(y / 2);
	kCenterY = (int)(y / 2);

	for(i=0; i < x; ++i)              // rows
	{
	    for(j=0; j < x; ++j)          // columns
	    {
	        for(m=0; m < y; ++m)     // kernel rows
	        {
	            mm = y - 1 - m;      // row index of flipped kernel

	            for(n=0; n < y; ++n) // kernel columns
	            {
	                nn = y - 1 - n;  // column index of flipped kernel

	                // index of input signal, used for checking boundary
	                ii = i + (m - kCenterY);
	                jj = j + (n - kCenterX);

	                // ignore input samples which are out of bound
	                if( ii >= 0 && ii < x && jj >= 0 && jj < x ){
	                    out[i][j] += in[ii][jj] * kernel[mm][nn];
	                }
	            }
	        }
	    }
	}
	


	return;
}

void coFactor(double a[20][20],int n,double b[20][20])
{
   int i,j,ii,jj,i1,j1;
   double det;
   double c[20][20];

   
   for (j=0;j<n;j++) {
      for (i=0;i<n;i++) {

         /* Form the adjoint a_ij */
         i1 = 0;
         for (ii=0;ii<n;ii++) {
            if (ii == i)
               continue;
            j1 = 0;
            for (jj=0;jj<n;jj++) {
               if (jj == j)
                  continue;
               c[i1][j1] = a[ii][jj];
               j1++;
            }
            i1++;
         }

         /* Calculate the determinate */
         det = determinant(c,n-1);

         /* Fill in the elements of the cofactor */
         b[i][j] = pow(-1.0,i+j+2.0) * det;
      }
   }
   return;
   
}

/*
   Transpose of a square matrix, do it in place
*/
void transpose(double a[20][20],int n)
{
   int i,j;
   double tmp;

   for (i=1;i<n;i++) {
      for (j=0;j<i;j++) {
         tmp = a[i][j];
         a[i][j] = a[j][i];
         a[j][i] = tmp;
      }
   }
   return;
}

double determinant(double a[20][20],int n)
{
   int i,j,j1,j2;
   double det = 0;
   double m[20][20];

   if (n < 1) { /* Error */

   } else if (n == 1) { /* Shouldn't get used */
      det = a[0][0];
   } else if (n == 2) {
      det = a[0][0] * a[1][1] - a[1][0] * a[0][1];
   } else {
      det = 0;
      for (j1=0;j1<n;j1++) {
         for (i=1;i<n;i++) {
            j2 = 0;
            for (j=0;j<n;j++) {
               if (j == j1)
                  continue;
               m[i-1][j2] = a[i][j];
               j2++;
            }
         }
         det += pow(-1.0,j1+2.0) * a[0][j1] * determinant(m,n-1);
         
      }
   }
   return(det);
}
