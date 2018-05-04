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

// Key value storage.
typedef struct{
    int key;
    char *value;
    size_t used; //size_t is a type guaranteed to hold any array index
    size_t size;
}KVstore;

KVstore *kv = NULL; // TODO donc ici on pourrait supprimer tous les KVstore *array dans les arguments

struct IDsock{ //on peut rajouter ici des trucs qu'on aurait besoin de passer
    int id;
    int sock;
};

//function to dynamically allocate memory to KVstore
void initKVstore(size_t initialSize);
void insertKV(int newkey, char *newvalue);
//ajouter une fonction pour nettoyer un peu l'array?
void freeKVstore();

//functions to manipulate the KVstore
void addpair(int newkey, char* newvalue);

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

	//initialize kv array
	initKVstore(2);

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

//itinialize Key value array with a array of initialSize length
void initKVstore(size_t initialSize){
	int i;
	kv = (KVstore*) malloc(initialSize * sizeof(KVstore));
	if (kv == NULL) {
		printf("ERROR: Memory allocation failure!\n");
		exit(1);
	}
	kv->used = 0; 
	kv->size = initialSize; 
	for(i=0;i<kv->size;i++){// faire de la place pour les strings
		kv[i].value = (char*) malloc(sizeof(char*));
		kv[i].key = -1;
	}
}

//insert element into kv array and resize if necessary
void insertKV(int newkey, char *newvalue) {
	
	// resize if number of items is equal to size
	if (kv->used == kv->size) {
		int i;
		size_t newsize = (size_t)(kv->size)*2;
		printf("resized array to: %zu\n",newsize);
		//resize with realloc the kv
		kv = (KVstore*) realloc(kv, newsize * sizeof(KVstore));
		for(i=kv->size;i<newsize;i++){// faire de la place pour les strings
			kv[i].value = (char*) malloc(sizeof(char*)); 
		}
		if (kv == NULL) {
			freeKVstore(kv);
			printf("ERROR: Memory allocation failure!\n");
			exit(1);
		}
		else{
			kv->size = newsize;
		}
	}
	/* if the string passed is not NULL, copy it */
	if (newvalue != NULL) {
		size_t length = strlen(newvalue);
		//array->value[array->used] = malloc(1 + length);
		int index = kv->used; // on ajoute un element
		kv->used++;	
		strncpy(kv[index].value, newvalue,length); // on insère la valeur
		kv[index].key = newkey; // on indique aussi la clé
	}
	else
		printf("newvalue is NULL");
			
}

//free kv array at the end
void freeKVstore() {
	int i;
	for(i=0;i<kv->size;i++){
		free(kv[i].value); // free all the string place
	}
	free(kv);
}

// 
void addpair(int newkey, char* newvalue){
	if(newkey == 0){ // we have to give a new key to the pair
		int i,j,possiblekey;
		bool check;
		for(i=0;i<kv->size+1;i++){//on check si une clé peut être utilisée
			check = true;
			for(j=0;j<kv->size;j++){ // on check tout l'array
				if(kv[j].key == i){
					check = false;
					break;
				}
				else{
					possiblekey = i;// on garde la valeur tant qu'elle est possible
				}	
			}
			if(check){
				break;
			}	
		}
		if(check){
			insertKV(possiblekey,newvalue);
		}				
	}else{
		insertKV(newkey, newvalue);
	}
}

void readpair(int key, char* value){
	int i;
	if(key==0){// we just have the value and want to read the key
		for(i=0;i<kv->size;i++){
			if(strcmp(kv[i].value,value)){ // we found the value and show the key
				printf("value - %s - has the key %d\n",kv[i].value, kv[i].key);
				break;
			}	
		}
	}
	else{ // we just have the key and want the value
		for(i=0;i<kv->size;i++){
			if(kv[i].key==key){ // we found the value and show the key
				printf("the key %d has value %s \n",kv[i].key, kv[i].value);
				break;
			}	
		}
	}
}	

void printKV(){
    int i,kvsize;
    kvsize = kv->used;
    for(i=0;i<kvsize;i++){
        printf("kv[%d].value is: %s and key is: %d\n",i,kv[i].value,kv[i].key);
    }
}

// each of these thread will handle a connection to a client
void *multiconnect(void* socketdesc){
    struct IDsock *persID = (struct IDsock*)socketdesc;
    int clsock = persID->sock;
    //int clsock = *(int*)socketdesc;
    int bytesread, byte;
    char reply[MSGSIZE], clmsg[MSGSIZE];
    //rcv msgs from the client
    // !!!!!!!!!!! le massage est rempli par des espaces blancs à la fin!!!!!!!!!
    while((bytesread = recv(clsock,clmsg,MSGSIZE-1,0))>0){
        //clmsg[bytesread+1]='\0';
        printf("bytesread: %d\n", bytesread);

        printf("client msg: %zu\n", strlen(clmsg));

        /*if(strlen(clmsg) != 0){
          i++;
          insertKV(kv, i); // TODO quelqu'un peut m'expliquer?
          //printf("%d\n", kv.used);  // print number of elements
        }*/

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
				
				// TODO traiter le string pour avoir la partie key et value séparé
				insertKV(100,clmsg); //add strcat (de 2 à strlen)
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
	int newkey, cmdlen;
	char* tok;
	char* mode;
	char value[MSGSIZE]; 
    while(running){
        char cmd[MSGSIZE];
        fgets(cmd,MSGSIZE,stdin); // read command from CLI
        if(ctrlregex(cmd)==0){// check the string input
            cmdlen = (int)strlen(cmd);
            printf("length of cmd: %i\n", cmdlen); //contains the string + the return key
            //substring = strstr(cmd, " ");
            //strncpy(substring, cmd+2,sizeof(cmd)-2);
            tok = strtok(cmd," ");
            mode = tok;
            printf("mode:%s\n",mode);
            tok = strtok(NULL, " ");
            if(tok != NULL)
				newkey = atoi(tok);
				
            printf("newkey:%d\n",newkey);
            tok = strtok(NULL, " \n"); 
            if(tok != NULL)
				strcpy(value,tok);
				
            printf("value:%s\n",value);
            
            
            if(strcmp(mode, "a")==0){
				puts("add via value");
				newkey = 1;
                addpair(newkey,cmd); //add strcat (de 2 à strleng)
                break;
			}else if(strcmp(mode, "ak")==0){
				puts("add via key");
			}else if(strcmp(mode, "r")==0){
				puts("read key");
			}else if(strcmp(mode, "rv")==0){
				puts("read value");
			}else if(strcmp(mode, "d")==0){
				puts("delete via key");
			}else if(strcmp(mode, "dv")==0){
				puts("delete via value");
			}else if(strcmp(mode, "m")==0){
				
			}else if(strcmp(mode, "mv")==0){
				
			}else if(strcmp(mode, "p")==0){
				printKV();
			}								
            
            /*
            //process the command/readcmd
            switch(cmd[0]){
                case 'a':
                    switch(cmd[1]){
                        case ' ':
							newkey = 0;
                            //puts("add via key");
                            addpair(newkey,cmd); //add strcat (de 2 à strleng)
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
                            readpair(0,"a 1 hello");
                            break;
                        case 'v':
                            puts("read value");
                            break;
                        default:
                            printdefault();
                            break;
                    }
                    break;
                case 'p':
                    printKV();
                    break;
                case 'q':
                    puts("stop server");
                    running = false;
                    freeKVstore(kv);
                    //how to shut down server properly?
                    break;
                default:
                    printdefault();
                    break;
            }
            */
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
