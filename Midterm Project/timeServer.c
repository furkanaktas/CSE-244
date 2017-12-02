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
#include <time.h>
#include <math.h>

#define miliSecond 1000.0
#define MILLION 1000000L
#define MSGLEN 64
char buf[BUFSIZ];
char myfifo[BUFSIZ];
char tempPath[BUFSIZ]; /* temp directory */

int server_to_client;
int ClientPidArr[BUFSIZ];/* client pid lerin tutuldugu array */
int ConnectedClientCount=0; /* baglanan Client sayisi icin */
int UniqueConsol =0;
int static fixArr[BUFSIZ];
int static fixCount=0;
int fd;


typedef struct 
{
	double matrix[20][20];
	int n;
}Matrix;



static void hdl (int sig, siginfo_t *siginfo, void *context)
{
	
	if (sig == SIGUSR2)
	{
		int i=0,j=ConnectedClientCount,k=0,status=0;

		while(i < fixCount){
			if (fixArr[i] == (int)(siginfo->si_pid) ){
				status =1;
			}

			++i;
		}
		if(status == 0){
			fixArr[fixCount] = (int)(siginfo->si_pid);
		}

		i=0;status=0;

		while(i < ConnectedClientCount)
		{

			if(ClientPidArr[k]==(int)(siginfo->si_pid) ){
				status = 1;
			}
			if(ClientPidArr[k] == -100)
			{
				j = i;
			}	

			if(ClientPidArr[k]!= -100){
				++i;
			}
			++k;
		}
		if(status == 0)
		{
			ClientPidArr[j]=(int)siginfo->si_pid;
			/*printf("client  :   %d\n", ClientPidArr[j]);*/
			++ConnectedClientCount;	
		}		
	
		
	}
	
}

void catchSigInt(int signo) {
	

	if(signo == SIGINT){
		int i=0,j=0;

		printf("catchSigInt\n");
				
		i=0;

		while(i < fixCount){
			kill(fixArr[i],SIGUSR1);
		}

		printf("connected :  %d\n", ConnectedClientCount);
		while (i<ConnectedClientCount){
			if(ClientPidArr[j] != -100){
				kill(ClientPidArr[j],SIGUSR1);
				++i;
			}
			++j;
			
		}
		
		unlink(myfifo);
		unlink(tempPath);
		close(server_to_client);
		close(fd);
		remove("logs/result.txt");				
		exit(EXIT_SUCCESS);
	}
	
	

}

void matrixGenerator(Matrix* matrix,int n);
double determinant(double a[20][20],int n);




int main(int argc, char *argv[])
{
		
		
	 
	 if(argc != 4){
	 	printf("%s\n","Usage: ./timeServer <ticks in miliseconds> <n>  <mainpipename>" );
	 	exit(-1);
	 }               	
	                	
	int fid;
	struct sigaction act;
	long double times,current;
	long timedif,timedif2;
    struct timeval tpend;
    struct timeval tpstart;
    struct timeval start;
    struct timeval end;
    FILE* log;
    char logPath[BUFSIZ];
   
	int control=0;
	int kontrol=0;
	 
	memset (&act, '\0', sizeof(act));
	 
	/* Use the sa_sigaction field because the handles has two additional parameters */
	act.sa_sigaction = &hdl;
	 
	/* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
	act.sa_flags = SA_SIGINFO;

	signal(SIGINT,catchSigInt);
	sigaction(SIGUSR2, &act, NULL);  

	
    
	sprintf(myfifo, "%s", argv[3]);  /* mainpipename*/
	sprintf(buf, "%d", getpid() );
	
	sprintf(logPath,"logs/%s","log_timeServer.log");

	log= fopen(logPath,"a"); /* log dosyası açıldı */
    if( log == NULL){ /* dosya açılamazsa*/
        perror("Dosya açılamadı !\n");
        return 0;
    }
    fprintf(log, "%s\n","time   ,pid   ,determinant" );
    fflush(log);
    fclose(log);		

    if (gettimeofday(&start, NULL)) {
	    fprintf(stderr, "Failed to get start time\n");
	    return 1;
	}

	printf("%s\n",	"Server başlatıldı ! " );	   
    	
	while (1)
    {
    	
    	mkfifo(myfifo , 0666);
		server_to_client = open(myfifo, O_WRONLY);
		write(server_to_client,buf,BUFSIZ);

		close(server_to_client);
		unlink(myfifo);
		
    	if (kontrol ==0)
    	{
    		

    		if (gettimeofday(&tpstart, NULL)) {
		      fprintf(stderr, "Failed to get start time\n");
		      return 1;
		    }

		   
    		kontrol =1;

    	}

		if (gettimeofday(&tpend, NULL)) {
	      fprintf(stderr, "Failed to get end time\n");
	      return 1;
	   	}
	   	timedif = MILLION*(tpend.tv_sec - tpstart.tv_sec) + tpend.tv_usec - tpstart.tv_usec;
		
		

		timedif /= miliSecond; /* mili saniye yapıldı*/
		

	   
	   	

		if ( timedif >= (long )(atoi(argv[1])) )   
		{  	
				
			int i=0;
			pid_t pid;


			pid=fork();

			if(pid == 0)
			{

				signal(SIGINT,catchSigInt);
				sigaction(SIGUSR2, &act, NULL);  
				i=0;
				while(i< ConnectedClientCount)
				{
					
					int fdes[2];
					int matches;
					int result;
					pid_t childPid;
					
					pipe(fdes);
					childPid = fork();
					if(childPid >= 0)
					{
		                if(childPid ==0)	    /* child  */
		                {

		                	Matrix matrix;
		                	Matrix temp;
		                	signal(SIGINT,catchSigInt);
							sigaction(SIGUSR2, &act, NULL);  
		                	kill(ClientPidArr[i], SIGUSR2);
		                	
		                	
		                	/*printf("%d\n", i);*/
		                	sprintf(tempPath,"%d",(int)ClientPidArr[i]);  /* client 'la  haberleşilecek fifo*/
							
		            /*******************************************************************************************************/    	
		    				matrixGenerator(&matrix,atoi(argv[2])*2);
		                	
		                	if (gettimeofday(&end, NULL)) {
						      fprintf(stderr, "Failed to get end time\n");							/*matrix oluşturulma zamanı*/
						      return 1;
						   	}
						   	timedif2 = MILLION*(end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
							timedif2 /= miliSecond; /* mili saniye yapıldı*/
		            /********************************************************************************************************/    	
						   	log= fopen(logPath,"a"); /* log dosyası açıldı */
						    if( log == NULL){ /* dosya açılamazsa*/
						        perror("Dosya açılamadı !\n");
						        return 0;
						    }

						    fprintf(log, "%.2Lf   ,%d   ,%.2f \n",(long double)timedif2,(int)ClientPidArr[i],determinant(matrix.matrix, (double)2*(atoi(argv[2]))));
						    fflush(log);
		                	fclose(log);

							mkfifo(tempPath, 0666);
							
							fd = open(tempPath, O_WRONLY );

							write(fd,&matrix,sizeof(Matrix));
														
							close(fd);
							unlink(tempPath);
							
							
						
							    			/*child process sonlandı*/
		                }
		                else if(childPid >0)
		                {
		                	signal(SIGINT,catchSigInt);
							sigaction(SIGUSR2, &act, NULL);  
		                	while(1){
								waitpid(-1,NULL,0);
								if(errno == ECHILD)
								{
									break;
								}
								/*convolusison();*/
							}	
							
		                	
							ClientPidArr[i] = -100;
							--ConnectedClientCount;
								
							
		                	i++;

		                }
					}
				}
				exit(0);
			}
			else if( pid > 0)
			{	
				i=0;

				while(i<ConnectedClientCount)
				{
					ClientPidArr[i] = -100;
					--ConnectedClientCount;
				}

				kontrol =0;
			}
			
			
		}

	}
	
   
   return 0;
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
void matrixGenerator(Matrix* matrix,int n){
	
	int i,j,count;
	int stop=0;
	(*matrix).n= n;
	
	
	for(i=0;i<20;++i){

		for(j=0;j<20;++j)
			(*matrix).matrix[i][j] = -1;

	}

	srand(time(NULL));
	
	while(stop ==0)
	{
		Matrix temp;
		temp.n = n/2;
		count =0;
		while(count <4)
		{
			
			for(i=0;i<temp.n;++i){

				for(j=0;j<temp.n;++j){
					temp.matrix[i][j] = rand()%500;
				}
			}
			
			
			if(determinant(temp.matrix, temp.n) !=0.0)
			{	
				int k,l;
				
				if(count == 0)
				{	k=0;l=0;
					for(i=0;i<n/2;++i){

						for(j=0;j<n/2;++j){
							(*matrix).matrix[i][j] = temp.matrix[k][l];
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
							(*matrix).matrix[i][j] = temp.matrix[k][l];
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
							(*matrix).matrix[i][j] = temp.matrix[k][l];
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
							(*matrix).matrix[i][j] = temp.matrix[k][l];
							++l;
						}
						l=0;
						++k;
					}
					++count;
				}	
				
			}
				
		}
		if(determinant((*matrix).matrix, n) != 0){
			stop =1;
		}
		
	}

	
	return;

}