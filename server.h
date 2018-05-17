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


// CONSTANTS 
#define PORT 7777
#define MSGSIZE 1024
#define STORESIZE 1000 // modify later for dynamic size
#define VALUESIZE 50

// GLOBAL VARIABLE

bool running = true;// tenté d'avoir une var globale pour arrêter le serveur

// Key value storage.
typedef struct{
    int key;
    char *value;
    size_t used; //size_t is a type guaranteed to hold any array index
    size_t size;
}KVstore;

KVstore *kv = NULL; // our main KV store array

struct IDsock{ //on peut rajouter ici des trucs qu'on aurait besoin de passer
    int id;
    int sock;
};

char rep_client[MSGSIZE] = "";

// FUNCTIONS

//main functions operating the strings
void* multiconnect(void* socketdesc);
void* readcmd(void*); 

//function to dynamically allocate memory to KVstore
void initKVstore(size_t initialSize);
void insertKV(int newkey, char *newvalue);
void freeKVstore();

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


