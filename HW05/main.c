/* 
 * File:   main.c
 * Author: furkan
 *
 * Created on 22 Nisan 2017 Cumartesi, 12:56
 */
 



#include "requiredIncs.h"

threadVar info;
sem_t mutex;
char gtarget[PATH_MAX];
long mainPid;
long timedif;
struct timeval start;
struct timeval end;
pthread_t threads[NUM_THREADS];
int threadCount;
DIR* dirpMain;
sharedVar* shmVar;
int shmid;
key_t sigKey =616161; 			/* sinyal handler için özel key*/
signalVar* sVar;
int shmidSig;

void *myFunc(void *threadid)
{
	sharedVar* temp;
	long  tid;
    
    temp = (sharedVar*) threadid;
    tid = (long)pthread_self();
	search(gtarget, tid, temp);
	sem_post(&mutex);
   	return 0;
}

void catchSigInt(int signo) {
	
	if(signo == SIGINT)
	{
		FILE* file;
		int i;
		for(i=0;i<threadCount;++i)
		{
			pthread_join(threads[i],NULL);
		}
		
		if((long)getpid() == mainPid )
		{
			file = fopen("log.txt","a");
			fprintf(file, "%d %s were found in total.\n",(*sVar).numOfStrings,gtarget);	
			printf("\nTotal number of strings found         : %d\n",(*sVar).numOfStrings );
	    	printf("Number of directories searched        : %d\n",(*sVar).numOfShared);
	    	printf("Number of files searched              : %d\n",(*sVar).numOfFiles);
	    	printf("Number of lines searched              : %d\n",(*sVar).numOfLines);
	    	printf("Number of search threads created      : %d\n",(*sVar).numOfThreads);
	    	printf("Max # of threads running concurrently : %d\n",(*sVar).maxOfThread);
	    	printf("Number of created shared memory       : %d\n",(*sVar).numOfShared);
	    	
	    	
	    	if (gettimeofday(&end, NULL)) 
	    	{
			  fprintf(stderr, "Failed to get end time\n");
			  return;
			}	
			
			timedif = MILLION*(end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
			timedif /= miliSecond; /* mili saniye yapıldı*/
			printf("Total run time, in milliseconds       : %ld\n",timedif);				
			printf("Exit condition                        : due to signal SIGINT(CTRL+C)\n");
			shmctl(shmidSig, IPC_RMID, NULL);
			
    	}
		
		sem_destroy(&mutex);
		shmctl(shmid, IPC_RMID, NULL);
		closedir(dirpMain);
		exit(EXIT_SUCCESS);
	}
}


int main(int argc, char *argv[]){
	
	FILE* log;

    threadVar result;
    
    if (argc != 3 ) 	/*hatalı usage */
    {    
        fprintf(stderr, "Usage: %s target directory\n", argv[0]);
		return -1;
    }   
    else if((dirpMain = opendir(argv[2])) == NULL)  /*Directory açılamazsa.*/
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

	    shmidSig = shmget(sigKey,sizeof(signalVar), IPC_CREAT | 0666);		/*sinyal handler için özel shared açıldı*/
    	if (shmid < 0)
    	{
    		perror("shmget");
    		exit(-1);
    	}

    	sVar = shmat(shmidSig,NULL,0);
	    
	    (*sVar).numOfStrings=0;
		(*sVar).numOfDir=0;
		(*sVar).numOfShared=0;
		(*sVar).numOfFiles=0;
		(*sVar).numOfLines=0;			/* initialize datas*/
		(*sVar).numOfThreads=0;	
		(*sVar).numOfProcess=0;
		(*sVar).maxOfThread=0;
	    

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

		fclose(log);
		shmctl(shmidSig, IPC_RMID, NULL);
    }
	
    if (gettimeofday(&end, NULL)) {
	  fprintf(stderr, "Failed to get end time\n");
	  return -1;
	}	
	
	timedif = MILLION*(end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
	timedif /= miliSecond; /* mili saniye yapıldı*/
	printf("Exit condition                        : normal\n");
    printf("Total run time, in milliseconds       : %ld\n",timedif);
    
    closedir(dirpMain);
    return 0;
}

/**
 * @param target hedef kelime
 * @param dirName klasör ve dosya ismi (döngüde değişiyor)
 * @return tüm dosyalardaki eşleşme sayısı
 */
threadVar match(char* target,char *dirName){
  
    DIR* dirp;
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
    	
    	int tempMax,max,i;
    	key_t key;
    
       	key= getpid();						/* her process'e özel shared mem açıldı.*/
    	shmid = shmget(key,sizeof(sharedVar), IPC_CREAT | 0666);
    	if (shmid < 0)
    	{
    		perror("shmget");
    		exit(-1);
    	}

    	shmVar = shmat(shmid,NULL,0);

    	(*shmVar).numOfStrings=0;
    	(*shmVar).numOfLines =0;
    	
    	sVar = shmat(shmidSig,NULL,0);

       	threadCount=0;
       	tempMax=0;
       	max=0;
       	sem_destroy(&mutex);
       	sem_init(&mutex,0,1);


		info.numOfDir +=1;
		(*sVar).numOfShared +=1;
		(*sVar).numOfDir++;
        while((direntp = readdir(dirp)) != NULL)
        {	
        	
            if(direntp->d_type == DT_REG  )   /* path, file ise*/
            {
            	sem_wait(&mutex);
            	int rc;
			    info.numOfFiles +=1; 	/* file sayısı artırıldı*/
			    (*sVar).numOfFiles +=1;
			    
                snprintf((*shmVar).path, PATH_MAX,"%s/%s", dirName, direntp->d_name); 
			    rc = pthread_create(&threads[threadCount], NULL, myFunc, (void *)shmVar);
				
				if(!rc)
			    {
			    	info.numOfThreads +=1;	/* thread sayısı artırıldı*/
			    	(*sVar).numOfThreads +=1;
			    }
			    else
			    {
			    	perror("thread error !!\n");
			    	exit(-1);
			    }
				   
			    ++threadCount;

				info.maxOfThread = threadCount;
				if(threadCount >(*sVar).maxOfThread)
				{
					(*sVar).maxOfThread = threadCount;
				}
				 

			}				
            else if( (direntp->d_type) == DT_DIR && strcmp(direntp->d_name,".") != 0 && strcmp(direntp->d_name, "..") != 0 )   /* path, directory ise*/    
            {
	        	int msqid;					/*message queue için descriptor*/
	        	
            	info.numOfProcess +=1;
            	(*sVar).numOfProcess +=1;
            	
            	if((msqid= msgget((key_t)getpid(),FIFO_PERM | IPC_CREAT))== -1)
            	{
            		perror("Fail to creat msqueue");
            		exit(-1);
            	}
            	
            	pid=fork();			/* fork */
            	if(pid == 0)		/*child */
            	{
		         	char tempPath[PATH_MAX];  /* temp directory */
		         	threadVar matches;
		         	
		         	matches.numOfStrings=0;
					matches.numOfDir=0;
					matches.numOfFiles=0;
					matches.numOfLines=0;
					matches.numOfThreads=0;
					matches.numOfProcess=0;
					matches.maxOfThread=0;
					
		         	snprintf(tempPath, PATH_MAX,"%s/%s", dirName, direntp->d_name);
		         	matches = match(target, tempPath);
		         	if(msgsnd(msqid, &matches, sizeof(threadVar), 0) ==-1)
		         	{
		         		perror("msgsnd error!");
		         		exit(-1);
		         	}
					exit(0);				/* child process sonlandı */
				}
				else if( pid > 0)			/* parent */
				{
					threadVar matches;
					wait(NULL);
					(*sVar).numOfProcess -=1;
					if(msgrcv(msqid, &matches, sizeof(threadVar), 0, FIFO_PERM | O_NONBLOCK) ==-1)
		         	{
		         		perror("msgrcv error!");
		         		exit(-1);
		         	}
					
                    info.numOfStrings += matches.numOfStrings;
					info.numOfDir += matches.numOfDir;
					info.numOfFiles += matches.numOfFiles;
					info.numOfLines += matches.numOfLines;
					info.numOfThreads += matches.numOfThreads;
					info.numOfProcess += matches.numOfProcess;
					
					tempMax = matches.maxOfThread;
					if(max < tempMax)
					{
						max = tempMax;
					}
					if(msgctl(msqid, IPC_RMID, NULL) ==-1)
		         	{
		         		perror("msgctl error!");
		         		exit(-1);
		         	}
                }
            }
		}
		
		if(max > info.maxOfThread)
			info.maxOfThread = max;

		for(i=0;i<threadCount;++i){
			pthread_join(threads[i],NULL);
		}
		printf("Number of cascade threads created     : %d\n",threadCount );

		info.numOfLines += (*shmVar).numOfLines;		/*shared memory deki bilgiler, ana struct'a  aktarıldı*/
		info.numOfStrings += (*shmVar).numOfStrings;

		sem_destroy(&mutex); 
		shmctl(shmid, IPC_RMID, NULL);
		
	}
	closedir(dirp);
    return info;
}

/**
 * @param target hedef kelime
 * @param fileName kelimenin aranacağı dosya
 * @return bulunan eşleşme sayısı
 */
void search(char* target, long threadId, sharedVar* shr){
    
    FILE *fp, *log;
    char ch,*fileName;
    int targetSize,gapRepeat,indexTarget,row,column,i;
    
    fileName= (char*) (*shr).path;
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
                    (*shr).numOfStrings +=1;  /* memory güncellendi*/
                    (*sVar).numOfStrings +=1;
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
                (*shr).numOfLines +=1;	/* memory güncellendi*/
                (*sVar).numOfLines +=1;
            }
        }
        ch = getc(fp);  /* her seferinde yeni karakter */
    }
    fclose(fp);
    fclose(log);
   	return ;
}
