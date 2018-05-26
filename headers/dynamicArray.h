#ifndef _dynA_H_
#define _dynA_H_

//function to dynamically allocate memory to KVstore
void initKVstore(size_t initialSize);
void insertKV(int newkey, char *newvalue);
void freeKVstore();

#endif
