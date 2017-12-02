/* 
 * File:   main.c
 * Author: furkan
 *
 * Created on 22 Nisan 2017 Cumartesi, 12:56
 */
 



#include "requiredIncs.h"


threadVar info;
sem_t mutex;
char myfifo[PATH_MAX];		/*fifo konumu*/
char gtarget[PATH_MAX];
long mainPid;
long timedif;
struct timeval start;
struct timeval end;
pthread_t threads[NUM_THREADS];
int threadCount;
       	


void *myFunc(void *threadid)
{

	
		
	char *path;
    long  tid;
    
    path = (char*) threadid;
    tid = (long)pthread_self();
	search(gtarget, path, tid);
	sem_post(&mutex);
   	
   	return 0;
    
}

void catchSigInt(int signo) {
	

	if(signo == SIGINT)
	{
		if((long)getpid() == mainPid)
		{
			FILE *file;
			int i,num;
			sleep(1.5);


			for(i=0;i<threadCount;++i)
			{
				pthread_join(threads[i],NULL);
			}

			if((file = fopen("temp.txt","r")) != NULL )
			{
				

				i=0;
				while(i==0)
				{	
					if(fscanf(file, "%d",&num) != -1){

						info.numOfStrings += num;
						fscanf(file, "%d",&num);
						info.numOfDir+= num;
						fscanf(file, "%d",&num);
						info.numOfFiles += num;
						fscanf(file, "%d",&num);
						info.numOfLines += num;
						fscanf(file, "%d",&num);
						info.numOfThreads+=num;
						fscanf(file, "%d",&num);
						if(info.maxOfThread < num)
						{
							info.maxOfThread = num;
						}
					}
					else
					{
						i=1;
					}
				}
				fclose(file);
				remove("temp.txt");
			}	
			file = fopen("log.txt","a");
    		fprintf(file, "%d %s were found in total.\n",info.numOfStrings,gtarget);	
			printf("\nTotal number of strings found         : %d\n",info.numOfStrings );
	    	printf("Number of directories searched        : %d\n",info.numOfDir);
	    	printf("Number of files searched              : %d\n",info.numOfFiles);
	    	printf("Number of lines searched              : %d\n",info.numOfLines);

	    	printf("Number of search threads created      : %d\n",info.numOfThreads);
	    	
	    	printf("Max # of threads running concurrently : %d\n",info.maxOfThread);
	    	
	    	printf("Number of process created             : %d\n",info.numOfProcess);
	    	
	    	fclose(file);
	    	unlink(myfifo);
	    	
	    	if (gettimeofday(&end, NULL)) 
	    	{
			  fprintf(stderr, "Failed to get end time\n");
			  return;
			}	
			
			timedif = MILLION*(end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
			timedif /= miliSecond; /* mili saniye yapıldı*/
			timedif -= (1.5)*miliSecond;
		    printf("Total run time, in milliseconds       : %ld\n",timedif);				
			printf("Exit condition                        : due to signal SIGINT(CTRL+C)\n");
			sem_destroy(&mutex);
	    	exit(EXIT_SUCCESS);
		}
		else
		{
			int i;
			for(i=0;i<threadCount;++i)
			{
				pthread_join(threads[i],NULL);
			}
			FILE *file;
			char fileName[BUFSIZ];
			snprintf(fileName, BUFSIZ,"%s","temp.txt");
			file = fopen(fileName,"a");
			fprintf(file, "%d\n",info.numOfStrings);
			fflush(file);
			fprintf(file, "%d\n",info.numOfDir);
			fflush(file);
			fprintf(file, "%d\n",info.numOfFiles);
			fflush(file);
			fprintf(file, "%d\n",info.numOfLines);
			fflush(file);
			fprintf(file, "%d\n",info.numOfThreads);
			fflush(file);
			fprintf(file, "%d\n",info.maxOfThread);
			fflush(file);
			fclose(file);

			sem_destroy(&mutex);
			unlink(myfifo);				
			exit(EXIT_SUCCESS);	
		}
	}
}


int main(int argc, char *argv[]){
	
    DIR* dirp;
    FILE* log;
    threadVar result;
    
    if (argc != 3 ) 	/*hatalı usage */
    {    
        fprintf(stderr, "Usage: %s target directory\n", argv[0]);
		return -1;
    }   
    else if((dirp = opendir(argv[2])) == NULL)  /*Directory açılamazsa.*/
    {
        fprintf(stderr, "Açarken hata: %s\n", strerror(errno));
		return -1;	
    }
    else 
    {
    	if (gettimeofday(&start, NULL)) {
	      fprintf(stderr, "Failed to get start time\n");
	      return 1;
	    }
	    snprintf(gtarget,PATH_MAX,"%s",argv[1]);
    	mainPid = (long)getpid();
     	result= match(gtarget, argv[2]); /* toplam eşleşme sayısı */;
		log= fopen("log.txt","a");
    	fprintf(log, "%d %s were found in total.\n",result.numOfStrings,argv[1]);
    	printf("\nTotal number of strings found         : %d\n",result.numOfStrings );
    	printf("Number of directories searched        : %d\n",result.numOfDir);
    	printf("Number of files searched              : %d\n",result.numOfFiles);
    	printf("Number of lines searched              : %d\n",result.numOfLines);

    	printf("Number of search threads created      : %d\n",result.numOfThreads);
    	
    	printf("Max # of threads running concurrently : %d\n",result.maxOfThread);
    	
    	printf("Number of process created             : %d\n",result.numOfProcess);
    	
	}
	
    fclose(log);
    closedir(dirp);
    if (gettimeofday(&end, NULL)) {
	  fprintf(stderr, "Failed to get end time\n");
	  return -1;
	}	
	
	timedif = MILLION*(end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
	timedif /= miliSecond; /* mili saniye yapıldı*/
	printf("Exit condition                        : normal\n");
    printf("Total run time, in milliseconds       : %ld\n",timedif);
    return 0;
}

/**
 * @param target hedef kelime
 * @param dirName klasör ve dosya ismi (döngüde değişiyor)
 * @return tüm dosyalardaki eşleşme sayısı
 */
threadVar match(char* target,char *dirName){
  
    DIR *dirp;
    struct dirent *direntp;
    pid_t pid;
    
    signal(SIGINT,catchSigInt); /* signal catch*/

    info.numOfStrings=0;
	info.numOfDir=0;
	info.numOfFiles=0;
	info.numOfLines=0;			/* initialize datas*/
	info.numOfThreads=0;	
	info.numOfProcess=0;
	info.maxOfThread=0;
    
    
    if((dirp = opendir(dirName)) != NULL)
    {	
    	
    	int tempMax,i;

       	threadCount=0;
       	tempMax=0;
       	sem_destroy(&mutex);
       	sem_init(&mutex,0,1);
		info.numOfDir +=1;
        while((direntp = readdir(dirp)) != NULL)
        {	
        	
            if(direntp->d_type == DT_REG  )   /* path, file ise*/
            {
            	
            	
        		sem_wait(&mutex);
            	int rc;
			    char tempPath[PATH_MAX]; /* temp directory */
			    info.numOfFiles +=1; 	/* file sayısı artırıldı*/
			    
			    
                snprintf(tempPath, PATH_MAX,"%s/%s", dirName, direntp->d_name); 
			    
			    rc = pthread_create(&threads[threadCount], NULL, myFunc, (void *)tempPath);

			    if(!rc)
			    {
			    	info.numOfThreads +=1;	/* thread sayısı artırıldı*/
			    }
			    else
			    {
			    	perror("thread error !!\n");
			    	exit(-1);
			    }
				   
			  /*pthread_detach(threads[threadCount]);*/
			    
			    
			    ++threadCount;

				tempMax = threadCount;

				 

			}				
            else if( (direntp->d_type) == DT_DIR && strcmp(direntp->d_name,".") != 0 && strcmp(direntp->d_name, "..") != 0 )   /* path, directory ise*/    
            {
	        	int fd;					/*fifo için file descriptor*/
	        	snprintf(myfifo, PATH_MAX,"%s/%ld", dirName, (long)getpid()); /* her klasör için unique fifo konumu*/
	        	
            	info.numOfProcess +=1;
            	mkfifo(myfifo, FIFO_PERM);		/*fifo*/	/* FIFO_PERM (S_IRUSR | S_IWUSR)*/
            	fd = open(myfifo, FIFO_MODES);				/* FIFO_MODES   0666 */
            	
            	pid=fork();			/* fork */
            	if(pid == 0)		/*child */
            	{
		         	char tempPath[PATH_MAX];  /* temp directory */
		         	threadVar matches;
		         	snprintf(tempPath, PATH_MAX,"%s/%s", dirName, direntp->d_name);
		         	matches = match(target, tempPath);
					write(fd, &matches, sizeof(threadVar));		/*fifo ile yollandı*/
					exit(0);				/* child process sonlandı */
				}
				else if( pid > 0)			/* parent */
				{
					threadVar matches;
					wait(NULL);
					read(fd, &matches, sizeof(threadVar));		/*  fifo ile alındı */
                    close(fd);

                    info.numOfStrings += matches.numOfStrings;
					info.numOfDir += matches.numOfDir;
					info.numOfFiles += matches.numOfFiles;
					info.numOfLines += matches.numOfLines;
					info.numOfThreads += matches.numOfThreads;
					info.numOfProcess += matches.numOfProcess;
					
					if( info.maxOfThread < matches.maxOfThread)
						info.maxOfThread = matches.maxOfThread;
					
					unlink(myfifo);
                }
            }
		}
		printf("Number of cascade threads created     : %d\n",tempMax );

		if(tempMax > info.maxOfThread)
			info.maxOfThread = tempMax;

		

		for(i=0;i<threadCount;++i){
			pthread_join(threads[i],NULL);
		}

		sem_destroy(&mutex); 
		
	}
	
	closedir(dirp);
    return info;
}

/**
 * @param target hedef kelime
 * @param fileName kelimenin aranacağı dosya
 * @return bulunan eşleşme sayısı
 */
void search(char* target, const char* fileName, long threadId){
    
    FILE *fp, *log;
    char ch;
    int targetSize,gapRepeat,indexTarget,row,column,i;
    
    targetSize = strlen(target);	
    if(targetSize <1 || target =='\0'){ /* hedef kelime sıkıntılıysa*/  
        perror("Eşleşme bulunamadı !\n"); 
        return;
    }
    
    for(i=0;i< targetSize;++i)       /* hedef kelime küçük harfe çevrildi.*/
        if((int)target[i] >= (int)'A' && (int)target[i] <= (int)'Z')
            target[i]= tolower(target[i]);
    
    fp= fopen(fileName, "r");   /*aranacak dosya açıldı (txt) */
    if( fp == NULL){ /* dosya açılamazsa*/
        perror("Dosya açılamadı !\n");
        return;
    }
    
    log= fopen("log.txt","a"); /* log dosyası açıldı */
    if( log == NULL){ /* dosya açılamazsa*/
        perror("Dosya açılamadı !\n");
        return;
    }
    
        
    indexTarget=0;        /*  hedef kelime index kontrolcüsü 0-length */
    row=1;
    column=1;
    gapRepeat=0;
    ch = getc(fp);
   

    while ( ch != EOF && ch!='\0' ) {
        
        if( (ch ==' ' || (int)ch== 9 ) && indexTarget==0 )
        {                                         /* 9 tab'ın asci karşılığı */
            column++;     /*index==0 , hiç karakter eşleşmesi yapılmadıysa*/
        }
        
        else
        {   
            if( (int)ch >= (int)'A' && (int)ch <= (int)'Z')
                ch=tolower(ch);     /* okunan karakter küçük harfe çevrildi*/
            
            if(ch == target[indexTarget] )
            {
                if( indexTarget== targetSize-1 ) /* eşleşme olduysa ve hedef*/ 
                {                                /*kelimenin son index'i ise */
                    fprintf(log, "%ld - %ld %s: [%d, %d] %s first character is found.\n",(long)getpid(),threadId,fileName,row,column,target);
                    fflush(log);
                    info.numOfStrings +=1;
                    fseek(fp,-indexTarget-gapRepeat,SEEK_CUR);
                    column++;               /* dosya işaretçisi geri alındı */
                    indexTarget=0; 
                }
                else
                {
                    ++indexTarget; /* son eşleşme değilse */ 
                } 
            }                                /* 9 tab'ın ascii karşılığı*/
            else if(ch ==' ' || (int)ch== 9 || ch  =='\n') 
            {                 /*Eğer bu karakterlerle bir eşleşme durumu varsa*/      
                gapRepeat++;  /* eşleşme kadar boşluk sayısı artırılır.       */         
            }                 /* kelime bulunmazsa cursor'un yeterli sayıda   */
            else              /*  geri gidebilmesi için                       */
            {
                fseek(fp,-indexTarget-gapRepeat,SEEK_CUR);
                indexTarget=0;       /* dosya işaretçisi geri alındı */
                column++;
            }
            
            if(indexTarget==0)
            {    
                gapRepeat=0;    /* her yeni eşleşme için 0 lanır*/
            }
            if(ch == '\n' && indexTarget==0) 
            {				
                row++;
                column=1;
                gapRepeat=0;
                info.numOfLines +=1;
            }
        }
        ch = getc(fp);  /* her seferinde yeni karakter */
    }
    fclose(fp);
    fclose(log);
   	return ;
}
