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

#define TRUE  1
#define FALSE 0
#define PORT 7777
#define MSGSIZE 1024
#define STORESIZE 1000 // modify later for dynamic size

//global variable
bool running = true;// tenté d'avoir une var globale pour arrêter le serveur

/* Key value storage...
typedef struct KVstore KVstore; // quelle idée, cela sert à rien non?*/

typedef struct{
    int key;
    char **value;
    //int *kv_array;
    size_t used; //size_t is a type guaranteed to hold any array index
    size_t size;
    //int leng;
}KVstore;

KVstore kv;

struct IDsock{ //on peut rajouter ici des trucs qu'on aurait besoin de passer
    int id;
    int sock;
};

//function to dynamically allocate memory to KVstore
void initKVstore(KVstore *a, size_t initialSize);
void insertKV(KVstore *a , int element);
void freeKVstore(KVstore *a);

void* multiconnect(void* socketdesc);
void* readcmd(void*);
int ctrlregex(char* msg);
void printdefault();

int main(int argc, char *argv[])
{
    int socketdesc, clsock;
    int *nwsock;
    struct sockaddr_in srv, clt;
    int structsize;

	//KVstore kv;
	//initialize kv array
	initKVstore(&kv, 1);

    // Creating socket
    socketdesc = socket(AF_INET, SOCK_STREAM, 0);
    if(socketdesc == -1)
        perror("Error on Socket");

    // Editting Server socket
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = INADDR_ANY;
    memset(&(srv.sin_zero), '\0', 8);
    // means reuse the port
    int reuse = 1;
    if (setsockopt(socketdesc, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int)) == -1)
        perror("Can't set the reuse option on the socket");
    // Bind
    if(bind(socketdesc, (struct sockaddr *)&srv, sizeof(srv)) == -1)
        perror("Error on Bind");

    // Start listen the port, waiting queue limited to 20
    if(listen(socketdesc, 20) == -1  )
        perror("Error on Listen");

    structsize = sizeof(clt);
    puts("enter to start server");

    // manage the user CLI inputs form server side (more powerfull?)
    pthread_t cliserver;
    if(pthread_create(&cliserver, NULL, readcmd, NULL)<0){
        perror("thread creation failed");
        return 1;
    }
    int i = 1;//counter for clients
    struct IDsock *client;
    // Accept  TODO =>accept est bloquant, trouver une solution pour arrêter le serveur proprement
    while((clsock = accept(socketdesc , (struct sockaddr *)&clt, (socklen_t*)&structsize)) && running){
        //important to put running on the right since it first checks the left side
        // TODO trouver un moyen d'arreter le serveur correctement
        puts("new connection accepted");
        pthread_t th;
        nwsock = malloc(sizeof(nwsock));
        *nwsock = clsock;
        client = malloc(sizeof(struct IDsock));
        client->id = i;
        client->sock=clsock;

        if(pthread_create(&th, NULL, multiconnect, (void*) client)<0){
            perror("thread creation failed");
            return 1;
        }
        puts("new client accepted and thread occupied for it");
        i++;
    }

    if(clsock == -1){
        perror("Error on Accept");
        return 1;
    }

    return (EXIT_SUCCESS);
}

//itinialize Key value array
void initKVstore(KVstore *array, size_t initialSize){
  array->value = malloc(initialSize * sizeof(char*));
  if (array->value == NULL) {
        printf("ERROR: Memory allocation failure!\n");
        exit(1);
    }
  array->used = 0;
  array->size = initialSize;
}

//insert element into kv array and resize if necessary
void insertKV(KVstore *array, char element) {
  // a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
  // Therefore a->used can go up to a->size
  // if the number of used entries == size of the array, then we have to resize the kv_array
  if (array->used == array->size) {
    void *pointer;
    array->size *= 2;
    //resize with realloc
    pointer = (int *)realloc(array->value, array->size * sizeof(char*));
    if (array->value == NULL) {
      freeArray(array);
      printf("ERROR: Memory allocation failure!\n");
      exit(1);
    }
    array->value = pointer;
  }
  /* if the string passed is not NULL, copy it */
  if (element != NULL) {
    size_t length;

    length = strlen(element);
    array->value[array->used] = malloc(1 + length);
    if (array->value[array->used] != NULL)
        strcpy(array->value[array->used++], element);
  }
  // a->used is the number of used entries (0 at the beginning), it keeps track of the number of used entries
  else
    a->value[a->used++] = element;
}

//free kv array at the end
void freeKVstore(KVstore *array) {
  size_t i;
  /* Free all the copies of the strings */
  for (i = 0; i < array->used; ++i)
    free(array->value[i]);

  free(array->value);
  free(array);
  /*a->kv_array = NULL;
  a->used = a->size = 0;*/
}

// different modes:
// 0: add giving just the string
// 1: add giving key and string
void addelementKV(int mode, int newkey, char* newvalue){
	if(mode == 0){
		// add with string
	}else{
		// add with key and string
	}		// TODO: continue making functions here
}

void printKV(){
    int i,kvsize;
    kvsize = sizeof(kv)/sizeof(KVstore);
    for(i=0;i<kvsize;i++){
        printf("kv %d key is %d\n",i,kv.key);
    }
}

// each of these thread will handle a connection to a client
void *multiconnect(void* socketdesc){
    struct IDsock *persID = (struct IDsock*)socketdesc;
    int clsock = persID->sock;
    //int clsock = *(int*)socketdesc;
    int bytesread, byte;
    char reply[MSGSIZE], clmsg[MSGSIZE];
    int i = 0; //counter to check if clmsg is empty, increment if not empty
    //rcv msgs from the client
    // !!!!!!!!!!! le massage est rempli par des espaces blancs à la fin!!!!!!!!!
    while((bytesread = recv(clsock,clmsg,MSGSIZE-1,0))>0){
        clmsg[bytesread+1]='\0';

        if(strlen(clmsg) != 0){
          i++;
          insertKV(&kv, i); // TODO quelqu'un peut m'expliquer?
          //printf("%d\n", kv.used);  // print number of elements
        }

		// check regex and react
		if (ctrlregex(clmsg) == 0){
		  printf("client %i said a valid string: %s\n",persID->id, clmsg);
		  // TODO send message to the client why not put on end to wait for confirmation of modify of the kv store???
		  snprintf(reply,sizeof(reply),"client %d, your message is valid", persID->id);//response to client
		  byte = send(clsock, reply, strlen(reply)+1,0); //in send, we know MSGSIZE of string so we can use strlen
		  if(byte == -1) perror("Error on Recv");
		  else if(byte == 0) printf("Connection is close\n");

		  //analyse message
		  if(clmsg[0] == 'a'){      //add in the kv
			//TODO add value from clmsg[3] until end
			if(clmsg[1] == 'k'){
			  //add the value by the key
			} else {
			  //add the value and generate a key    --> default
			}
		  }
		  if(clmsg[0] == 'r'){      //read in the kv
			//TODO search for the value at a certain key and print it
			if(clmsg[1] == 'v'){
			  //read by the value
			} else {
			  //read by the key   --> default
			}
		  }
		  if(clmsg[0] == 'd'){      //delete in the kv
			//TODO read + delete this line in the kv
			if(clmsg[1] == 'v'){
			  //delete by the value
			} else {
			  //delete by the key   --> default
			}
		  }
		  if(clmsg[0] == 'm'){      //modify in the kv
			//TODO read + modify this line in the kv
			if(clmsg[1] == 'v'){
			  //modify by the value
			} else {
			  //modify by the key   --> default
			}
		  }
		  if(clmsg[0] == 'p'){      //print all the values in the kv
			//TODO parcours tout le kv et affiche chaque value
		  }
		  /*if(clmsg[0] == 'q'){      //a client have left
			printf("bye bye client %d\n", persID->id);
			freeKVstore(&kv); //just to free memory, but this need to be deleted aftwards
		  }*/
		}
		else{
		  printf("client %i said a not valid string: %s\n",persID->id, clmsg);
		  //send message to the client
		  snprintf(reply,sizeof(reply),"client %d, your message is not valid! Try again", persID->id);//response to client
		  byte = send(clsock, reply, strlen(reply)+1,0); //in send, we know MSGSIZE of string so we can use strlen
		  if(byte == -1)
			  perror("Error on Recv");
		  else if(byte == 0)
			  printf("Connection is close\n");
		}
        memset(reply, 0, MSGSIZE);
        memset(clmsg, 0, MSGSIZE);
    }
    if(bytesread==0){
        puts("client disconnected");
    }
    else if(bytesread==-1){
        perror("recv failed");
    }

    //clean up
    close(clsock);

    free(socketdesc);
    pthread_exit(NULL);
    return NULL;
}

//structure of a msg: REGEX controlling this
//a msg     --> add the msg and generate a key
//ak 111 msg    --> add the msg at key 111
//r 111     --> read the value at key 111
//rv msg     --> read the value corresponding to the msg
//d 111     --> delete the value at key 111
//dv msg     --> read the value corresponding to the msg
//m 111 msg     --> modify the value at key 111 with msg
//mv msg msg     --> modify the value corresponding to the msg with msg
//p   --> print all the values in the kv

int ctrlregex(char* msg){
	int err, match;
	regex_t preg;
	const char *str_regex = "([ard] .+)|(ak [1-9]{1,3} .+)|(rv .+)|(dv .+)|(m [1-9]{1,3} .+)|(mv .+ .+)|p|q";
	err = regcomp(&preg, str_regex, REG_NOSUB | REG_EXTENDED);
	if (err == 0) {// compilation of regex successful
		match = regexec (&preg, msg, 0, NULL, 0);
		regfree(&preg);
		return match;
	}
	else{
		regfree(&preg);
		return REG_NOMATCH;
	}
}

void* readcmd(void* unused){
    while(running){
        char cmd[MSGSIZE];
        fgets(cmd,MSGSIZE,stdin); // read command from CLI
        if(ctrlregex(cmd)==0){// check the string input
            printf("length of cmd: %i\n",(int)strlen(cmd)); //contains the string + the return key
            //process the command/readcmd
            switch(cmd[0]){
                case 'a':
                    switch(cmd[1]){
                        case ' ':
                            puts("add via key");
                            printf("content of kv: %d\n",kv.key);
                            break;
                        case 'v':
                            puts("add via value");
                            break;
                        default:
                            printdefault();
                            break;
                    }
                    break;
                case 'd':
                    switch(cmd[1]){
                        case 'k':
                            puts("delete via key");
                            break;
                        case 'v':
                            puts("delete via value");
                            break;
                        default:
                            printdefault();
                            break;
                    }
                    break;
                case 'r':
                    switch(cmd[1]){
                        case ' ':
                            puts("read key");
                            printKV();
                            break;
                        case 'v':
                            puts("read value");
                            break;
                        default:
                            printdefault();
                            break;
                    }
                    break;
                case 'q':
                    puts("stop server");
                    running = false;
                    freeKVstore(&kv);
                    //how to shut down server properly?
                    break;
                default:
                    printdefault();
                    break;
            }
        }
        else{
            printdefault();
        }

    }
    return NULL;
}
//mettre ça dans un .h plus tard
void printdefault(){ //annoying to write each time the printf
    puts("commands: -hh (help), -ak/-av [key/val] (add key/val), -dk/-dv [key/val] (delete key/val)");
}
