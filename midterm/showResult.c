#include <stdio.h>
#include <stdlib.h>
#include <string.h>





int main(int argc, char *argv[])
{
	FILE* file,*file2;
	long int konum=0;
	int ilk=0;
	
	while(1)
	{
		
		int c;
		char n;
		if ( (file = fopen("logs/result.txt", "r") ) != NULL) 
		{
			

			if(fseek(file,konum,SEEK_SET) == 0){
				file2 = fopen("logs/showResult.log", "a");
				
	    		while ((c = getc(file)) != EOF){
	    			printf("%c", c);
	    			fprintf(file2, "%c",c );
	    			fflush(file2);
	    			konum=ftell(file);
	    		}
	    	}
	    	fclose(file2);
	    		
			

		}
		else{
			exit(0);
		}
		fclose(file);
		

	}

	return 0;	
}	