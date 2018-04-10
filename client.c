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
    int sock;
    struct sockaddr_in srv;
    char text[SIZE], msg[SIZE],reply[SIZE];
    int byte;
    //Create Socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1)
        perror("Error on Socket");

    srv.sin_family = AF_INET;
    srv.sin_port = htons(ServerPort);
    srv.sin_addr.s_addr =inet_addr(ServerIP);
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
        // quel que soit le string on l'envoie au serveur, sinon on doit contrôler plusieurs fois que c'est correct...
        if(msg[0] == 'q'){
            close(sock);//interrompre la connection
            return 0;
        }
        else{
            //TODO récupérer l'ID client perso
            snprintf(text,sizeof(text),"%s",msg);
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