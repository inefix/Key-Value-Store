#include <stdio.h>      //printf
#include <string.h>     //strcpy
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>       //close the socket
#include <arpa/inet.h>    //inet_addr


//#define ServerIP "35.162.226.229"
#define ServerIP "127.0.0.1"
#define ServerPort 7777
#define SIZE 1024

int main(int argc, char** argv) {

    int sock;
    struct sockaddr_in srv;
    char text[SIZE], msg[SIZE],reply[SIZE];
    int byte;
    
    // Creating socket (TCP)
    // Address Family - AF_INET (this is IP version 4)
    // Type - SOCK_STREAM (this means connection oriented TCP protocol)
    // Protocol - 0 [ or IPPROTO_IP This is IP protocol]
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1)
        perror("Error on Socket");

    srv.sin_family = AF_INET;
    srv.sin_port = htons(ServerPort);
    srv.sin_addr.s_addr = inet_addr(ServerIP);    //inet_addr convert an IP address to a long format
    memset(&(srv.sin_zero), '\0', 8);

    // Connect to server
    if(connect(sock, (struct sockaddr *)&srv, sizeof(struct sockaddr)) == -1)
        perror("Error on Connect");

    puts("Connected");

    //comm with srv
    while(1)
    {
        memset(msg, 0, SIZE);
        memset(text, 0, SIZE);
        memset(reply, 0, SIZE);
        //Get message
        printf("'q' to close session \n"); //TODO ajouter toutes les fonctions
        printf("type your msg >>> ");
        fgets(msg, SIZE+1, stdin); //lire tout le string
        if(msg[0] == 'q'){
            close(sock);
            return 0;
        }
        else{
            strcpy(text, "client: ");
            strcat(text, msg);
            printf("sending string  %s",text);
            //Send message
            byte = send(sock, text, strlen(text)+1,0);
            if(byte == -1)
                perror("sending from client failed");
            else if(byte == 0)
                printf("Connection've been closed");

            //Get reply from server
            if (recv(sock, reply, SIZE, 0)<0){
                perror("recv failed");
            };

            printf("reply: %s \n", reply);
        }
    }
    //Close socket
    close(sock);
    return (EXIT_SUCCESS);
}
