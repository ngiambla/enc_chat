#include "enc.h"

#define SIZE 32



/***********************************************************************
function: genChallenge()
description: generates random value for challenge response.
***********************************************************************/
int challenge(){

	//srand(time(NULL));
	int r = rand(); 
	return r; 

}




/***********************************************************************
function: passEnforce()
description: enforces password requirements.
***********************************************************************/
void passEnforce(char * pass){
	int i=0,j=0;
	
	char vector[SIZE];

	printf("\n\n\n[SECURE]--computing password\n");
	for(i=0;i<SIZE;++i){
		srand((i^pass[i])%93+33);
		vector[i]=rand()%93+33;
		
		for(j=0;j<SIZE*4;++j){
			vector[i]=(char)(vector[i]^rand()%93+33);
		}

	}

	for(i=0;i<SIZE;++i){
		if(strlen(pass)<i){
			
			pass[i]=(char)(rand()%93+33);
		}
	}
	

	for (i=0;i<SIZE;++i){
		for(j=0;j<SIZE*8;++j){
			pass[i]=(char)((pass[i]^vector[i])%93+33);
		}
	}
	write(1,"\n--[done]\n",12);
}

/***********************************************************************
function: initVector()
description: enforces password requirements.
***********************************************************************/
void initVector(unsigned int vector[][32], char* pass){
int sum=0;
unsigned int xor=0;
int i=0,j=0;

	for(i=0;i<32;++i){
		sum=sum+pass[i];
		xor=xor^pass[i];
		srand(xor^(sum%93+33));

		for(j=0;j<32;++j){

		vector[i][j]=rand()%93+33;				

		}
	}



}


/***********************************************************************
function: genVector()
description: generators a new vector based on a previous vector
***********************************************************************/
void genVector(unsigned int vector[][32], unsigned int* prevVector){
int sum=0;
unsigned int xor=0;
int i=0,j=0;

	for(i=0;i<32;++i){
		sum=sum+prevVector[i];
		xor=xor^prevVector[i];
		srand(xor^(sum%93+33));

		for(j=0;j<32;++j){
		vector[i][j]=rand()%93+33;

		}

	}



}

/***********************************************************************
function: confuse()
description: provides randomness with a vector of random numbers.
***********************************************************************/

void confuse(unsigned int* randVec, unsigned int* initVec){
	int i=0;

	for(i=0;i<32;++i){

	srand(initVec[i]+i);
	randVec[i]=rand()%31;

	}

}

/***********************************************************************
function: enc()
description: encrypts message from password.
***********************************************************************/
void enc(char * pass, char * data, char * encry){

	int i=0;
	int j=0;
	int k=0;
	int sum=0;
	unsigned int tmp;
	unsigned char output[3];
	unsigned int swap[32];
	unsigned int vector[32][32];
	unsigned int randVec[32];
	

	initVector(vector,pass);
	
	for(j=0;j<32;++j){
		sum=sum+pass[j];
	}
	
	
	confuse(randVec,vector[sum%31]);
	
		for(i=0;i<strlen(data);++i){
			srand(vector[k][k]+i);
			tmp=data[i]^vector[k][randVec[k]];
                        genVector(vector,vector[randVec[k]]);
			for(j=0;j<32;++j){
				swap[j]=(vector[randVec[k]][j]^pass[j]);
			}
			tmp=tmp^swap[rand()%31];
			
			if(k==31){
			k=0;
			confuse(randVec,vector[rand()%31]);
			}
			else{
			++k;
			}
			snprintf(output,3,"%02X",tmp);
			
			strcat(encry,output);
			//printf("[OUTPUT]: %c [TMP]: %02X\n",output[0],tmp);
			//printf("%02X",tmp);
			
			//printf("Size of output: %d\n",strlen(output));
			//data[i]=output[0];
	
		}
	printf("\n");
	//printf("%s\n",out);
}


/***********************************************************************
function: dec()
description: decrypts message from password.
***********************************************************************/
void dec(char* pass, char* data, char * decry){

	int j=0;
	int k=0;
	int count=0;
	int sum=0;
	
	unsigned int tmp;
	unsigned int swap[32];
	unsigned int vector[32][32];
	unsigned int randVec[32];
	unsigned char out[3];

	initVector(vector,pass);

	
	for(j=0;j<32;++j){
		sum=sum+pass[j];
	}

	confuse(randVec,vector[sum%31]);
	//printf("Decoded Data:\n");
	
		while(sscanf(data,"%02X",&tmp)==1){
				
			srand(vector[k][k]+count);
			tmp=tmp^vector[k][randVec[k]];
                        genVector(vector,vector[randVec[k]]);
			for(j=0;j<32;++j){
                                swap[j]=(vector[randVec[k]][j]^pass[j]);
                        }
                        tmp=tmp^swap[rand()%31];
			
			
			if(k==31){

			k=0;
			confuse(randVec,vector[rand()%31]);
			}
			else{
			++k;

			}
			//printf("%c", (isprint(tmp) ? tmp : '\n'));
			snprintf(out, 2, "%c", (isprint(tmp) ? tmp : '\n'));
                        strcat(decry,out);     
		       	
                	data+= 2;
			++count;

        	}
	
        printf("\n");



}

/***********************************************************************
function: getpasswrd()
description: hides terminal display temporarily until password is input.
***********************************************************************/
void getpasswrd(char * pass){



struct termios old, new;
  int nread;

  /* Turn echoing off and fail if we can’t. */
  if (tcgetattr (fileno (stdin), &old) != 0)
    exit(-1);
  new = old;
  new.c_lflag &= ~ECHO;
  if (tcsetattr (fileno (stdin), TCSAFLUSH, &new) != 0)
    exit(-1);

  /* Read the password. */
  printf("~$: ");
  fgets (pass, SIZE, stdin);

  /* Restore terminal. */
  (void) tcsetattr (fileno (stdin), TCSAFLUSH, &old);
		fflush(stdin);
		fflush(stdout);
  passEnforce(pass);
}
