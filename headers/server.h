#ifndef SERVER_H_
#define SERVER_H_
/*
 * SERVER.H :
 *
 * Header file for the KV store server part
 *
 * contains all global variables, prototypes and
*/

// INCLUDES
// we decided to put these in the header for simplicity and readability reasons since it's not a big project and we only use it once
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <regex.h>
#include <ctype.h>
#include <sys/time.h>


// CONSTANTS
#define PORT 7777
#define MSGSIZE 1024
#define STORESIZE 1000 // modify later for dynamic size
#define VALUESIZE 50
#define SLEEPLENG 5


// Key value storage.
typedef struct{
    int key;
    char *value;
    size_t used; //indicate where we are in the array
    size_t size; //indicate the size of the array
}KVstore;

struct IDsock{ //on peut rajouter ici des trucs qu'on aurait besoin de passer
    int id;
    int sock;
};


// FUNCTIONS

//main functions operating the strings
void* multiconnect(void* socketdesc);
void* readcmd(void*);


// control functions and default messages
void processcmd(char* input);
int ctrlregex(char* msg);
void printdefault();

//functions to manipulate the KVstore through commands
void addpair(int newkey, char* newvalue);
void readpair(int key, char* value);
void deletepair(int key, char* value);
void modifyPair(int key, char* value1, char* value2);
void printKV();

#endif
