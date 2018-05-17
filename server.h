// header file for the KV store

 

#define PORT 7777
#define MSGSIZE 1024
#define STORESIZE 1000 // modify later for dynamic size
#define VALUESIZE 50

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
void readpair(int key, char* value);
void deletepair(int key, char* value);
void modifyPair(int key, char* value1, char* value2);
void printKV();

void* multiconnect(void* socketdesc);
void* readcmd(void*);
void processcmd(char* input);
int ctrlregex(char* msg);
void printdefault();
const char* delnewline(char* str);
