#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define ServerIP "127.0.0.1"
#define ServerPort 7777
#define SIZE 1024

int main(int argc, char** argv) {
    int sock;
    struct sockaddr_in srv;
    char text[SIZE], msg[SIZE], reply[SIZE];
    int byte;
    //Create Socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        perror("Error on Socket");

    srv.sin_family = AF_INET;
    srv.sin_port = htons(ServerPort);
    srv.sin_addr.s_addr = inet_addr(ServerIP);
    memset(&(srv.sin_zero), '\0', 8);

    // Connect to server
    if (connect(sock, (struct sockaddr *) &srv, sizeof (struct sockaddr)) == -1)
        perror("Error on Connect");
    else {
        puts("Connected");

        printf("\n");

        //comm with srv
        printf("'a vale' to add a value\n");
        printf("'ak key vale' to add a value with a key\n");
        printf("'r key' to read a value with a certain key\n");
        printf("'rv vale' to read a key with a certain value\n");
        printf("'d key' to delete a value with a certain key\n");
        printf("'dv vale' to delete a key with a certain value\n");
        printf("'m key new_value' to modify a value with a certain key\n");
        printf("'mv old_vale new_value' to modify a old_value with a new_value\n");
        printf("'p' to print all the entries in the Key Value Store\n");


        while (1) {
            memset(msg, 0, SIZE);
            memset(text, 0, SIZE);
            memset(reply, 0, SIZE);
            //Get message
            printf("'q' to close session\n");
            printf("\n");
            printf("type your msg >>> ");
            fgets(msg, SIZE + 1, stdin); //lire tout le string
            // quel que soit le string on l'envoie au serveur, sinon on doit contrôler plusieurs fois que c'est correct...
            if (msg[0] == 'q') {
                snprintf(text, sizeof (text), "%s", msg);
                printf("sending string %s", text);
                printf("closing connection");
                close(sock); //interrompre la connection
                return 0;
            } else {
                //TODO récupérer l'ID client perso
                snprintf(text, sizeof (text), "%s", msg);
                printf("sending string %s", text);

                //Send message
                byte = send(sock, text, strlen(text) + 1, 0);
                if (byte == -1)
                    perror("sending from client failed");
                else if (byte == 0)
                    printf("Connection've been closed");

                //Get reply from server
                if (recv(sock, reply, SIZE, 0) < 0) {
                    perror("recv failed");
                }

                printf("reply: %s \n", reply);

                if (strcmp(strstr(reply, "is valid"), "is valid") == 0) {
                    //Get reply from server
                    printf("waiting for reply\n");
                    if (recv(sock, reply, SIZE, 0) < 0) {
                        perror("recv failed");
                    };
                    printf("reply 2: %s \n", reply);
                } else {
                    printf("don't expect any second reply\n");
                }

            }
            printf("\n");
        }
    }
    close(sock); //Close socket
    return (EXIT_SUCCESS);
}
