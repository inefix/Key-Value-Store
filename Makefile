# ------------------------------------------------------------------------------
# Makefile for key-value store
#
# ------------------------------------------------------------------------------

all :   server client

clean :
	rm -f  *.o client server
	


#------------------------------------------
# Both programms
#------------------------------------------

server : server.o dynamicArray.o
	gcc -pthread -Wall server.o dynamicArray.o -o server 

client : client.o
	gcc -pthread -Wall client.o -o client

#----------------------------
# client
#----------------------------
client.o :  client.c 
	gcc -pthread -Wall -c client.c


#-----------------------------------------------
# Server
#-----------------------------------------------
server.o : server.c server.h
	gcc -pthread -Wall -c server.c

dynamicArray.o : dynamicArray.c dynamicArray.h
	gcc -pthread -Wall -c dynamicArray.c
