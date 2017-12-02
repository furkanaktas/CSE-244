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
#include <fcntl.h>		/*open */
#include <sys/stat.h>   /*mkfifo*/
#define FIFO_PERM (S_IRUSR | S_IWUSR)
#define FIFO_MODES 0666		/*open permission read-write */

/**
 *  Konsoldan gönderilen dosya ismi ve hedef kelime'yi gerekli işlemler 
 * vasıtasıyla, row ve column bilgisini verir.  
 * 
 * @param target hedef kelime
 * @param fileName kelimenin aranacağı dosya
 * @return bulunan eşleşme sayısı
 */
int search(char* target, const char* fileName);


/**
 *   Konsoldan gelen klasör'ü açar.Recursion olarak klasör bitene kadar
 * klasörlere girer.Daha sonra dosyalarda hedef kelimeyi search fonksiyonu ile
 * arar.
    	
 * @param target hedef kelime
 * @param dirName klasör ve dosya ismi (döngüde değişiyor)
 * @return tüm dosyalardaki toplam eşleşme sayısı
 */
int match(char* target, char *dirName);



/*References
 * hw2 --
 * https://www.quora.com/What-is-the-difference-between-a-wait-status-and-WEXITSTATUS-status-macro
 * http://joequery.me/code/snprintf-c/
 * hw3--
 * https://www.youtube.com/watch?v=4FtzkIol_K4
 * http://stackoverflow.com/questions/8611035/proper-fifo-client-server-connection
 */

#endif
