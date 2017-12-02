
#include "requiredIncs.h"

key_t socKey =616161;
int shmidSocket;
int* socketNo;
int sock;
int shmid[PATH_MAX];
int clientNum;


void catchSigInt(int signo) {
	
	if(signo == SIGINT)
	{
		for (int i = 0; i < clientNum; ++i)
		{
			shmctl(shmid[i], IPC_RMID, NULL);
		}
		close(sock);
		shmctl(shmidSocket, IPC_RMID, NULL);
		exit(0);
	}
}

int main(int argc, char *argv[]){

	if(argc != 3 && argc !=2 )
	{
		fprintf(stderr,"Usage: %s <port #, id> <thpool size, k >\n",argv[0]);
		return -1;
	}
	else
	{
		signal(SIGINT,catchSigInt); /* signal catch*/

		shmidSocket = shmget(socKey,sizeof(int), IPC_CREAT | 0666);		/*sinyal handler için özel shared açıldı*/
		if (shmidSocket < 0)
		{				/*socket numarası client'a  shrmem ile aktarıldı*/
			perror("shmget");
			exit(-1);
		}
		socketNo = shmat(shmidSocket,NULL,0);
		*socketNo = atoi(argv[1]);

		int mysock;
		struct sockaddr_in server;
		char buff[PATH_MAX];
				
		if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){ /*create socket*/
			perror("Failed to create socket");
			exit(-1);
		}

		server.sin_family = AF_INET;
		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port = htons(*socketNo);

		if(bind(sock,(struct sockaddr*) &server, sizeof(struct sockaddr_in)) <0)
		{
			perror("bind failed!");
			close(sock);
			exit(-1);
		}

		listen(sock, 100);

		if(argc == 2){
			clientNum=0;
			do{
				mysock = accept(sock,(struct sockaddr*) 0, 0 );
				if(mysock == -1)
				{
					perror("Failed to accept");
				}
				else
				{
					memset(buff, 0, sizeof(buff));
					if( read(mysock, buff, sizeof(buff)) < 0)
						perror("reading stream error!");

					int rowA,colA,rowB,colB,clientID;
					char *tokens;
					
					tokens = strtok(buff,".");
					rowA = atoi(tokens);
					tokens = strtok(NULL,".");
					colA = atoi(tokens);
					
					tokens = strtok(NULL,".");
					clientID = atoi(tokens);
					
					//printf("%d  %d   %d \n",rowA, colA, clientID );


					rowB = rowA;
					colB = 1;
					
					
					key_t key = getpid()+clientNum;			/* her client için yeni Shared memory yer alındı*/
					results* result;
					shmid[clientNum] = shmget(key,sizeof(results), IPC_CREAT | 0666);		/*sinyal handler için özel shared açıldı*/
					if (shmid[clientNum] < 0)
					{				/*socket numarası client'a  shrmem ile aktarıldı*/
						perror("shmget");
						exit(-1);
					}
					result = shmat(shmid[clientNum],NULL,0);

					result->generatedA =0;
					result->generatedB =0;
					result->generatedX =0;
					
					pid_t pidP1;
					pidP1 = fork();

					if (pidP1<0)
					{
						perror("fork error!\n");
						close(mysock);
						exit(-1);
					}
					else if(pidP1 ==0)
					{
						/* generate */ 


						generateMatrix(&(*result).A,rowA,colA);
						generateMatrix(&(*result).B,rowB,colB);
						result->generatedA =1;
						result->generatedB =1;
					
						exit(0);
					}
					else
					{
						pid_t pidP2;
						pidP2 = fork();
						if (pidP2<0)
						{
							perror("fork error!\n");
							close(mysock);
							exit(-1);
						}
						else if(pidP2 ==0)
						{
							while(!(result->generatedA ==1 && result->generatedB ==1));		/* A,B genarate edilene kadar sonsuz döngü*/
							
							result->X = pseudoInverse(result->A,result->B);
							result->generatedX=1;
							/* solve */
							exit(0);
						}
						else
						{
							
							pid_t pidP3;
							pidP3 = fork();
							if (pidP2<0)
							{
								perror("fork error!\n");
								close(mysock);
								exit(-1);
							}
							else if(pidP3 ==0)
							{
								while(!(result->generatedX ==1));	 /* X sonucu elde edilene kadar sonsuz döngü*/
						
								/*verify*/

								Matrix error;
								error = matrixMul(result->A,result->X);
								error = matrixSub(error,result->B);
								error = matrixMul(matrixTranspose(error),error);
								result->normErr = sqrt(error.matrix[0][0]);		
								if(write(mysock, result, sizeof(results)) <0)	
								{
									perror("send failed!");
									close(sock);
									exit(-1);
								}
								char logName[PATH_MAX];
								snprintf(logName,PATH_MAX,"logs/%d-Server",clientID);
								FILE *log= fopen(logName,"a");
								writeToFile(log,result->A,"A=[");
								fclose(log);
								
								log= fopen(logName,"a");
								writeToFile(log,result->B,"B=[");
								fclose(log);
								
								log= fopen(logName,"a");
								writeToFile(log,result->X,"X=[");
								fclose(log);

	 							shmctl(shmid[clientNum], IPC_RMID, NULL);
								exit(0);
							}
							else
							{
								++clientNum;
								printf("Number of clients being served: %d\n",clientNum );
							}
						}
					}
					
					close(mysock);
				}

			}
			while(1);
		}
	}

	return 0;
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

void generateMatrix(Matrix* matrix,int row,int col)
{
	int i,j,count;
	struct timespec random;

	(*matrix).row= row;
	(*matrix).col = col;
	
	for(i=0;i<20;++i){

		for(j=0;j<20;++j)
			(*matrix).matrix[i][j] = -1;

	}

	clock_gettime(CLOCK_REALTIME,&random);
	srand(random.tv_nsec);

	for(i=0;i<row;++i){
		for(j=0;j<col;++j)
		{
			(*matrix).matrix[i][j] = rand()%100;
		}

	}
}

Matrix matrixTranspose(Matrix matrix)
{
	Matrix temp;
	for (int i = 0; i < matrix.row; i++) {
		for (int j = 0; j < matrix.col; j++) {
			temp.matrix[j][i] = matrix.matrix[i][j];
		}
	}
	
	temp.row = matrix.col;
	temp.col = matrix.row;
	return temp;
}

Matrix matrixCopy(Matrix a)
{
	Matrix x;
	x.row=a.row;
	x.col=a.col;
	for (int i = 0; i < a.row; i++)
		for (int j = 0; j < a.col; j++)
			x.matrix[i][j] = a.matrix[i][j];
	return x;
}

Matrix matrixSub(Matrix x, Matrix y)
{
	Matrix r;
	if (x.col == y.col && x.row == y.row)
	{	

		r.row = x.row;
		r.col = x.col;
		for (int i = 0; i < x.row; ++i)
			for (int j = 0; j < x.col; ++j)
				r.matrix[i][j] = x.matrix[i][j] - y.matrix[i][j];
	}
	return r;	
}

Matrix matrixMul(Matrix x, Matrix y)
{
	Matrix r;
	if (x.col == y.row)
	{	

		r.row = x.row;
		r.col = y.col;
		for (int i = 0; i < x.row; ++i)
			for (int j = 0; j < y.col; ++j)
				for (int k = 0; k < x.col; ++k)
					r.matrix[i][j] += x.matrix[i][k] * y.matrix[k][j];
	}
	return r;
}

void matrixShow(Matrix m)
{
	printf("\n");
	for(int i = 0; i < m.row; i++) {
		for (int j = 0; j < m.col; j++) {
			printf(" %8.2f", m.matrix[i][j]);
		}
		printf("\n");
	}
	printf("\n\n");
}

double determinant(double a[20][20],int n)
{
   int i,j,j1,j2;
   double det = 0;
   double m[20][20];
   if (n < 1) { /* Error */

   } else if (n == 1) { /* Shouldn't get used */
      det = a[0][0];
   } else if (n == 2) {
      det = a[0][0] * a[1][1] - a[1][0] * a[0][1];
   } else {
      det = 0;
      for (j1=0;j1<n;j1++) {
         for (i=1;i<n;i++) {
            j2 = 0;
            for (j=0;j<n;j++) {
               if (j == j1)
                  continue;
               m[i-1][j2] = a[i][j];
               j2++;
            }
         }
   
         det += pow(-1.0,j1+2.0) * a[0][j1] * determinant(m,n-1);
   		      
      }
   }
   return(det);
}

void coFactor(double a[20][20],int n,double b[20][20])
{
   int i,j,ii,jj,i1,j1;
   double det;
   double c[20][20];


   for (j=0;j<n;j++) {
      for (i=0;i<n;i++) {

         /* Form the adjoint a_ij */
         i1 = 0;
         for (ii=0;ii<n;ii++) {
            if (ii == i)
               continue;
            j1 = 0;
            for (jj=0;jj<n;jj++) {
               if (jj == j)
                  continue;
               c[i1][j1] = a[ii][jj];
               j1++;
            }
            i1++;
         }
         /* Calculate the determinate */
         det = determinant(c,n-1);

         /* Fill in the elements of the cofactor */
         b[i][j] = pow(-1.0,i+j+2.0) * det;
      }
   }
   return;
   
}
Matrix inversMatrix(Matrix matrix)
{    
	Matrix temp; 
	temp.row = matrix.row;
	temp.col = matrix.col;
	coFactor(matrix.matrix,matrix.row,temp.matrix);
   	temp = matrixTranspose(temp);

   	double det = determinant(matrix.matrix,matrix.row);	
	
	for (int i = 0; i < temp.row; ++i)
	{
		for (int j = 0; j < temp.col; ++j)
		{
			temp.matrix[i][j] = (1.0/det)*(temp.matrix[i][j]);
		}
	}
	
	return temp;

}

void householder(Matrix m, Matrix *R, Matrix *Q)
{
	Matrix q[m.row];
	Matrix z, z1;
	//z = m;
	z= matrixCopy(m);
	for (int k = 0; k < m.col && k < m.row - 1; k++) {
		double e[m.row], x[m.row], a;
		
		//z1 = matrix_minor(z, k);

		Matrix temp;
		temp.row = z.row;
		temp.col = z.col;
		for (int i = 0; i < k; i++)
			temp.matrix[i][i] = 1;
		for (int i = k; i < z.row; i++)
			for (int j = k; j < z.col; j++)
				temp.matrix[i][j] = z.matrix[i][j];
		
		z1 = temp;
		z = z1;
 		//z = matrixCopy(temp);
		//mcol(z, x, k);
		for (int i = 0; i < z.row; i++)
			x[i] = z.matrix[i][k];

		//a = vnorm(x,     m.row);

		double sum = 0;
		for (int i = 0; i < m.row; i++)
	    	sum += x[i] * x[i];
		
		a = sqrt(sum);


		if (m.matrix[k][k] > 0) 
			a = -a;
 
		for (int i = 0; i < m.row; i++)
			e[i] = (i == k) ? 1 : 0;
 
		//vmadd(x, e, a, e, m.row);

		for (int i = 0; i < m.row; i++)
			e[i] = x[i] + a * e[i];
		
		
		//vdiv(e, vnorm(e, m.row), e, m.row);
		double norm;
		sum = 0;
		for (int i = 0; i < m.row; i++)
			sum += e[i] * e[i];
		
		norm = sqrt(sum);

		for (int i = 0; i < m.row; i++)
			e[i] = e[i] / norm;

		//q[k] = vmul(e, m.row);

		Matrix temp2;
		temp2.row=m.row;
		temp2.col=m.row;
		for (int i = 0; i < m.row; i++)
			for (int j = 0; j < m.row; j++)
				temp2.matrix[i][j] = (-2) *( e[i] * e[j] );
		
		for (int i = 0; i < m.row; i++)
			temp2.matrix[i][i] += 1;
	 
		//q[k] = temp2;
		q[k] = matrixCopy(temp2);


		z = matrixMul(q[k], z1);
		//z = z1;
		//z= matrixCopy(z1);
	}
	
	//*Q = q[0];
	*Q = matrixCopy(q[0]);
	*R = matrixMul(q[0], m);
	for (int i = 1; i < m.col && i < m.row - 1; i++) {
		z1 = matrixMul(q[i], *Q);
		//*Q = z1;
		*Q = matrixCopy(z1);
		
	}
	 
	*R = matrixMul(*Q, m);;
	*Q = matrixTranspose(*Q);
}
Matrix QRfactorization(Matrix A, Matrix *R, Matrix *Q, Matrix B)
{
	householder(A, R, Q);
	*Q = matrixTranspose(*Q);
	*R = inversMatrix(*R);
	Matrix temp,result;
	temp = matrixMul(*R,*Q);
	result = matrixMul(temp,B);
	return result;
}
Matrix pseudoInverse(Matrix A, Matrix B)
{
	Matrix transA,temp;
	transA =  matrixTranspose(A);
	temp = matrixMul(transA,A);
	temp = inversMatrix(temp);
	temp = matrixMul(temp,transA);
	temp = matrixMul(temp,B);
	return temp;

}






