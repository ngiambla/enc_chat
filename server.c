/***********************************
File: server.c
Description: Secured Server Program
Authors: N. Giamblanco
***********************************/
#include "include.h"
#include "enc.h"


#define PORT 7802	/* well-known port */
#define BUFLEN	512	/* buffer length */

char userNames[5][10];   //Stores Known User Names.
char help[5][100];
char userNonce[5][110];  //Stores Nonces for Username.
int loggedin[5];      // Holding log-in state for each of the five members
int userbusy[5];      // A specific user is trying to authenticate. 


char pass[32]="thirtyfive";
struct sockaddr_in userIPs[5];
char *ip;

/******************************************
function: listUsers()
description: lists active users.
******************************************/
void listUsers(){

        int i=0,count=0;
        printf("Users:\n");
        for(i=0;i<5;++i){
		if(loggedin[i]==1){
                	printf("User: [%s]      IP:[%s]\n",userNames[i],inet_ntoa(userIPs[i].sin_addr));
			++count;
		}
        }
	if(count==0){
		printf("[No one is logged in...]\n");
	}

}

/******************************************
function: init()
description: provides the functionality to
	     initialize the server vals
******************************************/
void init(){
	int i=0;
	printf("Initializing.\n");
	printf("Session Passcode:\n");
	bzero(pass,32);
	getpasswrd(pass);
	for(i=0;i<5;++i){
		loggedin[i]=0;
		userbusy[i]=0;
	}
	strcpy(userNames[0],"r1cardo");
	strcpy(userNames[1],"leon4rdo");
	strcpy(userNames[2],"appl3");
	strcpy(userNames[3],"micr0");
	strcpy(userNames[4],"c0de");
	
	strcpy(help[0],"+--[HELP]---------------+");
	strcpy(help[1],"| help - displays this  |");
	strcpy(help[2],"| quit - logoff         |");
	strcpy(help[3],"| ls   - lists users    |");
	strcpy(help[4],"+-----------------------+");
	
	listUsers();
	printf("Initialization Complete.\n");
}

/****************************************
function: sess_decode()
description: decodes the current session 
	     for handling messages.
****************************************/
int sess_decode(int sd, struct sockaddr_in cliaddr, int len)
{
        pdu_t pdu_in, pdu_out;
        char    *bp, buf[BUFLEN], msg[110],tmpname[10];
	char 	nonce[110];
	char    *response;
        int     nbytes, i=0,w=0,j=0;

        while(1) {
		bzero(pdu_out.data,220);
		bzero(pdu_out.sname,20);
		bzero(pdu_in.data,220);
		bzero(pdu_in.sname,220);
		bzero(msg,110);
		bzero(tmpname,10);
		bzero(nonce,110);
        	nbytes=recvfrom(sd,(void*)&pdu_in,sizeof(pdu_t),0,(struct sockaddr *)&cliaddr, &len);

		ip = inet_ntoa(cliaddr.sin_addr);
		printf("Receiving from: %s\n",ip);

		switch(pdu_in.type){

		case AUTH:
			dec(pass,pdu_in.sname,tmpname);
			dec(pass,pdu_in.data,msg);
			printf("About to Authenticate: %s",tmpname);
			for(i=0;i<5;++i){
				if(strcmp(tmpname,userNames[i])==0){
					if(strcmp(userNonce[i],msg)==0){
						
						pdu_out.type=AUTH;
						enc(pass,"s3rv3r",pdu_out.sname);
						enc(pass,userNames[i],pdu_out.data);
						loggedin[i]=1;
						userbusy[i]=0;
						userIPs[i]=cliaddr;
						nbytes=sendto(sd,(void*)&pdu_out,sizeof(pdu_t),0,(struct sockaddr *)&cliaddr, len);

						bzero(pdu_out.data,220);
						bzero(msg,110);
						pdu_out.type=INFO;
						printf("Notifying Users about: %s",userNames[i]);
						snprintf(msg,110,"*<<%s>> has joined the chat*",userNames[i]);
						enc(pass,msg,pdu_out.data);

						for(j=0;j<5;++j){
							if(loggedin[j]==1){
								nbytes=sendto(sd,(void*)&pdu_out,sizeof(pdu_t),0,(struct sockaddr *)&userIPs[j], len);
							}
						}
						for(j=0;j<5;++j){
							if(loggedin[j]==1){
								bzero(pdu_out.data,220);
								bzero(msg,110);
								pdu_out.type=LST;
								printf("Sending User List: %s",userNames[j]);
								snprintf(msg,110,"[USERS]: <<%s>>",userNames[j]);
								enc(pass,msg,pdu_out.data);
								nbytes=sendto(sd,(void*)&pdu_out,sizeof(pdu_t),0,(struct sockaddr *)&cliaddr, len);
							}
						}
						for(j=0;j<5;++j){
							
							bzero(pdu_out.data,220);
							bzero(msg,110);
							pdu_out.type=HELP;
							printf("Sending User Help: %d/5",j+1);
							snprintf(msg,110,"%s",help[j]);
							enc(pass,msg,pdu_out.data);
							nbytes=sendto(sd,(void*)&pdu_out,sizeof(pdu_t),0,(struct sockaddr *)&cliaddr, len);
							
						}
						break;
					}
				}
			}
			break;
		case CHAL:
			printf("Receiving Authentication Request\n");
			dec(pass,pdu_in.sname,tmpname);
			dec(pass,pdu_in.data,msg);
			printf("[DEC]: User=%s\n",tmpname);
			printf("[DEC]: Msg=%s\n",msg);
			for(i=0;i<5;++i){

				if(strcmp(tmpname,userNames[i])==0&&userbusy[i]==0&&loggedin[i]==0){
					printf("[ALERT] User Known.\n");
					userbusy[i]=1;
					pdu_out.type=RESP;
					w=challenge();
					snprintf(userNonce[i],110,"%d",w);
					snprintf(nonce,110, "%d-", w);
					strcat(nonce,msg);
					enc(pass,nonce,pdu_out.data);
					enc(pass,"s3rv3r",pdu_out.sname);
					break;
				}else if(strcmp(tmpname,userNames[i])==0&&loggedin[i]==1){
					pdu_out.type=BAD;
                       			snprintf(msg, sizeof(msg), "The username entered is already logged in.");
                        		printf("[ALERT] %s",msg);
								strcpy(pdu_out.sname,"s3rv3r");
                        		strcpy(pdu_out.data,msg);
					break;
				}else if(strcmp(tmpname,userNames[i])==0&&userbusy[i]==1){
					pdu_out.type=BAD;
                       			snprintf(msg, sizeof(msg), "The username entered is already authenticating.");
                        		printf("[ALERT] %s",msg);
								strcpy(pdu_out.sname,"s3rv3r");
                        		strcpy(pdu_out.data,msg);
					break;
				}else if(strcmp(tmpname,userNames[i])!=0&&i==4){
					pdu_out.type=BAD;
                       			snprintf(msg, sizeof(msg), "**User Unknown**");
                        		printf("[ALERT] %s",msg);
								strcpy(pdu_out.sname,"s3rv3r");
                        		strcpy(pdu_out.data,msg);
					break;
				}

			}

			nbytes=sendto(sd,(void*)&pdu_out,sizeof(pdu_t),0,(struct sockaddr *)&cliaddr, len);
			break;

		case MSG:
			pdu_out.type=BCAST;
			strcpy(pdu_out.data,pdu_in.data); //No decryption of user messages. *BENEFIT*
			dec(pass,pdu_in.sname,tmpname); //Ensure Sender will not receive duplicate message.
			enc(pass,tmpname,pdu_out.sname); //Replace Sender ID with [An0n]
			for(i=0;i<5;++i){
				if(loggedin[i]==1&&strcmp(userNames[i],tmpname)!=0){
					nbytes=sendto(sd,(void*)&pdu_out,sizeof(pdu_t),0,(struct sockaddr *)&userIPs[i], len);
				}
			}
			break;

		case LST:
			for(i=0;i<5;++i){
				if(loggedin[i]==1){
					bzero(pdu_out.data,220);
					bzero(msg,110);
					pdu_out.type=LST;
					printf("Sending List: %s",userNames[i]);
					snprintf(msg,110,"[USER]: <<%s>>",userNames[i]);
					enc(pass,msg,pdu_out.data);
		        		nbytes=sendto(sd,(void*)&pdu_out,sizeof(pdu_t),0,(struct sockaddr *)&cliaddr, len);
				}
			}	
			break;
		case HELP:
			for(j=0;j<5;++j){	
				bzero(pdu_out.data,220);
				bzero(msg,110);
				pdu_out.type=HELP;
				printf("Sending User Help: %d/5",j+1);
				snprintf(msg,110,"%s",help[j]);
				enc(pass,msg,pdu_out.data);
				nbytes=sendto(sd,(void*)&pdu_out,sizeof(pdu_t),0,(struct sockaddr *)&cliaddr, len);					
			}
			break;
		
		case QUIT:			
			dec(pass,pdu_in.sname,tmpname);
			dec(pass,pdu_in.data,msg);
			for(i=0;i<5;++i){
				if(strcmp(userNames[i],tmpname)==0){
					printf("[%s] has requested to leave.\n",tmpname);
					loggedin[i]=0;
					printf("[%s] is logged out.\n",tmpname);
					break;
				}
			}
			pdu_out.type=INFO;
			enc(pass,tmpname,pdu_out.sname);
			enc(pass," has left the chat",pdu_out.data);
			for(i=0;i<5;++i){
				if(loggedin[i]==1){
					printf("Notifying [%s] about <<%s>>.\n",userNames[i],tmpname);			
					nbytes=sendto(sd,(void*)&pdu_out,sizeof(pdu_t),0,(struct sockaddr *)&userIPs[i], len);
				}
			}
			break;
			

	}


        }
        close(sd);

        return(0);
}


int main(int argc, char **argv)
{
	int 	sd, new_sd, client_len, port;
	struct	sockaddr_in server, client;

	init();

	/* Create a UDP socket	*/	
	if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		fprintf(stderr, "Can't create a socket\n");
		exit(-1);
	}else{
		printf("Socket Created.\n");
	}
	printf("[Welcome to Ba$H...]");

	/* Bind an address to the socket	*/
	bzero((char *)&server, sizeof(struct sockaddr_in));

	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
		fprintf(stderr, "Can't bind name to socket\n");
		exit(1);
	}

	/* queue up to 5 connect requests  */
	listen(sd, 5);
	client_len=sizeof(client);
	sess_decode(sd, client,client_len);
}

