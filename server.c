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

#define PORT 7777
#define SIZE 1024
//global variable
bool running = true;// tenté d'avoir une var globale pour arrêter le serveur

void* multiconnect(void* socketdesc);
void* readcmd(void*);
void printdefault();

int main(int argc, char *argv[])
{
    int socketdesc, clsock;
    int *nwsock;
    struct sockaddr_in srv, clt;
    int structSize;

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

    structSize = sizeof(clt);
    puts("enter to start server");

    // manage the user CLI inputs form server side (more powerfull?)
    pthread_t cliserver;
    if(pthread_create(&cliserver, NULL, readcmd, NULL)<0){
        perror("thread creation failed");
        return 1;
    }

    // Accept
    while((clsock = accept(socketdesc , (struct sockaddr *)&clt, (socklen_t*)&structSize)) && running){
        //important to put running on the right since it first checks the left side
        // TODO trouver un moyen d'arreter le serveur correctement
        puts("new connection accepted");
        pthread_t th;
        nwsock = malloc(sizeof(nwsock));
        *nwsock = clsock;

        if(pthread_create(&th, NULL, multiconnect, (void*) nwsock)<0){
            perror("thread creation failed");
            return 1;
        }
        //pthread_join(th, NULL);
        puts("new client accepted and thread occupied for it");
    }

    if(clsock == -1){
        perror("Error on Accept");
        return 1;
    }

    return (EXIT_SUCCESS);
}

void* readcmd(void* unused){
    printf("running is: %d",running);
    while(running){
        char cmd[SIZE];
        fgets(cmd,SIZE,stdin); // read command from CLI
        if(cmd[0] == '\n' && cmd[1] == '\0'){
            puts("server started");
        }
        else{//process the command/readcmd
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
                    //how to shut down server properly?
                    break;
                default:
                    printdefault();
                    break;
            }
        }
    }
    return NULL;
}

void printdefault(){ //annoying to write each time the printf
    puts("commands: -hh (help), -ak/-av [key/val] (add key/val), -dk/-dv [key/val] (delete key/val)");
}


// each of these thread will handle a connection to a client
void *multiconnect(void* socketdesc){
    int clsock = *(int*)socketdesc;
    int bytesread, byte;
    char reply[SIZE], clmsg[SIZE];

    //rcv msgs from the client
    while((bytesread = recv(clsock, clmsg,SIZE-1,0))>0){
        clmsg[bytesread+1]='\0';
        printf("%s",clmsg);
        strcpy(reply, "ack from server"); //response to client
        byte = send(clsock, reply, strlen(reply)+1,0); //in send, we know size of string so we can use strlen
        if(byte == -1)
            perror("Error on Recv");
        else if(byte == 0)
            printf("Connection is close\n");
        memset(reply, 0, SIZE);
        memset(clmsg, 0, SIZE);
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