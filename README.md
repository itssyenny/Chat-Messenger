# CN project2

Messaging app with client and server in C language

Input format :

$ gcc server.c -pthread

$ ./server listen_port

$ ./client username host_port

The Additional Files we need :

- database.txt
	
	FORMAT :
	username:password

- log.txt
	
	FORMAT :
	sender receiver message

- messagefile.txt
	
	FORMAT :
	sender receiver message

# Server 

How it works ?

- In the main program works as follows :
	1. do the socket, bind, and listen
	2. in the while(1) loop, server accept every client's connfd
	3. create a pthread for every new client's process

- In the function rcv_message works as follows :
	1. Server asks the client's identity and to whom he/she wants to talk to
		- username
		- destination client's username
	2. check from the "messagefile.txt" if there is a message that is yet to send to this current user
	3. After finding that unsent message, update the "log.txt" and close those files.
	4. After that, store the current client's info in struct client_info and add him/her to the ONLINE clients' array.
	5. check if there is received message to sent to dest_client
	6. check whether the dest_client is online by looking at the ONLINE clients' array.
	7. If exists, then send directly to the dest_client and update the "log.txt"
	8. If not, then check from the "database.txt" if there is a user with this username.
		- If exists, then update the "messagefile.txt"
		- Else, send error message to this current client.
	9. After finish all the file transfer process, delete this current client from ONLINE clients' list.


# Client

How it works ?
	1. As for now, to run the client type 
		./client hostname port dest_username
	2. directly type for sending message. If you are sending file, type -f #of_file list of filename

# Registration part

- Login 
	1. send the username directly to the server
	2. send the destination client's username to the server
	3. check the "log.txt" for chatting history and print it all in order between sender and receiver
	4. After receiving the greeting message, you can start chatting

-Add Friend (Optional)
	1. After Login, We show the list of friends of the user 
	2. If it is a new user, then show 

-Connect to the server
	1. Build socket
	2. Connect to the server


