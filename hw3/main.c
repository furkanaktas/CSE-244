/* 
 * File:   main.c
 * Author: furkan
 *
 * Created on 16 Mart 2017 Perşembe, 12:56
 */
 
#include "requiredIncs.h"

int main(int argc, char *argv[]){
	
    DIR* dirp;
    FILE* log;
    int totalMatch;
    
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
     	totalMatch= match(argv[1], argv[2]); /* toplam eşleşme sayısı */;
		log= fopen("log.log","a");
    	fprintf(log, "\n%d %s were found in  total.\n\n",totalMatch,argv[1]);
	}
	
    fclose(log);
    closedir(dirp);
    return 0;
}

/**
 * @param target hedef kelime
 * @param dirName klasör ve dosya ismi (döngüde değişiyor)
 * @return tüm dosyalardaki eşleşme sayısı
 */
int match(char* target,char *dirName){
  
    DIR *dirp;
    struct dirent *direntp;
    pid_t pid,childPid;
    int totalMatch,matches;
    
    totalMatch=0;
    if((dirp = opendir(dirName)) != NULL)
    {	
    	
        while((direntp = readdir(dirp)) != NULL)
        {	
        	
            if(direntp->d_type == DT_REG  )   /* path, file ise*/
            {
            	int fd[2];				/*pipe için file descriptor*/
                pipe(fd);				/*pipe*/
                childPid = fork();      /*Fork */
				if(childPid >= 0)
				{
                    if(childPid > 0)   /* parent */
                    {
                    	wait(NULL);
                    	close(fd[1]);
                    	read(fd[0], &matches, sizeof(int));		/*  pipe ile alındı */
                    	totalMatch +=  matches;
                    }
                    else if(childPid ==0)	    /* child  */
                    {
                    	char tempPath[PATH_MAX]; /* temp directory */
                    	close(fd[0]);
                    	snprintf(tempPath, PATH_MAX,"%s/%s", dirName, direntp->d_name); 
						closedir(dirp); 
						matches = search(target, tempPath);
						write(fd[1], &matches, sizeof(int));	/*pipe ile yollandı*/
						exit(0);    			/*child process sonlandı*/
                    }
                }
				else if(childPid == -1)
				{
                    perror("Fork başarısız !\n");
                    closedir(dirp);
                    return -1;	
				}
			}				
            else if( (direntp->d_type) == DT_DIR && strcmp(direntp->d_name,".") != 0 && strcmp(direntp->d_name, "..") != 0 )   /* path, directory ise*/    
            {
            	char myfifo[PATH_MAX];		/*fifo konumu*/
            	int fd;					/*fifo için file descriptor*/
            	snprintf(myfifo, PATH_MAX,"%s/%ld", dirName, (long)getpid()); /* her klasör için unique fifo konumu*/
            	
            	
            	mkfifo(myfifo, FIFO_PERM);		/*fifo*/	/* FIFO_PERM (S_IRUSR | S_IWUSR)*/
            	fd = open(myfifo, FIFO_MODES);				/* FIFO_MODES   0666 */
            	
            	pid=fork();			/* fork */
            	if(pid == 0)		/*child */
            	{
		         	char tempPath[PATH_MAX];  /* temp directory */
		         	snprintf(tempPath, PATH_MAX,"%s/%s", dirName, direntp->d_name); 
					matches = match(target, tempPath);
					write(fd, &matches, sizeof(int));		/*fifo ile yollandı*/
					exit(0);				/* child process sonlandı */
				}
				else if( pid > 0)			/* parent */
				{
					wait(NULL);
					read(fd, &matches, sizeof(int));		/*  fifo ile alındı */
                    close(fd);
                    totalMatch += matches;
                    unlink(myfifo);
                }
            }
		}  
    }
    
    closedir(dirp);
    return totalMatch;
}

/**
 * @param target hedef kelime
 * @param fileName kelimenin aranacağı dosya
 * @return bulunan eşleşme sayısı
 */
int search(char* target, const char* fileName){
    
    FILE *fp, *log;
    char ch;
    int targetSize,gapRepeat,indexTarget,row,column,i,matches;
    
    targetSize = strlen(target);	
    if(targetSize <1 || target =='\0'){ /* hedef kelime sıkıntılıysa*/  
        perror("Eşleşme bulunamadı !\n"); 
        return 0;
    }
    
    for(i=0;i< targetSize;++i)       /* hedef kelime küçük harfe çevrildi.*/
        if((int)target[i] >= (int)'A' && (int)target[i] <= (int)'Z')
            target[i]= tolower(target[i]);
    
    fp= fopen(fileName, "r");   /*aranacak dosya açıldı (txt) */
    if( fp == NULL){ /* dosya açılamazsa*/
        perror("Dosya açılamadı !\n");
        return 0;
    }
    
    log= fopen("log.log","a"); /* log dosyası açıldı */
    if( log == NULL){ /* dosya açılamazsa*/
        perror("Dosya açılamadı !\n");
        return 0;
    }
    
        
    indexTarget=0;        /*  hedef kelime index kontrolcüsü 0-length */
    row=1;
    column=1;
    gapRepeat=0;
    ch = getc(fp);
    matches =0;  

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
                    fprintf(log, "%s: [%d, %d] %s first character is found.\n",fileName,row,column,target);
                    matches++;
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
            }
        }
        ch = getc(fp);  /* her seferinde yeni karakter */
    }
    
    fclose(fp);
    fclose(log);
   
    return matches;
}
