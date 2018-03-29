#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

//#define ServerIP "35.162.226.229"
#define ServerIP "127.0.0.1"
#define ServerPort 7777
#define SIZE 1024





int main(int argc, char** argv) {

    int srv_sock;
    struct sockaddr_in serverAddr;
    char text[SIZE],msg[SIZE],get_msg[SIZE];
    int byte;
    //Create Socket
    srv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(srv_sock == -1)
        perror("Error on Socket");

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(ServerPort);
    serverAddr.sin_addr.s_addr =inet_addr(ServerIP);
    memset(&(serverAddr.sin_zero), '\0', 8);

    // Connect to server
    if(connect(srv_sock, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) == -1)
        perror("Error on Connect");
    puts("Connected");
    while(1)
    {
        //Get message
        printf("type your msg >>> "); scanf("%s",msg);
        //Edit message
        if(msg == "close") //TODO : find way to close socket properly after testing
            close(socket);

        strcpy(text, "client ");
        strcat(text, ": ");
        strcat(text, msg);
        //Send message
        byte = send(srv_sock, text, strlen(text), 0);
        if(byte == -1)
            perror("Error on Send");
        else if(byte == 0)
            printf("Connection've been closed");
        //Get reply from server
        recv(srv_sock, &get_msg, SIZE-1, 0);
        printf("received message %s \n", get_msg);
    }
    //Close socket
    close(srv_sock);



    return (EXIT_SUCCESS);
}