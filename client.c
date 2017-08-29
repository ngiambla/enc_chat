/***********************************
File: client.c
Description: Secured Client Program
Authors: N. Giamblanco
***********************************/
#include"include.h" 
#include <pthread.h>
#include <ncurses.h>
#include <unistd.h>
#include "enc.h"

//Global Variable Definitions
	
	char name[10]; //; Client User Name
	char key[32]; //; Client/Server Session Key
	int reg=0, done=0;  
	int sockfd; //; Socket Descriptor
	struct sockaddr_in servaddr,cliaddr;

//Thread Mutex
	pthread_mutex_t mutexsum = PTHREAD_MUTEX_INITIALIZER; 

//NCURSES definitions for windows...
	int parent_x, parent_y, new_x, new_y;
    	int in_winsize = 4; //3
	int line=1; // Line position of top
	int input=1; // Line position of top
	WINDOW *msgwin;
  	WINDOW *inputwin;


/***********************************************************************
function: sendMessage()
description: updates screen and sends message to server Used by a thread.
*************************************************************************/
void *sendMessage(){

	pdu_t pdu_out;
	char str[95];
	char msg[105];

    while(1)
    {
	bzero(str,95);
	bzero(msg,105);
	bzero(pdu_out.data,220);
	bzero(pdu_out.sname,20);
        // Get user's message
	mvwgetnstr(inputwin, 1, 4, str, 95);	


	// Format Message for Display...
        snprintf(msg, sizeof(msg), "[%s]%s%s", name, ": ", str);

	enc(key,str,pdu_out.data);
	enc(key,name,pdu_out.sname);

        // Check for quiting
        if(strcmp(str,"quit")==0)
        {
	    pdu_out.type=QUIT;
            done = 1;
            // Sending Off Exit.
	    sendto(sockfd, (void*)&pdu_out, sizeof(pdu_t), 0,(struct sockaddr *)&servaddr, sizeof(struct sockaddr));
            pthread_mutex_destroy(&mutexsum);
            pthread_exit(NULL);
            close(sockfd);
      
	}if(strcmp(str,"help")==0){
	    pdu_out.type=HELP;
	}else if(strcmp(str, "ls")==0){
	    pdu_out.type=LST;
	}else{
	   pdu_out.type=MSG;
	}

        // scroll the top if the line number exceed height
        pthread_mutex_lock (&mutexsum);

        // Send message to server
	if(strcmp(str,"")!=0){
        	sendto(sockfd, (void*)&pdu_out, sizeof(pdu_t), 0,(struct sockaddr *)&servaddr, sizeof(struct sockaddr));
	}
        // write it in chat window (top)
	mvwprintw(msgwin,++input,2,msg);
	wclear(inputwin);
	box(inputwin,0,0);
    	mvwprintw(inputwin, 1, 1, "$:  ");
        // scroll the bottom if the line number exceed height
       if(input==parent_y/2+9){
                        wclear(msgwin);
                        box(msgwin,0,0);
                        input=1;
	}
	wrefresh(msgwin);
        wrefresh(inputwin);
        pthread_mutex_unlock (&mutexsum);
    }



}

/***********************************************************
function: listener()
description: updates screen and responds to a server update.
************************************************************/
void *listener(){
	pdu_t pdu_in;
	char dname[10];
	char dMsg[110];
	char tname[15];
	char tempMsg[120];
    while(1)
    {
        //Receive message from server
	
	bzero(dname,10);
	bzero(dMsg,110);
	bzero(tname,15);
	bzero(tempMsg,120);
	bzero(pdu_in.data,220);
	bzero(pdu_in.sname,20);
	recvfrom(sockfd,(void*)&pdu_in,sizeof(pdu_t),0,  NULL, NULL);

	pthread_mutex_lock (&mutexsum);

        switch(pdu_in.type){
		
        	case BCAST:
			dec(key,pdu_in.sname,dname);
			dec(key,pdu_in.data,dMsg);		
			snprintf(tempMsg, sizeof(tempMsg), "%s%s%s", dname, ": ", dMsg);
	        	mvwprintw(msgwin,++input,3, tempMsg);
                	break;
		case INFO:
			dec(key,pdu_in.sname,dname);
			dec(key,pdu_in.data,dMsg);
        	        wattron(msgwin, COLOR_PAIR(1));
			snprintf(tempMsg,sizeof(tempMsg),"[%s]-- %s",dname, dMsg);
        	        mvwprintw(msgwin,++input,7, tempMsg);
			wattron(msgwin, COLOR_PAIR(2));
			break;
		case LST:
			dec(key,pdu_in.sname,dname);
			dec(key,pdu_in.data,dMsg);
                	wattron(msgwin, COLOR_PAIR(4));
                	snprintf(tempMsg,sizeof(tempMsg),"List of Users: %s", dMsg);
                	mvwprintw(msgwin,++input,1, tempMsg);
                	wattron(msgwin, COLOR_PAIR(2));
                	break;			
		case HELP:
			dec(key,pdu_in.sname,dname);
			dec(key,pdu_in.data,dMsg);
                	wattron(msgwin, COLOR_PAIR(3));
                	snprintf(tempMsg,sizeof(tempMsg),"     %s", dMsg);
                	mvwprintw(msgwin,++input,1, tempMsg);
                	wattron(msgwin, COLOR_PAIR(2));
                	break;
			
	}

        //scroll the top if the line number exceed height
        if(input==parent_y/2+9){
            	wclear(msgwin);
		box(msgwin,0,0);
		input=1;
	}
      	wrefresh(msgwin);
        wrefresh(inputwin);
        pthread_mutex_unlock (&mutexsum);
    }


}




/*********************************
Main() of client.c
*********************************/

int main()
{
	int nbytes,n,chal=0;
	//PDU required for IN and OUT.
    	pdu_t pdu_in,pdu_out;
    	int connfd,len,recv=1;
    	char tempname[10], respname[10];
    	char tempmsg[110], respmsg[110];
    	char encryp[220],serverName[10];
	char *token,token1[100],token2[100];
    	char temppass[32],temp[10];
    	time_t rawtime;
    	struct tm * timeinfo;
	//Getting Current Time for Display.
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	// create socket in client side
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
 
	if(sockfd==-1){
	    printf("Socket Not Created. Error.\n");
	    exit(-1);
	}
	else{
	    printf("Socket Created.\n");
	}
 

	bzero(&servaddr, sizeof(servaddr));
 
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY; // ANY address or use specific address
	servaddr.sin_port = htons(7802);  // Port address

	printf("[Welcome to An0n...]\n\n");
	printf("[Time] %s\n",asctime (timeinfo));
    		while(reg==0){
			bzero(pdu_out.data,110); /*-----------------*/
			bzero(pdu_out.sname,10); /*Clear Char Arrays*/
			bzero(tempname,10);      /*-----------------*/
			bzero(temppass,32);
			bzero(respname,10);
			bzero(serverName,10);
			bzero(respmsg,110);
			bzero(tempmsg,110);
			bzero(key, 32);
			bzero(name,10);

			printf("User Name?\n~$: "); //Requesting For User Name, setting PDU type as CHAL. 
        	        pdu_out.type=CHAL;	    //Beginning AUTH phase.

        	        fgets(temp,10,stdin);
			strncpy(tempname,temp,strlen(temp)-1);
			printf("Authentication Required for, %s\n",tempname); //Requesting for Password. Should Hide terminal output.
	
			getpasswrd(temppass); //Getting Password.
			
			printf("Passcode: %s\n\n",temppass);
			chal=challenge();
			snprintf (tempmsg, 110, "%d", chal);
			bzero(encryp,220);
		
			enc(temppass,tempname,pdu_out.sname);//Encryping Nonce.
			enc(temppass,tempmsg,pdu_out.data);  //Encrypting Name
		
			printf("\n[CHAL]: Asking server to Authenticate...\n");
			nbytes=sendto(sockfd, (void*)&pdu_out, sizeof(pdu_t), 0,(struct sockaddr *)&servaddr, sizeof(struct sockaddr));
			printf("[RECV]: Got PDU.\n");
			recvfrom(sockfd,(void*)&pdu_in,sizeof(pdu_t),0,  NULL, NULL);
        		switch(pdu_in.type){
				case RESP:
					printf("[RESP]: Parsing Response.\n");
					dec(temppass,pdu_in.sname,respname);				
					dec(temppass,pdu_in.data,respmsg);

					token=strtok(respmsg,"-");
					strcpy(token1,token);
					token=strtok(NULL,"-");
					strcpy(token2,token);
					if(strcmp(token2,tempmsg)==0){
						printf(">> Sending info to server.\n");
						strncpy(key,temppass,32);
						strncpy(name,tempname,10);
						pdu_out.type=AUTH;
						bzero(pdu_out.data,110);
						bzero(pdu_out.sname,10);
						enc(key,token1,pdu_out.data);
						enc(key,name,pdu_out.sname);
						sendto(sockfd, (void*)&pdu_out, sizeof(pdu_t), 0,(struct sockaddr *)&servaddr, sizeof(struct sockaddr));
						printf("[RECV]: Got PDU.\n");
						recvfrom(sockfd,(void*)&pdu_in,sizeof(pdu_t),0,  NULL, NULL);
						switch(pdu_in.type){
							case AUTH:
								dec(key,pdu_in.sname,serverName);
								//printf("\n%s <--> %s",serverName,respname);
								if(strcmp(serverName,respname)==0){
									reg=1;
								}else{
									printf("[ERR] something weird happened.");
									exit(-1);
								}
								break;
						}
					}else{
						printf("[ERR] something weird happened.");
						exit(-1);
					}

					break;

				case BAD:
					printf("[AUTH] %s says:\n\n",pdu_in.sname);
					printf("<< %s >>",pdu_in.data);
					exit(-2);
        			}

		}

    	// Set up threads
    	pthread_t threads[2];
    	void *status;

	//Setting ncurses screen...
  	initscr();
  	start_color();
  	init_pair(1, COLOR_GREEN, COLOR_BLACK ); //Green Black Color Pair
  	init_pair(2, COLOR_WHITE, COLOR_BLACK ); //White Black Color Pair
  	init_pair(3, COLOR_CYAN, COLOR_BLACK ); //Cyan Black Color Pair
  	init_pair(4, COLOR_RED, COLOR_BLACK ); //Red Black Color Pair

  	getmaxyx(stdscr, parent_y, parent_x); //Setting Window for std screen.

  	msgwin = newwin(parent_y - in_winsize, parent_x, 0, 0);
  	inputwin = newwin(in_winsize, parent_x, parent_y - in_winsize, 0);
  	box(msgwin, 0 , 0);
  	box(inputwin, 0 , 0);		
  	scrollok(msgwin,TRUE);
  	scrollok(inputwin, TRUE);
  	keypad(inputwin, TRUE );

    	mvwprintw(msgwin, 1, 1, "*An0n Chat*");
	timeinfo = localtime ( &rawtime );
	mvwprintw(msgwin, 1, 14, asctime (timeinfo));
	line=2;
    	mvwprintw(inputwin, 1, 1, ">> ");
    	wrefresh(msgwin);
    	wrefresh(inputwin);

    	pthread_attr_t attr;
    	pthread_attr_init(&attr);
    	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    	pthread_create(&threads[0], &attr, sendMessage, NULL);
    	pthread_create(&threads[1], &attr, listener, NULL);
    	while(done==0);
    	endwin();
 
	return(0);
}
