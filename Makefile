# ------------------------------------------------------------------------------
# Makefile for key-value store
#
# ------------------------------------------------------------------------------

all :   server client

clean :
	rm -f  *.o client
	rm -f  *.o server


#------------------------------------------
#
#------------------------------------------

server :    server.o
	gcc -pthread -Wall server.o -o server

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
server.o : server.c
	gcc -pthread -Wall -c server.c dynamicArray.c
