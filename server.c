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

#define PORT 7777
#define SIZE 1024

void* multiconnect(void* socketdesc);

struct clients{
 struct sockaddr_in clientAddr;
 struct clients *next;
};

int main(int argc, char *argv[])
{
    int socketdesc, clsock;
    int *nwsock;
    struct sockaddr_in srv, clt;
    int structSize;
    //char msg[SIZE]; char response[SIZE];

    // Creating socket
    socketdesc = socket(AF_INET, SOCK_STREAM, 0);
    if(socketdesc == -1)
        perror("Error on Soket");

    puts("socket made");

    // Editting Server socket
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = INADDR_ANY;
    memset(&(srv.sin_zero), '\0', 8);

    // Bind
    if(bind(socketdesc, (struct sockaddr *)&srv, sizeof(srv)) == -1)
        perror("Error on Bind");

   // Start listen the port
   if(listen(socketdesc, 20) == -1  )
        perror("Error on Listen");

    structSize = sizeof(clt);

    // Accept
    while((clsock = accept(socketdesc , (struct sockaddr *)&clt, (socklen_t*)&structSize))){
        puts("new connection accepted");
        pthread_t th;
        nwsock = malloc(sizeof(nwsock));
        *nwsock = clsock;

        if(pthread_create(&th, NULL, multiconnect, (void*) nwsock)<0){
            perror("thread creation failed");
            return 1;
        }
        puts("new client accepted and thread occupied for it");

    }

    if(clsock == -1){
        perror("Error on Accept");
        return 1;
    }

    return (EXIT_SUCCESS);
}


// each of these thread will handle a connection to a client
void *multiconnect(void* socketdesc){
    int clsock = *(int*)socketdesc;
    int check, byte;
    char reply[SIZE], clmsg[SIZE];

    //rcv msgs from the client
    while((check = recv(clsock, &clmsg,SIZE, 0))>0){
        printf("client's msg: %s",clmsg);

        strcat(reply, "ack from server");
        byte = send(clsock, reply, strlen(reply), 0);
        if(byte == -1)
            perror("Error on Recv");
        else if(byte == 0)
            printf("Connection is close\n");

    }

    if(check==0){
        puts("client disconnected");
    }
    else if(check==-1){
        perror("recv failed");
    }

    //clean up
    free(socketdesc);
    close(clsock);
    pthread_exit(NULL);
    return NULL;
}





/*
//old version
// Accept
    clsock = accept(socketdesc , (struct sockaddr *)&clt, &structSize);
    if(clsock == -1)
        perror("Error on Accept");
    // Recv msg from client
    byte = recv(clsock, &msg, SIZE-1, 0);
    if(byte == -1)
        perror("Error on Recv");
    else if(byte == 0)
        printf("Connection is close\n");


while(1)
    {
        puts("accept and receive msgs");
        printf("msg is : %s \n", msg);
        //Send ack to client we received msg
        strcpy(response, "ack from server");
        byte = send(clsock, response, strlen(response), 0);
        if(byte == -1)
            perror("Error on Recv");
        else if(byte == 0)
            printf("Connection is close\n");
        //supprimer les anciens messages
        memset(msg, 0, SIZE);
        byte = recv(clsock, &msg, SIZE-1, 0);
        if(byte == -1)
            perror("Error on Recv");
        else if(byte == 0)
            printf("Connection is close\n");

    }
*/