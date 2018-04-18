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

#define PORT 7777
#define MSGSIZE 1024
#define STORESIZE 1000 // modify later for dynamic size

//global variable
bool running = true;// tenté d'avoir une var globale pour arrêter le serveur

// Key value storage...
typedef struct KVstore KVstore;
struct KVstore{
    int key;
    char* value;
    int *kv_array;
    size_t used;
    size_t size;
    //TODO rajouter un size pour savoir jusqu'à combien faire la boucle pour éviter de faire boucle sur STORESIZE?
    //int leng;
};

KVstore kv; // malloc(sizeof(KVstore)*STORESIZE); //here or in main?

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
void printdefault();

int main(int argc, char *argv[])
{
    int socketdesc, clsock;
    int *nwsock;
    struct sockaddr_in srv, clt;
    int structsize;

    /* testing the kvstore
    kv[0].key = 5;
    kv[0].value = "test";*/

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
void initKVstore(KVstore *a, size_t initialSize){
  a->kv_array = (int *)malloc(initialSize * sizeof(int));
  a->used = 0;
  a->size = initialSize;
}

//insert element into kv array and resize if necessary
void insertKV(KVstore *a, int element) {
  // a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
  // Therefore a->used can go up to a->size
  if (a->used == a->size) {
    a->size *= 2; //TODO check if it works...
    a->kv_array = (int *)realloc(a->kv_array, a->size * sizeof(int));
  }
  a->kv_array[a->used++] = element;
}

//free kv array at the end
void freeKVstore(KVstore *a) {
  free(a->kv_array);
  a->kv_array = NULL;
  a->used = a->size = 0;
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
    while((bytesread = recv(clsock, clmsg,MSGSIZE-1,0))>0){
        //KVstore kv;
        //initialize kv array
        if(i == 0){
          initKVstore(&kv, 1);
          //it is unecessary to print sizeof the array because
          //it's impossible to the compiler to know the real allocated size.
          //So it returns the size of the pointer.
        }

        clmsg[bytesread+1]='\0';

        if(strlen(clmsg) != 0){
          i++;
          insertKV(&kv, i);
          printf("%d\n", kv.used);  // print number of elements
        }

        /*kv[0].key = 5;
        kv[0].value = "test";*/
        printf("client %i said %s",persID->id, clmsg);

        //TODO analyse clmsg
        //TODO check regex    regcomp
        //regex
        int err;
        regex_t preg;
        const char *str_regex = "www\\.[-_[:alnum:]]+\\.[[:lower:]]{2,4}";

        err = regcomp (&preg, str_regex, REG_NOSUB | REG_EXTENDED);
        if (err == 0) {
           int match;
           match = regexec (&preg, clmsg, 0, NULL, 0);
           regfree (&preg);
           if (match == 0){
              printf ("%s is valid\n", clmsg);
              //send message to the client
              snprintf(reply,sizeof(reply),"client %d, your message is valid", persID->id);//response to client
              byte = send(clsock, reply, strlen(reply)+1,0); //in send, we know MSGSIZE of string so we can use strlen
              if(byte == -1) perror("Error on Recv");
              else if(byte == 0) printf("Connection is close\n");

              //analyse message
              if(clmsg[0] == 'a'){      //add in the kv
                //TODO add value from clmsg[2] until end
              }
              if(clmsg[0] == 'r'){      //read in the kv
                //TODO search for the value at a certain key and print it
              }
              if(clmsg[0] == 'd'){      //delete in the kv
                //TODO read + delete this line in the kv
              }
              if(clmsg[0] == 'm'){      //modify in the kv
                //TODO read + modify this line in the kv
              }
              if(clmsg[0] == 'p'){      //print all the values in the kv
                //TODO parcours tout le kv et affiche chaque value
              }
              if(clmsg[0] == 'q'){      //a client have left
                printf("bye bye client %d\n", persID->id);
                freeKVstore(&kv); //just to free memory, but this need to be deleted aftwards
              }
           }
           else if (match == REG_NOMATCH){
              printf ("%s is not valide\n", clmsg);
              //send message to the client
              snprintf(reply,sizeof(reply),"client %d, your message is not valid! Try again", persID->id);//response to client
              byte = send(clsock, reply, strlen(reply)+1,0); //in send, we know MSGSIZE of string so we can use strlen
              if(byte == -1)
                  perror("Error on Recv");
              else if(byte == 0)
                  printf("Connection is close\n");
           }
         }

        //snprintf(reply,sizeof(reply),"ack to client %d", persID->id);//response to client
        /*byte = send(clsock, reply, strlen(reply)+1,0); //in send, we know MSGSIZE of string so we can use strlen
        if(byte == -1)
            perror("Error on Recv");
        else if(byte == 0)
            printf("Connection is close\n");*/
        memset(reply, 0, MSGSIZE);
        memset(clmsg, 0, MSGSIZE);

        //testing kvstore
        int k =0;
        //printf("kv[%i] has key = %i, value = %s\n",k,kv[k].key,kv[k].value);

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

void* readcmd(void* unused){
    while(running){
        char cmd[MSGSIZE];
        fgets(cmd,MSGSIZE,stdin); // read command from CLI
        if(cmd[0] == '\n' && cmd[1] == '\0'){
            puts("server started");
        }
        else if((int)strlen(cmd)>=6){
        //first of all check if the string lenght is >= 6 which is minimum to make something (-[][] [][return]=>6)
            printf("length of cmd: %i\n",(int)strlen(cmd)); //contains the string + the return key
            //process the command/readcmd
            switch(cmd[0]){
                case '-':
                    switch(cmd[1]){
                        case 'a':
                            switch(cmd[2]){
                                case 'k':
                                    puts("add via key");
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
                            switch(cmd[2]){
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
                            switch(cmd[2]){
                                case 'k':
                                    puts("read key");
                                    break;
                                case 'v':
                                    puts("read value");
                                    break;
                                default:
                                    printdefault();
                                    break;
                            }
                            break;
                        case 'h':
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
