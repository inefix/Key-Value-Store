#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>


#define PORT 7777
#define SIZE 1024


 struct clients{
     struct sockaddr_in clientAddr;
     struct clients *next;
 };

int main(int argc, char *argv[])
{
    struct clients *head = NULL;
    int socket_fd, cl_sock;
    struct sockaddr_in serverAddr,new_clientAddr;
    int structSize,byte;
    char msg[SIZE]; char response[SIZE];

    // Creating socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd == -1)
        perror("Error on Soket");
    puts("socket made");

    // Editting Server socket
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    memset(&(serverAddr.sin_zero), '\0', 8);

    // Bind
    if(bind(socket_fd, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) == -1)
        perror("Error on Bind");

   // Start listen the port
   if(listen(socket_fd, 20) == -1  )
        perror("Error on Listen");

    structSize = sizeof(new_clientAddr);

    // Accept
    cl_sock = accept(socket_fd , (struct sockaddr *)&new_clientAddr, &structSize);
    if(cl_sock == -1)
        perror("Error on Accept");
    // Recv msg from client
    byte = recv(cl_sock, &msg, SIZE-1, 0);
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
        byte = send(cl_sock, response, strlen(response), 0);
        if(byte == -1)
            perror("Error on Recv");
        else if(byte == 0)
            printf("Connection is close\n");
        //supprimer les anciens messages
        memset(msg, 0, SIZE);
        byte = recv(cl_sock, &msg, SIZE-1, 0);
        if(byte == -1)
            perror("Error on Recv");
        else if(byte == 0)
            printf("Connection is close\n");

    }


    close(socket_fd);


    return (EXIT_SUCCESS);
}