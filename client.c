#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
//fcntl (0,F_SETFL, fcntl(0,F_GETGL)|O_NONBLOCK);
//Todo : Registration, Encryption, Add friend, connect to the server
#define ERR_EXIT(a) { perror(a); exit(1); }

int main(int argc, char** argv){

	// Login or registration successfull
	//client to server connection
	char port[10];
	char host[40];
	char dest_username[20];

	//parse argument
	strcpy(host,argv[1]);
	strcpy(port,argv[2]);
	// strcpy(dest_username,argv[3]);

	
	//initialize the server address
	struct sockaddr_in server_add;
	int temp_port = atoi(port);
	bzero(&server_add,sizeof(server_add));
	server_add.sin_family = AF_INET;
	server_add.sin_addr.s_addr = htonl(INADDR_ANY);
	server_add.sin_port = htons(temp_port);

	//get the address of the host
	struct addrinfo hints, *res;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	int address  = getaddrinfo(host,port,&hints,&res);
	if(address!=0){
		fprintf(stderr,"get address error\n");
	}
	//build socket
	printf("building socket\n");
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd <0){
		fprintf(stderr,"error opening socket\n");
	}

	//connect to the server
	printf("connecting to the server\n");
	int connectfd = connect(sockfd,res->ai_addr,res->ai_addrlen);
	if(connectfd<0){
		fprintf(stderr,"connect error\n");
	}
	fprintf(stderr,"connect to server success\n");

	int sock_ret = fcntl(sockfd,F_SETFL, fcntl(sockfd,F_GETFL)| ~O_NONBLOCK);
	if(sock_ret<0) fprintf(stderr,"sock_ret block error");

	//parse the variable
	char message_sent[1024];
	char message_received[1024];
	char server_IP[30];

	//Registration used variable
	char username[20];
	char password[20];
	char confirm_pass[20];
	char reply[10] = "";

	//Ask for option 
	int correct = 0;
	int choice;
	while(!correct){
		printf("1. Register (If you are first timer)\n");
		printf("2. Login\n");
		printf("3. Quit\n");
		printf("Please Enter Your Choice :");
		scanf("%d",&choice);
		if(choice ==1 || choice==2 || choice==3){
			correct=1;
		}
		else printf("Error!\n");
	}
	// If correct choice is choosen 
	if(choice ==3){
		char choicex = 'c';
		send(sockfd, &choicex, 1, 0);
		return 0;
	}
	else if(choice == 1){
		//Registration
		correct =2; 
		char choicex = 'a';
		send(sockfd, &choicex, 1, 0);
		while(correct ==2){

			printf("Enter Username:");
			scanf("%s",username);
			printf("Enter password:");
			scanf("%s",password);
			printf("Confirm password:");
			scanf("%s",confirm_pass);
			if(strcmp(confirm_pass,password) == 0){

				send(sockfd, username, 20, 0);
				send(sockfd, password, 20, 0);

				//receive the reply correct
				char reply[2];
				int recv_reply = 0;
				while((recv_reply = recv(sockfd, reply, 2, 0)) < 0);
				if(recv_reply < 0) fprintf(stderr, "error in receiving the reply\n");

				if(strcmp(reply, "2") == 0) {
					printf("Error! Invalid username or password\n");
					correct = 2;

				}
				else {
					printf("Login successfull\n");
					correct = 1;
					break;
				}

			}
			else {
				printf("password and confirm password do not match\n");
			}
		}
	}
	else if(choice==2){
		char choicex = 'b';
		// fprintf(stderr, "CHOICEX = %c\n", choicex);
		send(sockfd, &choicex, 1, 0);

		correct=0;
		while(!correct){	

			printf("Enter Username:");
			scanf("%s",username);
			send(sockfd, username, 20, 0);
			printf("Enter password:");
			scanf("%s",password);
			send(sockfd, password, 20, 0);			

			//receive the success message
			char reply[2];
			int recv_reply = 0;

			while((recv_reply = recv(sockfd, reply, 2, 0)) < 0);

			if(recv_reply < 0) fprintf(stderr, "error in receiving the reply\n");
			
			if(strcmp(reply, "1") == 0) {
				printf("Login successfull\n");
				correct = 1;
				break;

			} else if(strcmp(reply, "0") == 0) {
				printf("Error! Invalid username or password\n");
				correct = 0;

			}
		} 

	}


	//send  the destination username
	int dest_correct=0;
	while(correct &&  !dest_correct){
		fprintf(stderr,"Enter dest_username:");
		scanf("%s",dest_username);
		write(sockfd,dest_username,20);

		char reply[2];
		int reply_n;
		while((reply_n = read(sockfd,reply,2)) <0);
		if(strcmp(reply,"1")==0){
			fprintf(stderr,"dest_username correct\n");
			dest_correct=1;
		}
		else {
			fprintf(stderr,"Error, No such username\n");
			dest_correct=0;
		}
	}
	// send(sockfd, dest_username, 20, 0);


	/*======================READ LOG.TXT==============================*/
	char l_username[40];
	while(1){
		int l_username_n = read(sockfd,l_username,40);
		if(l_username_n>0){
			if(strcmp(l_username,"/end")==0){
				break;
			}
			//read destination
			char l_dest[40];
			int l_dest_n;
			while((l_dest_n = read(sockfd,l_dest,40))<0);
			// read size 
			char size_c[10];
			int l_size_n;
			while((l_size_n = read(sockfd,size_c,10))<0);
			int l_size=0;
			int x=0;
            // ganti size dr char ke int
            while(isdigit(size_c[x])){
                l_size*=10;
                l_size+=(size_c[x]-'0');
                x++;
            }

            //baca message 
            char l_message [l_size];
            int l_message_n;
            while((l_message_n= read(sockfd,l_message,l_size+2))<0);

            fprintf(stderr,"from %s to %s : %s\n",l_username,l_dest,l_message);
		}
	}

	/* ================== MESSAGES / FILES TRANSFER ================ */
	char greeting[40];
	while(1){
		int greeting_n = read(sockfd,greeting,40);
		if(greeting_n>0){
			fprintf(stderr,"greeting from %s : %s",dest_username,greeting);
			break;
		}
	}

	// int greeting_n = read(sockfd,greeting,200);
	// if(greeting_n >0) fprintf(stderr,"greeting from %s : %s\n",dest_username,greeting);

	char w_message[2048]="test";
	// fprintf(stderr,"Please enter your message:");
	while(1){
		sock_ret = fcntl(sockfd,F_SETFL, fcntl(sockfd,F_GETFL)|O_NONBLOCK);
		if(sock_ret<0) fprintf(stderr,"sockret error\n");

		//read file type from server
		char r_type = '\0';
		
		int n_type = read(sockfd,&r_type,1);
		// fprintf(stderr,"rtype= %c\n",r_type);
			// fprintf(stderr,"hello?\n");
		if(n_type>0){
			// fprintf(stderr, "R_TYPE STARTING = %c\n", r_type);
			if(r_type == 'y'){ // isfile
				fcntl(sockfd,F_SETFL, fcntl(sockfd,F_GETFL)|~O_NONBLOCK);

				// fprintf(stderr, "R_TYPE = %c\n", r_type);

				//read num of file
				char r_filenum_temp[10];

				int n_file = read(sockfd,r_filenum_temp,10);
				r_filenum_temp[strlen(r_filenum_temp)] = '\0';

				if(n_file<0) fprintf(stderr,"read filenum from server error\n");
				// fprintf(stderr,"r_filenum_temp in client = %s.\n",r_filenum_temp);
				// int r_filenum = atoi(&r_filenum_temp);
				int r_filenum = 0;
				int i = 0;
				while(isdigit(r_filenum_temp[i])) {
					r_filenum *= 10;
					r_filenum += (r_filenum_temp[i]-'0');
					i++;
				}

				// fprintf(stderr, "r_file_num in client = %d\n", r_filenum);

				for(int i=0;i<r_filenum;i++){
					char r_filename[20];
					//read filename
					n_file = read(sockfd,r_filename,20); 
					r_filename[n_file] = '\0';
					// if(n_file<0) fprintf(stderr,"read filename from socket error\n");
					// fprintf(stderr,"HI filename = %s\n",r_filename);
					char real_filename[n_file];
					strcpy(real_filename, r_filename);
					// fprintf(stderr, "REAL_FILENAME = %s.\n", real_filename);
					//read size of the file 
					char r_size_tempx[10];
					int read_size = read(sockfd,r_size_tempx,10);
					r_size_tempx[read_size] = '\0';
					// fprintf(stderr, "R_SIZE_TEMPX = %s\n", r_size_tempx);
					// int r_size = atoi(r_size_temp);
					int r_size = 0;
					int i = 0;
					while(isdigit(r_size_tempx[i])) {
						r_size *= 10;
						r_size += (r_size_tempx[i]-'0');
						i++;
					}
					// fprintf(stderr, "R_SIZE_NUM = %d\n", r_size);

					//change the name of the filename
					char r_filename1[40]="";
					char r_filename_start[40];
					char r_filename_end[5];
					int before=1;
					// int after_idx=0;
					fprintf(stderr,"filename = %s\n",real_filename);
					for(int i=0;i<strlen(real_filename);i++){
						if(before){
							if(real_filename[i]=='.'){
								before=0;
								r_filename1[i]='1';
								r_filename1[i+1] = real_filename[i];
							}
							else{
								r_filename1[i] = real_filename[i];
							}
						}
						else{
							fprintf(stderr,"hello\n");
							r_filename1[i+1] = real_filename[i];
						}
					}
					if(before ==1){

						r_filename1[strlen(r_filename1)]='1';
						r_filename1[strlen(r_filename1)+1]='\0';

					}
					fprintf(stderr,"new filename = %s\n",r_filename1);
					// fprintf(stderr,"r_filename1 = %s\n",r_filename1);

					//open a file
					// fprintf(stderr, "R_FILENAME = %s.\n", r_filename);
					FILE *rfile = fopen(r_filename1,"wb");
					char r_temp[r_size];

					//read from server until eof 
					int n_read = read(sockfd,r_temp,r_size);
					if(n_read < 0) fprintf(stderr, "error reading the file buffer.\n");
					r_temp[r_size]='\0';
					fwrite(r_temp, 1, r_size, rfile);
					fclose(rfile);
					memset(r_temp, '\0', sizeof(r_temp));
					memset(r_size_tempx, '\0', sizeof(r_size_tempx));
					memset(r_filename, '\0', sizeof(r_filename));
					memset(real_filename, '\0', sizeof(real_filename));
				}

			}
			else if(r_type =='n'){ //message 
				//======================RECEIVE MESSAGE========================================
				// fprintf(stderr,"GOOD UNTIL HERE\n");
				fcntl(sockfd,F_SETFL, fcntl(sockfd,F_GETFL)|~O_NONBLOCK);
				char r_size_temp[10];
				int r_size_n = read(sockfd,r_size_temp,10);
				// fprintf(stderr, "R_SIZE_TEMP = %s\n", r_size_temp);
				r_size_temp[r_size_n] = '\0';
				// int r_size = atoi(r_size_temp);
				int r_size = 0;
				int i = 0;
				while(isdigit(r_size_temp[i])) {
					r_size *= 10;
					r_size += (r_size_temp[i]-'0');
					i++;
				}

				// fprintf(stderr,"r_size =%d\n",r_size);
				char r_message[r_size];
				//read from server
				memset(r_message,'\0',sizeof(r_message));
				int n = read(sockfd,r_message,r_size);
				r_message[n]='\0';
				if(n>0){
					fprintf(stderr,"received from %s : %s\n",dest_username,r_message);
				}
			}
		}


		//set nonblock
		int ret =fcntl(0,F_SETFL, fcntl(0,F_GETFL)|O_NONBLOCK);
		if(ret<0) printf("fcntl error\n");

		//read input from users
		char isfile;
		int read_input = read(0,w_message,2047);
		w_message[read_input]='\0';
		if(strcmp(w_message,"/quit\n")==0) {
			// fprintf(stderr,"its breaking\n");
			break;
		}
		
		if(read_input>0 && strcmp(w_message,"/quit\n")!=0){
			// fprintf(stderr,"w_message=%s.\n",w_message);
			// fprintf(stderr,"confirm message : %s\n",w_message);
			//parse the file
			char flag[4];
			flag[0] = w_message[0];
			flag[1] = w_message[1];
			w_message[strlen(w_message)-1]='\0';
			char w_filename[20][20]; //array for all filename to send  
			//printf("hello\n");
			
			// strcpy(flag,tok);
			if(strcmp(flag,"-f")==0) { //isfile
				// =======================FILE TRANSFER======================================================
				fcntl(sockfd,F_SETFL, fcntl(sockfd,F_GETFL)|~O_NONBLOCK);
				char *tok;
				tok = strtok(w_message," ");
				isfile = 'y';
				int w_n = write(sockfd,&isfile,1);
				if(w_n<0) fprintf(stderr,"write type error in filetransfer\n");

				//send filenumber
				tok = strtok(NULL," ");
				char filenum_temp[10];
				strcpy(filenum_temp, tok);
				// filenum_temp = *tok;
				// filenum_temp[strlen(filenum_temp)]='\0';
				// fprintf(stderr,"FILENUM SENT = %s.\n",filenum_temp);
				w_n = write(sockfd,filenum_temp,10);
				if(w_n<0) fprintf(stderr,"write filenumber in filetransfer error\n");
				//send the file one by one 
				// int filenum = atoi(&filenum_temp);

				int filenum = 0;
				int i = 0;
				while(isdigit(filenum_temp[i])) {
					filenum *= 10;
					filenum += (filenum_temp[i]-'0');
					i++;
				}

				for(int i=0;i<filenum;i++){

					// fprintf(stderr, "I = %d\n", i);
					tok = strtok(NULL," ");
					memset(w_filename[i],'\0',sizeof(w_filename[i]));
					strcpy(w_filename[i],tok);
					w_filename[i][strlen(w_filename[i])]='\0';


					//send filename first
					// fprintf(stderr, "W_FILENAME = %s\n", w_filename[i]);
					w_n = write(sockfd,w_filename[i],20);
					if(w_n<0) fprintf(stderr,"write filename in filetransfer error\n");
					// fprintf(stderr,"filename = %s\n",w_filename[i]);

					// fprintf(stderr,"GOOD UNTIL HERE\n");
					//read all the file
					
					FILE *ftemp = fopen(w_filename[i],"rb");

					char size_temp[10];
					
					fseek(ftemp,0,SEEK_END);
					int size = ftell(ftemp);
					// fprintf(stderr, "SIZE IN WRITE = %d\n", size);
					fseek(ftemp,0,SEEK_SET);
					
					sprintf(size_temp,"%d",size);
					size_temp[strlen(size_temp)] = '\0';

					//write the size into server 
					// fprintf(stderr, "SIZE_TEMP IN WRITE = %s\n", size_temp);
					int w_size = write(sockfd,size_temp,10);
					if(w_size<0) fprintf(stderr,"w_size error\n");
					
					char file_temp[size];
					fread(file_temp,1,size,ftemp);
					file_temp[size]='\0';
					
					w_n = write(sockfd,file_temp,size);
					if(w_n<0) fprintf(stderr,"write file_temp error\n");

					fclose(ftemp);
				}
			}
			else{
				//===============================MESSAGING====================================================
				fcntl(sockfd,F_SETFL, fcntl(sockfd,F_GETFL)|~O_NONBLOCK);
				fprintf(stderr,"sending message\n");
				isfile = 'n';
				int w_n = write(sockfd,&isfile,1);
				if(w_n<0) fprintf(stderr,"write type error in messaging \n");

				//send message
				w_n = write (sockfd,w_message,strlen(w_message));
				if(w_n<0) fprintf(stderr,"write message error\n");
				
			}
		}
	}
}