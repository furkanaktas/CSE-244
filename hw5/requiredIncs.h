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
#include <pthread.h>	/* thread functions*/
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#define FIFO_PERM (S_IRUSR | S_IWUSR)
#define FIFO_MODES 0666		/*open permission read-write */
#define miliSecond 1000.0
#define MILLION 1000000L
#define NUM_THREADS	300

/**
*	threadVar (thread Variables)
*/
typedef struct threadVar
{
	int numOfStrings;
	int numOfDir;
	int numOfFiles;
	int numOfLines;
	int numOfThreads;
	int numOfProcess;
	int maxOfThread;	
}threadVar;

typedef struct signalVar
{
	int numOfStrings;
	int numOfDir;
	int numOfShared;
	int numOfFiles;
	int numOfLines;
	int numOfThreads;
	int numOfProcess;
	int maxOfThread;	
}signalVar;

typedef struct sharedVar
{
	int numOfStrings;
	int numOfLines;
	char path[PATH_MAX];
}sharedVar;

/**
 *  Konsoldan gönderilen dosya ismi ve hedef kelime'yi gerekli işlemler 
 * vasıtasıyla, row ve column bilgisini verir.  
 * 
 * @param target hedef kelime
 * @param fileName kelimenin aranacağı dosya
 * @param threadId anlık thread id'si
 * @return void
 */
void search(char* target, long threadId, sharedVar* shrVar);


/**
 *   Konsoldan gelen klasör'ü açar.Recursion olarak klasör bitene kadar
 * klasörlere girer.Daha sonra dosyalarda hedef kelimeyi search fonksiyonu ile
 * arar.
    	
 * @param target hedef kelime
 * @param dirName klasör ve dosya ismi (döngüde değişiyor)
 * @return thread verilerini döndürür(toplan satır,eşleşme gibi)(struct)
 */
threadVar match(char* target, char *dirName);


#endif
