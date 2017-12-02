#ifndef REQUIREDINCS_H
#define REQUIREDINCS_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> 		/*tolower()*/
#include <string.h>     /* strcmp()*/
#include <unistd.h>     /*fork() pid_t */
#include <sys/wait.h>	/* wait()*/
#include <errno.h>		/* for errors */
#include <dirent.h>		/*for directory*/


/**
 *  Konsoldan gönderilen dosya ismi ve hedef kelime'yi gerekli işlemler 
 * vasıtasıyla, row ve column bilgisini verir.  
 * 
 * @param target hedef kelime
 * @param fileName kelimenin aranacağı dosya
 * @return void
 */
void search(char* target, const char* fileName);


/**
 *   Konsoldan gelen klasör'ü açar.Recursion olarak klasör bitene kadar
 * klasörlere girer.Daha sonra dosyalarda hedef kelimeyi search fonksiyonu ile
 * arar.
    	
 * @param target hedef kelime
 * @param dirName klasör ve dosya ismi (döngüde değişiyor)
 * @return void
 */
void match(char* target, char *dirName);


/**
 *    totalMatches.txt 'deki 1 leri toplar.Bu sayede toplam eşleşme sayısı 
 * bulunur.
 *
 * @return totalMatches.txt den okunan 1'lerin sayısı (toplam eşleşme)
 */
int total();

/*References
 * https://www.quora.com/What-is-the-difference-between-a-wait-status-and-WEXITSTATUS-status-macro
 * http://joequery.me/code/snprintf-c/
 */

#endif
