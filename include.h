#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>


/* List of Types for PDU */


// MSG: Used for Transferring Message Contents to Server.
#define MSG 'M'

// BCAST: Used to Broadcast message to all users.
#define BCAST 'B'

// LST: List all Active Participants.
#define LST 'L'

// ERR: Defines An Error Message to Be Sent
#define E 'E'

//QUIT: Requests to be removed from the session.
#define QUIT 'Q'

//INFO: Displays Info to Users...
#define INFO 'I'

//HELP: Displays Help menu to users...
#define HELP 'H'

/******************************************
* Modified March 10th, 2017
******************************************/
//AUTH: Allows for Authentication Procedure
#define AUTH 'S'

//BAD: Identifies Bad Authentication.
#define BAD 'Z' 

//CHAL: Challenge Packet
#define CHAL 'C'

//RESP: Response Packet
#define RESP 'R'

typedef struct pdu
{
	char type;
	char sname[20];
	char data[220];
}pdu_t;


