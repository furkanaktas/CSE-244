
#include "requiredIncs.h"

pthread_t threads[NUM_THREADS];		/* NUM_THREADS == 300 */
int threadCount;
sem_t mutex;
int* socketNo;
double* times;
int timeIndex;
struct timeval end;
struct timeval start;
	

void catchSigInt(int signo) {
	
	if(signo == SIGINT)
	{
		FILE* log;
		double timedif;
		
		printf("%s\n","catch SIGINT !" );
		writeAvgStd();	/* aritmetik ortalama ve standart sapma dosyaya yazıldı*/
		free(times);
		sem_destroy(&mutex);
		
		gettimeofday(&end, NULL);
		timedif = MILLION*(end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
		timedif /= miliSecond; /* mili saniye yapıldı*/
		log= fopen("logs/Clients","a");
		fprintf(log, "time of the termination signal which received before the end of execution   : %.2f\n",timedif );
		fflush(log);
		fclose(log);
		exit(0);
	}
}

void *myFunc(void *threadid)
{
	sem_wait(&mutex);
	struct timeval tpend;
	struct timeval tpstart;
	double timedif;
	
	gettimeofday(&tpstart, NULL);



	int sock,mysock;
	struct sockaddr_in server;
	struct hostent *hp;
	char myname[PATH_MAX];
	
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		perror("Failed to create socket");
		exit(-1);
	}

	server.sin_family = AF_INET;
	gethostname(myname, PATH_MAX);
	hp = gethostbyname (myname);
	if(hp == NULL)
	{
		perror("gethostbyname Failed!");
		close(sock);
		exit(-1);
	}

	memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
	server.sin_port = htons(*socketNo);

	if(connect(sock, (struct sockaddr*) &server, sizeof(server)) <0)
	{
		close(sock);
		kill(getpid(),SIGINT);
	}
		    
	char temp[PATH_MAX];
	snprintf(temp,PATH_MAX,"%s.%d",(char*)threadid,(int) pthread_self());
	
	if(write(sock, temp, sizeof(temp)) <0)	
	{
		close(sock);
		kill(getpid(),SIGINT);
	}

	results result;
	if( read(sock, &result, sizeof(results)) < 0)
	{
		close(sock);
		kill(getpid(),SIGINT);
	}	

	
	
	char logName[PATH_MAX];
	snprintf(logName,PATH_MAX,"logs/%d-Client",(int) pthread_self());

	FILE* log;
	
	log= fopen(logName,"a");
	writeToFile(log,result.A,"A=[");
	fclose(log);

	log= fopen(logName,"a");
	writeToFile(log,result.B,"B=[");
	fclose(log);
	
	log= fopen(logName,"a");
	writeToFile(log,result.X,"X=[");
	fclose(log);

	log= fopen(logName,"a");
	fprintf(log, "Norm of the error : %.2f",result.normErr );
	fflush(log);
	fclose(log);

	gettimeofday(&tpend, NULL);

	timedif = MILLION*(tpend.tv_sec - tpstart.tv_sec) + tpend.tv_usec - tpstart.tv_usec;
	timedif /= miliSecond; /* mili saniye yapıldı*/
	times[timeIndex]= timedif;
	++timeIndex;
	close(sock);
	sem_post(&mutex);
   	return 0;
}


int main(int argc, char *argv[]){

	if(argc != 4)
	{
		fprintf(stderr, "Usage: %s <#of columns of A, m> <#of rows of A, p> <#of clients, q>\n",argv[0] );
		return -1;
	}
	else
	{
		key_t socKey =616161;
		int shmidSocket;
		gettimeofday(&start, NULL);

		shmidSocket = shmget(socKey,sizeof(int), IPC_CREAT | 0666);		/*sinyal handler için özel shared açıldı*/
		if (shmidSocket < 0)
		{
			perror("shmget");
			exit(-1);
		}
		
		socketNo = shmat(shmidSocket,NULL,0);

		if(atoi(argv[1]) < atoi(argv[2]))
		{
			printf("%s\n", "row >= col olmalı !");
			exit(0);
		}	
		
		if (*socketNo == 0)
		{
			perror("Server doesn't start !");
		}
		else
		{
			int rc;
		    threadCount =0;
		    sem_init(&mutex,0,1);
		    char temp[PATH_MAX];

		    signal(SIGINT,catchSigInt); /* signal catch*/

			times = (double*) malloc(atoi(argv[3])*sizeof(double)); 	/* ortalama süre hesabı için toplam client sayısı kadar yer alındı */
			timeIndex=0;
		    snprintf(temp,PATH_MAX,"%s.%s",argv[1],argv[2]); 		/* matrix row, col*/
		    for (int i = 0; i < atoi(argv[3]); ++i)
		    {
            	rc = pthread_create(&threads[threadCount], NULL, myFunc, temp);
				if(!rc)
			    {
			    	++threadCount;
			    }
			    else
			    {
			    	perror("thread error !!\n");
			    	exit(-1);
			    }
		    }

		    for(int i=0; i<atoi(argv[3]); ++i){		
				pthread_join(threads[i],NULL);
			}
			writeAvgStd();		/* aritmetik ortalama ve standart sapma dosyaya yazıldı*/
			free(times);		
			sem_destroy(&mutex); 
		}
	
	}
	return 0;
}

void writeAvgStd()
{
	double sum=0,avrg,stdDev;
	for (int i = 0; i < timeIndex; ++i)
	{
		sum += times[i];
	}
	avrg = sum/timeIndex; 	/* avarage */
	sum =0;
	for (int i = 0; i < timeIndex; ++i)
	{
		sum += pow((avrg-times[i]),2);
	}
	sum /= (timeIndex-1);
	stdDev = sqrt(sum);		/* standart dev. */

	char logName[PATH_MAX];
	snprintf(logName,PATH_MAX,"logs/Clients");
	
	FILE* log;

	log= fopen(logName,"a");
	fprintf(log, "Avarage connection time   : %.2f\n",avrg );
	fflush(log);
	fprintf(log, "Standard deviation of the overall run : %.2f\n",stdDev );
	fflush(log);
	fclose(log);
}

void writeToFile(FILE* log,Matrix matrix,char* name)
{
	fprintf(log, "%s",name );
	fflush(log);

	for (int i = 0; i < matrix.row; ++i)
	{
		for (int j = 0; j < matrix.col; ++j)
		{
			fprintf(log, "%.2f ", matrix.matrix[i][j] );
			fflush(log);
		}
		if(i != matrix.row-1){
			fprintf(log, "%c", ';' );
			fflush(log);
		}
		else{
			fprintf(log, "%s", "]\n" );
			fflush(log);	
		}	
	}
}

void matrixShow(Matrix m)
{
	for(int i = 0; i < m.row; i++) {
		for (int j = 0; j < m.col; j++) {
			printf(" %8.2f", m.matrix[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}



