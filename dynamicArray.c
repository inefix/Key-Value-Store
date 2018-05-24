
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "server.h"
#include "dynamicArray.h"

extern KVstore *kv; // our main KV store array
extern char rep_client[MSGSIZE];
extern bool running;// tenté d'avoir une var globale pour arrêter le serveur

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
		kv[i].value = (char*) malloc(sizeof(char*)*VALUESIZE);
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
			kv[i].value = (char*) malloc(sizeof(char*)*VALUESIZE);
			kv[i].key = -1;
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

		for(int j = 0; j<=kv->used; j++){
			if(kv[j].key == -1 || kv[j].value == NULL ){
				size_t length1 = strlen(kv[j].value);
				kv->used++;
				memset(kv[j].value, 0, length1);
				strncpy(kv[j].value, newvalue,length); // on insère la valeur
				kv[j].key = newkey; // on indique aussi la clé
				printf("kv[%d].value is: %s and key is: %d\n",j,kv[j].value,kv[j].key);
				break;
			}

			if(j == kv->used){
				kv->used++;
				strncpy(kv[j].value, newvalue,length); // on insère la valeur
				kv[j].key = newkey; // on indique aussi la clé
			}
		}

		/*int index = kv->used; // on ajoute un element
		kv->used++;
		strncpy(kv[index].value, newvalue,length); // on insère la valeur
		kv[index].key = newkey; // on indique aussi la clé*/
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
