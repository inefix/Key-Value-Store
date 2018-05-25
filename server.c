/*
 * Server code of the KVstore
 * Description of the program:
 */

#include "server.h"
#include "dynamicArray.h"

KVstore *kv; // our main KV store array
char rep_client[MSGSIZE];
bool running;// tenté d'avoir une var globale pour arrêter le serveur

int readcount, writecount;
static pthread_mutex_t rmutex;// = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t wmutex;
static pthread_mutex_t readTry;
static pthread_mutex_t resource;

int block_key_add;
int block_key_modify;
int block_key_delete;

// main launching the socket server and distributing the thread to handle several clients
int main(int argc, char *argv[])
{

    int socketdesc, clsock;
    int *nwsock;
    struct sockaddr_in srv, clt;
    int structsize;
    running = true;
    strcpy(rep_client,"");

    readcount = writecount = 0;
    block_key_add = -1;
    block_key_modify = -1;
    block_key_delete = -1;

	//initialize kv array
	kv = NULL;
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
    int reuse = true;
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

		if (pthread_mutex_init(&rmutex, NULL) != 0){
        printf("\n mutex init failed\n");
        return 1;
    }
		if (pthread_mutex_init(&wmutex, NULL) != 0){
        printf("\n mutex init failed\n");
        return 1;
    }
		if (pthread_mutex_init(&readTry, NULL) != 0){
        printf("\n mutex init failed\n");
        return 1;
    }
    if (pthread_mutex_init(&resource, NULL) != 0){
        printf("\n mutex init failed\n");
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

		pthread_mutex_destroy(&rmutex);
		pthread_mutex_destroy(&wmutex);
		pthread_mutex_destroy(&readTry);
    pthread_mutex_destroy(&resource);

    return (EXIT_SUCCESS);
}


// each of these thread will handle a connection to a client
void *multiconnect(void* socketdesc){
    struct IDsock *persID = (struct IDsock*)socketdesc;
    int clsock = persID->sock;
    int bytesread, byte;
    char reply[MSGSIZE], clmsg[MSGSIZE];
    //rcv msgs from the client
    while((bytesread = recv(clsock,clmsg,MSGSIZE-1,0))>0){
        //clmsg[bytesread+1]='\0';
        //printf("bytesread: %d\n", bytesread);
        //printf("client msg: %zu\n", strlen(clmsg));
        clmsg[strlen(clmsg)-1] = '\0';    //in oder to delete the new line

		// check regex and react
		if (ctrlregex(clmsg) == 0){
			//client side input processing
			printf("client %i said a valid string: %s\n",persID->id, clmsg);
			snprintf(reply,sizeof(reply),"client %d, your message is valid", persID->id);//response to client
      printf("clsock : %d\n", clsock);
      byte = send(clsock, reply, strlen(reply)+1,0);

			snprintf(rep_client,sizeof(rep_client),"request done");//if add key

			//int iRet;
			//struct timeval tv;

			//iRet = gettimeofday (&tv, NULL); // timezone structure is obsolete
			//if (iRet == 0)
			//{
			//printf ("Seconds since epoch: %d\n",
					//(int)tv.tv_sec);
			//return 0;
			//}
			//else
			//{
			//perror ("gettimeofday");
			//}

			processcmd(clmsg);

      printf("Server is sending response\n");

      printf("%ld\n", strlen(rep_client));
      printf("%s\n", rep_client);
      printf("clsock : %d\n", clsock);

			byte = send(clsock, rep_client, strlen(rep_client)+1,0);
			if(byte == -1) perror("Error on Recv");
			else if(byte == 0) printf("Connection is close\n");

      printf("Server have send response\n");

		}
		else{
		  printf("client %i said a not valid string: %s\n",persID->id, clmsg);
		  //send message to the client
		  snprintf(reply,sizeof(reply),"client %d, your message is not valid! Try again", persID->id);//response to client
		  byte = send(clsock, reply, strlen(reply)+1,0);
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

    close(clsock);

    free(socketdesc);
    pthread_exit(NULL);
    return NULL;
}

// Regex controlling the correct way to give a request
int ctrlregex(char* msg){
	int err, match;
	regex_t preg;
	const char *str_regex = "(^[ad] .+)|(^ak [[:digit:]]+ .+)|(^r [[:digit:]]+)|(^rv .+)|(^dv .+)|(^m [[:digit:]]+ .+)|(^mv .+ .+)|^p|^q";
	//structure of a msg: REGEX controlling this
	//a msg     --> add the msg and generate a key
	//ak 111 msg    --> add the msg at key 111
	//r 111     --> read the value at key 111
	//rv msg     --> read the value corresponding to the msg
	//d 111     --> delete the value at key 111
	//dv msg     --> read the value corresponding to the msg
	//m 111 msg     --> modify the value at key 111 with msg
	//mv msg msg2     --> modify the value corresponding to the msg with msg2
	//p   --> print all the values in the kv
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

// Server side command input
void* readcmd(void* unused){
    while(running){
        char cmd[MSGSIZE];
        fgets(cmd,MSGSIZE,stdin); // read command from CLI
        if(ctrlregex(cmd)==0){// check the string input
            cmd[strlen(cmd)-1] = '\0'; //in oder to delete the new line

            //int iRet;
			//struct timeval tv;

			//iRet = gettimeofday (&tv, NULL); // timezone structure is obsolete
			//if (iRet == 0)
			//{
				//printf ("Seconds since epoch: %d\n",(int)tv.tv_sec);
			//}
			//else
			//{
				//perror ("gettimeofday");
			//}

            processcmd(cmd);
        }
        else{
            printdefault();
        }
    }
    return NULL;
}

// takes decision to manipulate KVstore based on the input
void processcmd(char* input){
	int newkey;
	char* tok;
	char* mode;
	char value[MSGSIZE], value2[MSGSIZE];

	tok = strtok(input," ");
	mode = tok;      //première partie du string
	printf("server received req :'%s'\n",mode);
	tok = strtok(NULL, " ");   //deuxième partie du string
	if(mode[0]=='p'){
		printKV();
	}else if(mode[0] == 'q'){
		running = false;
		printf("server shut down\n");
	}else if(tok != NULL){
		if(strcmp(mode, "a")==0){
			puts("add via value");
			newkey = 0;
			strcpy(value,tok);
			addpair(newkey,value);
		}else if(strcmp(mode, "ak")==0){
			puts("add via key");
			if(isdigit(tok[0])){
				newkey = atoi(tok);
				printf("newkey: %d\n",newkey);
				tok = strtok(NULL, " \n");
				if(tok != NULL){
					strcpy(value,tok);
					addpair(newkey, value);
				}
			}else{
				puts("bad input");
			}
		}else if(strcmp(mode, "r")==0){
			puts("read key");
			if(isdigit(tok[0])){
				newkey = atoi(tok);
				printf("read key: %d\n",newkey);
				readpair(newkey,"");
			}else{
				puts("error on input");
			}
		}else if(strcmp(mode, "rv")==0){
			puts("read via value");
			if(isdigit(tok[0])==0){
				strcpy(value,tok);
				printf("searching key of: %s\n",value);
				readpair(-1,value);
			}else{
				puts("error on input");
			}
		}else if(strcmp(mode, "d")==0){
			puts("delete via key");
			if(isdigit(tok[0])){
				newkey = atoi(tok);
				printf("delete key: %d\n",newkey);
				deletepair(newkey, "");
			}else{
				puts("error on input");
			}
		}else if(strcmp(mode, "dv")==0){
			puts("delete via value");
			if(isdigit(tok[0])==0){
				strcpy(value,tok);
				printf("delete value: %s\n",value);
				deletepair(-1,value);
			}else{
				puts("error on input");
			}
		}else if(strcmp(mode, "m")==0){
			if(isdigit(tok[0])){
				newkey = atoi(tok);
				tok = strtok(NULL, " \n");
				if(tok != NULL){
					strcpy(value,tok);
					printf("modify key %d with value:%s\n",newkey,value);
					modifyPair(newkey, "", value);
				}
			}else{
				puts("error on input");
			}
		}else if(strcmp(mode, "mv")==0){
			if(isdigit(tok[0])==0){
				strcpy(value,tok);
				tok = strtok(NULL, " \n");
				if(tok != NULL){
					strcpy(value2,tok);
					printf("modify value of %s with value: %s\n",value,value2);
					modifyPair(-1, value, value2);
				}
			}else{
				puts("error on input");
			}
		}
		else{
            printdefault();
        }
	}
	else{
		puts("error on input");
	}
}


/*
 *
 * ----------------------------------
 * functions manipulating the KVstore
 * ----------------------------------
 *
 */

// adds a pair based on key or value
void addpair(int newkey, char* newvalue){		//write

  //=============== ENTRY SECTION ==============//
	pthread_mutex_lock(&wmutex);
	writecount++;
  if (writecount == 1){
    pthread_mutex_lock(&readTry);
  }
  pthread_mutex_unlock(&wmutex);

  //=============== CRITICAL SECTION ==============//
  pthread_mutex_lock(&resource);

	int i,j,possiblekey;
	bool check = true;
	if(newkey == 0){ // we have to give a new key to the pair
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
      block_key_add = possiblekey;    //store the key to block this entry to reader
			insertKV(possiblekey,newvalue);
			printf("New pair: value '%s' has the new generated key '%d'\n",newvalue, possiblekey);
			snprintf(rep_client,sizeof(rep_client),"New pair: value '%s' has the key '%d'",newvalue, possiblekey);
		}
	}else{
		for(i=0;i<kv->size+1;i++){//on check si une clé peut être utilisée
			if(kv[i].key==newkey){
				//key exists already, assign a new key:
				addpair(0,newvalue);
				check = false;
				break;
			}
		}
		if(check){
			insertKV(newkey, newvalue);
			printf("New pair: value '%s' has the key '%d'\n",newvalue, newkey);
			snprintf(rep_client,sizeof(rep_client),"New pair: value '%s' has the key '%d'",newvalue, newkey);
		}

	}

  //sleep(10);

  pthread_mutex_unlock(&resource);

  //=============== EXIT SECTION ==============//
	pthread_mutex_lock(&wmutex);
  writecount--;
  if (writecount == 0){
    pthread_mutex_unlock(&readTry);
  }
  pthread_mutex_unlock(&wmutex);
}


void modifyPair(int key, char* value, char* value2){		//write

  //=============== ENTRY SECTION ==============//
  pthread_mutex_lock(&wmutex);
	writecount++;
  if (writecount == 1){
    pthread_mutex_lock(&readTry);
  }
  pthread_mutex_unlock(&wmutex);

  //=============== CRITICAL SECTION ==============//
  pthread_mutex_lock(&resource);

  int i;
  size_t length1 = strlen(value);
  size_t length2 = strlen(value2);
  int counter = 0;
  if(key == -1){
    for(i=0; i<kv->size; i++){
      if(strcmp(kv[i].value, value) == 0){
        block_key_modify = i;
        printf("Modifying %s with %s \n", value, value2);
        snprintf(rep_client,sizeof(rep_client),"Modifying %s with %s", value, value2);
        memset(kv[i].value, 0, length1);
        strncpy(kv[i].value, value2,length2);
        counter++;
        break;
      }
    }
  }
  else{ // we just have the key
		for(i=0;i<kv->size;i++){
			if(kv[i].key == key){
        size_t length3 = strlen(kv[i].value);
        block_key_modify = i;
				printf("Modifying key %d having value %s \n",kv[i].key, kv[i].value);
				snprintf(rep_client,sizeof(rep_client),"Modifying key %d having value %s",kv[i].key, kv[i].value);
				memset(kv[i].value, 0, length3);
				strncpy(kv[i].value, value2,length2);
				counter++;
				break;
			}
		}
	}
  if(counter == 0){
    printf("Value not found!\n");
    snprintf(rep_client,sizeof(rep_client),"Value not found!");
  }
  pthread_mutex_unlock(&resource);

  //=============== EXIT SECTION ==============//
	pthread_mutex_lock(&wmutex);
  writecount--;
  if (writecount == 0){
    pthread_mutex_unlock(&readTry);
  }
  pthread_mutex_unlock(&wmutex);
}


void deletepair(int key, char* value){		//write

  //=============== ENTRY SECTION ==============//
  pthread_mutex_lock(&wmutex);
	writecount++;
  if (writecount == 1){
    pthread_mutex_lock(&readTry);
  }
  pthread_mutex_unlock(&wmutex);

  //=============== CRITICAL SECTION ==============//
  pthread_mutex_lock(&resource);

	int i;
	size_t length = strlen(value);
	if(key==-1){// we just have the value
		for(i=0;i<kv->size;i++){
			if(strcmp(kv[i].value,value) == 0){
        block_key_delete = i;
				printf("deleting - %s - having the key %d\n",kv[i].value, kv[i].key);
				snprintf(rep_client,sizeof(rep_client),"deleting - %s - having the key %d",kv[i].value, kv[i].key);
				kv[i].key=-1;
				memset(kv[i].value, 0, length);
				//printf("réduire taille de l'array? ");
				break;
			}
		}
	}
	else{ // we just have the key
		for(i=0;i<kv->size;i++){
			if(kv[i].key==key){ // we found the value and show the key
        block_key_delete = i;
				printf("deleting key %d having value %s \n",kv[i].key, kv[i].value);
				snprintf(rep_client,sizeof(rep_client),"deleting key %d having value %s",kv[i].key, kv[i].value);
				kv[i].key=-1;
				memset(kv[i].value, 0, length);
				//printf("réduire taille de l'array? ");
				break;
			}
		}
	}
  pthread_mutex_unlock(&resource);

  //=============== EXIT SECTION ==============//
	pthread_mutex_lock(&wmutex);
  writecount--;
  if (writecount == 0){
    pthread_mutex_unlock(&readTry);
  }
  pthread_mutex_unlock(&wmutex);
}


void readpair(int key, char* value){			//read

	int i;
	bool check = true;
	if(key==-1){// 'RV' we just have the value and want to read the key
		//printf("kv size: '%zu'\n",kv->size);
		for(i=0; i<block_key_add; i++){
			if(strcmp(kv[i].value,value)==0 && i!=block_key_modify && i!=block_key_delete){ // we found the value and show the key
				printf("value '%s' has the key '%d'\n",kv[i].value, kv[i].key);
				snprintf(rep_client,sizeof(rep_client),"value '%s' has the key '%d'",kv[i].value, kv[i].key);
				check = false;
				break;
			}
		}
	}
	else{ // 'R' we just have the key and want the value
		for(i=0;i<block_key_add;i++){
			if(kv[i].key==key && i!=block_key_modify && i!=block_key_delete){ // we found the value and show the key
				printf("the key '%d' has value '%s' \n",kv[i].key, kv[i].value);
				snprintf(rep_client,sizeof(rep_client),"the key '%d' has value '%s'",kv[i].key, kv[i].value);
				check = false;
				break;
			}
		}
	}

  //=============== ENTRY SECTION ==============//
  pthread_mutex_lock(&readTry);
	pthread_mutex_lock(&rmutex);
  readcount++;
  if (readcount == 1){
    pthread_mutex_lock(&resource);
  }
	pthread_mutex_unlock(&rmutex);
  pthread_mutex_unlock(&readTry);

  //=============== CRITICAL SECTION ==============// (reading is performed)
  if(key==-1){// 'RV' we just have the value and want to read the key
		for(i=block_key_add; i<kv->used; i++){
			if(strcmp(kv[i].value,value)==0 && i!=block_key_modify && i!=block_key_delete){ // we found the value and show the key
				printf("value '%s' has the key '%d'\n",kv[i].value, kv[i].key);
				snprintf(rep_client,sizeof(rep_client),"value '%s' has the key '%d'",kv[i].value, kv[i].key);
				check = false;
				break;
			}
		}
    for(i=0; i<kv->used; i++){    //when the value is modifier during the search
			if(i==block_key_modify){ // we found the value and show the key
				printf("value '%s' has the key '%d'\n",kv[i].value, kv[i].key);
				snprintf(rep_client,sizeof(rep_client),"value '%s' has the key '%d'",kv[i].value, kv[i].key);
				check = false;
				break;
			}
		}
		if(check){
			printf("no pair found\n");
			snprintf(rep_client,sizeof(rep_client),"no pair found");
		}
	}
	else{ // 'R' we just have the key and want the value
		for(i=block_key_add;i<kv->used;i++){
			if(kv[i].key==key && i!=block_key_modify && i!=block_key_delete){ // we found the value and show the key
				printf("the key '%d' has value '%s' \n",kv[i].key, kv[i].value);
				snprintf(rep_client,sizeof(rep_client),"the key '%d' has value '%s'",kv[i].key, kv[i].value);
				check = false;
				break;
			}
		}
    for(i=0;i<kv->used;i++){    //when the key is modifier during the search
			if(i==block_key_modify){ // we found the value and show the key
				printf("the key '%d' has value '%s' \n",kv[i].key, kv[i].value);
				snprintf(rep_client,sizeof(rep_client),"the key '%d' has value '%s'",kv[i].key, kv[i].value);
				check = false;
				break;
			}
		}
		if(check){
			printf("no pair found\n");
			snprintf(rep_client,sizeof(rep_client),"no pair found");
		}
	}

  block_key_modify = -1;
  block_key_delete = -1;

  //=============== EXIT SECTION ==============//
  pthread_mutex_lock(&rmutex);
  readcount--;
  if (readcount == 0){
    pthread_mutex_unlock(&resource);
  }
  pthread_mutex_unlock(&rmutex);
}


void printKV(){				//read

    int i;
    bool check = true;
    snprintf(rep_client,sizeof(rep_client),"printing KV");
    for(i=0;i<block_key_add;i++){
  		if(kv[i].key!=-1 && i!=block_key_modify && i!=block_key_delete){
  			printf("kv[%d].value is: %s and key is: %d\n",i,kv[i].value,kv[i].key);
        snprintf(rep_client+strlen(rep_client),sizeof(rep_client)-strlen(rep_client),"\nkv[%d].value is: %s and key is: %d",i,kv[i].value,kv[i].key);
      }
      else{
        check = false;
      }
    }

    //=============== ENTRY SECTION ==============//
    pthread_mutex_lock(&readTry);
    pthread_mutex_lock(&rmutex);
    readcount++;
    if (readcount == 1){
      pthread_mutex_lock(&resource);
    }
    pthread_mutex_unlock(&rmutex);
    pthread_mutex_unlock(&readTry);

    //=============== CRITICAL SECTION ==============// (reading is performed)
    for(i=0;i<kv->used;i++){
      if(i==block_key_modify){
            printf("kv[%d].value is: %s and key is: %d\n",i,kv[i].value,kv[i].key);
            snprintf(rep_client+strlen(rep_client),sizeof(rep_client)-strlen(rep_client),"\nkv[%d].value is: %s and key is: %d",i,kv[i].value,kv[i].key);
      }
      else{
        check = false;
      }
    }

    for(i=block_key_add;i<kv->used;i++){
  		if(kv[i].key!=-1 && i!=block_key_modify && i!=block_key_delete){
  			printf("kv[%d].value is: %s and key is: %d\n",i,kv[i].value,kv[i].key);
        snprintf(rep_client+strlen(rep_client),sizeof(rep_client)-strlen(rep_client),"\nkv[%d].value is: %s and key is: %d",i,kv[i].value,kv[i].key);
      }
      else{
        check = false;
      }
    }

    if(check==true){
      printf("there are no values in the Key-Value Store\n");
      snprintf(rep_client+strlen(rep_client),sizeof(rep_client)-strlen(rep_client),"\nthere are no values in the Key-Value Store");
    }





    block_key_modify = -1;
    block_key_delete = -1;

    //sleep(10);

    pthread_mutex_lock(&rmutex);
    readcount--;
    if (readcount == 0){
      pthread_mutex_unlock(&resource);
    }
    pthread_mutex_unlock(&rmutex);
}

void printdefault(){ //annoying to write each time the printf
    puts("commands ('cmd [args] effect'): h (help), a/ak [-/k] (add k/v), -d/-dv [-/v] (delete k/v)");
}
