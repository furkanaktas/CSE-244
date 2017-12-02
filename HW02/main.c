/* 
 * File:   main.c
 * Author: furkan
 *
 * Created on 4 Mart 2017 Cumartesi, 10:48
 */
 
#include "requiredIncs.h"

int main(int argc, char *argv[]){
	
    DIR* dirp;
    FILE* log;
    int totalMatch;
    
    if (argc != 3 ) 	/*hatalı usage */
    {    
        fprintf(stderr, "Usage: %s directory\n", argv[0]);
		return -1;
    }   
    else if((dirp = opendir(argv[2])) == NULL)  /*Directory açılamazsa.*/
    {
        fprintf(stderr, "Açarken hata: %s\n", strerror(errno));
		return -1;	
    }
    else 
    {
        match(argv[1], argv[2]); /* toplam eşleşme sayısı */
		
		totalMatch = total();
		log= fopen("log.log","a");
    	fprintf(log, "\n%d %s were found in  total.\n",totalMatch,argv[1]);
	}
	
    fclose(log);
    closedir(dirp);
    return 0;
}

/**
 * @param target hedef kelime
 * @param dirName klasör ve dosya ismi (döngüde değişiyor)
 * @return void
 */
void match(char* target,char *dirName){
  
    DIR *dirp;
    struct dirent *direntp;
    pid_t pid,childPid;
    
	if((dirp = opendir(dirName)) != NULL)
    {	
    	
        while((direntp = readdir(dirp)) != NULL)
        {	
            if(direntp->d_type == DT_REG  )   /* path, file ise*/
            {
                childPid = fork();          /*Fork */
				if(childPid >= 0)
				{
                    if(childPid != 0)   /* parent */
                    {
                        wait(NULL);
                    }
                    else 	    /* child olustuysa */
                    {
                    	char tempPath[PATH_MAX]; /* temp directory */
						snprintf(tempPath, PATH_MAX,"%s/%s", dirName, direntp->d_name); 
						closedir(dirp); 
						search(target, tempPath);
						exit(0);  /*child eşleşme sayısı ile exit yaptı*/
                    }
                }
				else if(childPid == -1)
				{
                    perror("Fork başarısız !\n");
                    closedir(dirp);
                    return;	
				}
            }				
            else if( (direntp->d_type) == DT_DIR && strcmp(direntp->d_name,".") != 0 && strcmp(direntp->d_name, "..") != 0 )   /* path, directory ise*/    
            {
            	pid=fork();
            	if(pid == 0)
            	{
		         	char tempPath[PATH_MAX];  /* temp directory */
					snprintf(tempPath, PATH_MAX,"%s/%s", dirName, direntp->d_name); 
					match(target, tempPath);
					exit(0);
				}
				else if( pid != -1)
				{
					wait(NULL); 
				}	
            }
		}  
    }
    
    closedir(dirp);
    return ;
}

/**
 * @param target hedef kelime
 * @param fileName kelimenin aranacağı dosya
 * @return void
 */
void search(char* target, const char* fileName){
    
    FILE *fp, *log, *matches;
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
    
    log= fopen("log.log","a"); /* log dosyası açıldı */
    if( log == NULL){ /* dosya açılamazsa*/
        perror("Dosya açılamadı !\n");
        return;
    }
    
    matches= fopen("totalMatches.txt","a"); /*eşleşme oldugu zaman 1 yazılcak*/
        
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
                    fprintf(log, "%s: [%d, %d] %s first character is found.\n",fileName,row,column,target);
                    fprintf(matches, "1 ");  /*eşleşme oldu , 1 yazıldı */
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
    fclose(matches);
   
    return;
}

/**
 * @return totalMatches.txt den okunan 1'lerin sayısı (toplam eşleşme) 
 *
 */
int total(){
	
	FILE* input = fopen("totalMatches.txt", "r");
	int match;
	int totalMatch = 0;
	while(!feof(input)){

		fscanf(input, "%d", &match);
		totalMatch+= match;
	}
	fclose(input);
	
	remove("totalMatches.txt");
return totalMatch-1;
}

