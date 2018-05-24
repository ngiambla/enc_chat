#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>


int challenge();
void passEnforce(char * pass);
void getpasswrd(char * pass);
void enc(char * pass, char * data, char * encry);
void dec(char * pass, char * data, char * decry);
