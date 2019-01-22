ALL :
	gcc server.c -o server -lpthread
	gcc client.c -o client

CLEAN :
	rm -f server client
