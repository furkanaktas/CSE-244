/* 
 * File:   main.c
 * Author: furkan
 *
 * Created on 26 Şubat 2017 Pazar, 20:31
 */

#include "requiredIncs.h"

int main(int argc, char** argv) {
    
    if(argc > 3){
    	printf("Usage: ./list target file\n");
    }
    else{
    	search(argv[1], argv[2]);
    }
    return (EXIT_SUCCESS);
}

/**
 * Function : search
 *  Verilen index'i belli kurala göre işleme sokarak satır ve stün konumunu
 * verir.
 * 
 * fp:     file pointer
 * row:    satır
 * column: stün
 * ch:     dosyadan okunan karkater
 * targetSize: hedef kelime uzunluğu
 * index: hedef kelime kontrolcüsü( 0 <=  <= (size-1))
 * gapRepeat: açılan txt'deki hedef string'in karakerleri arası boşluk sayısı
 * totalMatch: toplam eşleşme sayısı  
 * i: döngü kontrolcüsü
 */

void search(char* target, const char* fileName){
    
    FILE *fp;
    char ch;
    int targetSize,gapRepeat,indexTarget,row,column,totalMatch,i;
    
    targetSize = strlen(target);	
    if(targetSize <1 || target =='\0'){ /* hedef kelime sıkıntılıysa*/  
        perror("Eşleşme bulunamadı !\n"); 
        return;
    }
    
    for(i=0;i< targetSize;++i)       /* hedef kelime küçük harfe çevrildi.*/
        if((int)target[i] >= (int)'A' && (int)target[i] <= (int)'Z')
            target[i]= tolower(target[i]);
    
    fp= fopen(fileName, "r");
    if( fp == NULL){ /* dosya açılamazsa*/
        perror("Dosya açılamadı !\n");
        return;
    }
    
    indexTarget=0;        /*  hedef kelime index kontrolcüsü 0-length */
    row=1;
    column=1;
    gapRepeat=0;
    totalMatch=0;
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
                printf("[%d, %d] konumunda ilk karakter bulundu.\n",row,column);
                    fseek(fp,-indexTarget-gapRepeat,SEEK_CUR); 
                    column++;               /* dosya işaretçisi geri alındı */
                    indexTarget=0; 
                    totalMatch++;
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
    
    if(totalMatch==0)
        printf("Eşleşme bulunamadı !\n");
    else
        printf("\n%d adet %s bulundu. \n",totalMatch,target);
    
    fclose(fp);

    return;
}
