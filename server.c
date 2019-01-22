#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <pthread.h>
#include <ctype.h>
#define ERR_EXIT(a) { perror(a); exit(1); }

struct client_info {
    int socketno;
    int first;
    char username[20];
    char dest[20];
};
struct ONLINE_clients {
    int sockno;
    char username[20];
};

int n = 0;
struct ONLINE_clients ONLINE[1024];
void *rcv_msg(void *socket);

int main(int argc, char const *argv[]) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serveraddr, clientaddr;
    bzero(&serveraddr, sizeof(serveraddr));

    unsigned short port = (unsigned short) atoi(argv[1]);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(port);

    if(bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) 
        fprintf(stderr, "error on binding\n");

    if(listen(sockfd, 5) < 0) 
        fprintf(stderr, "error on listening\n");

    //preprocessing
    int connfd;
    pthread_t thread;
    char otherclient[20], username[20];

    while(1) {
        int clientlen = sizeof(clientaddr);
        if((connfd = accept(sockfd, (struct sockaddr*)&clientaddr, (socklen_t*)&clientlen)) < 0) {
            perror("Accept error.");
            exit(1);
        }

        //process starting here ...
        //create a pthread
        if(pthread_create(&thread, NULL, rcv_msg, &connfd) < 0) {
            perror("Creating a pthread error.");
            exit(1);
        }

        // sleep(1);
    }

    close(sockfd);
    close(connfd);
    return 0;
}

/* helper function */

int check_username(char username[], char password[],int islogin){
    FILE *fdatabase = fopen("database.txt","r");
    //islogin == if it is login else 0 
    char temp[50];
    char temp_username[20];
    char temp_password[20];
    int find = 0;
    while(fscanf(fdatabase,"%s",temp)!=EOF && !find){
        char *pch;
        pch = strtok(temp,":");
        strcpy(temp_username,pch);
        pch = strtok(NULL,":");
        strcpy(temp_password,pch);
        if(strcmp(username,temp_username) == 0 && strcmp(password,temp_password)==0 && islogin){
            find=1; //valid username and password
        }
        if(!islogin && strcmp(username,temp_username)==0){
            find =2;
        }
    }   
    fclose(fdatabase);
    return find;
}

void *rcv_msg(void *socket) {
    int connfd = *(int *)socket;

    int x;
    char username_destination[40];
    char username[20]="", destination[20]="";

    char choice = '\0';
    int validUser = 0;
    int correct = 0;
  
    recv(connfd, &choice, 1, 0);
    
    char password[20] = "";

    if(choice == 'c') {
        while(choice == 'c') {
            recv(connfd, &choice, 1, 0);
        } 
    }
    
    if(choice == 'b') { //Login
        correct = 0;
        while(!correct) {
            recv(connfd, username, 20, 0);
            recv(connfd, password, 20, 0);
            fprintf(stderr, "IN LOGIN, USERNAME  = %s\n", username);
            fprintf(stderr, "IN LOGIN, PASSWORD  = %s\n", password);

            char correctx[2];
            correct = check_username(username,password,1);
            
            sprintf(correctx,"%d",correct);

            int send_cor = send(connfd, correctx, 2, 0);
            if(send_cor < 0) fprintf(stderr, "error in sending correctx.\n");

            if(correct == 0){ //false username or password
                fprintf(stderr,"Error! Invalid username or password\n");
                validUser = 0;

            }
            else if(correct == 1){ // valid username and password
                fprintf(stderr, "Login successfull\n");
                validUser = 1;
            }
        }
    }
    else if(choice == 'a') {    //Register

        correct = 2;
        while(correct == 2) {
            recv(connfd, username, 20, 0);
            recv(connfd, password, 20, 0);
            fprintf(stderr, "IN REGISTER, USERNAME  = %s\n", username);
            fprintf(stderr, "IN REGISTER, PASSWORD  = %s\n", password);

            char correctx[2];
            correct = check_username(username,password,0); // check is there is the same username used before
   
            sprintf(correctx,"%d",correct);
    
            //send the correct to client            
            int send_cor = send(connfd, correctx, 2, 0);
            if(send_cor < 0) fprintf(stderr, "error in sending correctx.\n");

            if(correct == 2){
                fprintf(stderr, "Error! username have been used\n");
                validUser = 0;
            }
            else{
                fprintf(stderr, "Registration successfull\n");
                // if registration succesfull, then write to the file 
                FILE *fdata_append = fopen("database.txt","a");
                fprintf(fdata_append,"%s:%s",username,password);
                fprintf(fdata_append,"\n");
                fclose(fdata_append);
                validUser = 1;
            }
        }
    }


    //check if there is the right dest username
    int validDest = 0;
    while(!validDest){
        recv(connfd, destination, 20, 0);
        char reply[2];
        int dest_correct = check_username(destination,"////",0);
        if(dest_correct!=2){
            fprintf(stderr,"incorrect destination\n");
            validDest = 0;
            
        } 
        else {
            fprintf(stderr, "correct destination.\n");
            validDest=1;
        }
        sprintf(reply,"%d",validDest);
        int dest_correct_n = write(connfd,reply,2);
    }

    // fprintf(stderr, "validUser = %d, validDest = %d \n", validUser, validDest);

    if(validUser == 1 && validDest == 1) {
        

        /* ================================= */
        fprintf(stderr, "Client %s is connected\n", username);
        fprintf(stderr, "Client destination is %s\n", destination);
        /* ================================= */
        
        /*====================================SEND LOG.TXT=====================================*/
        FILE *log = fopen("log.txt","rb");
        char l_username[40]="";
        char l_dest[40]="";

        while(fscanf(log,"%s %s ",l_username,l_dest)>0){
            l_username[strlen(l_username)]='\0';
            l_dest[strlen(l_dest)]='\0';
            char size_c[10];
            int size=0;
            fscanf(log,"%s",size_c);
            int x=0;
            // ganti size dr char ke int
            while(isdigit(size_c[x])){
                size*=10;
                size+=(size_c[x]-'0');
                x++;
            }
            // fprintf(stderr,"size char=%s int =%d\n",size_c,size);
            char l_message[size+2];
            fread(l_message,size+1,1,log);
            l_message[size+1]='\0';
            // fprintf(stderr,"log message =%s\n",l_message);
            if((strcmp(username,l_username)==0 && strcmp(destination,l_dest)==0)|| (strcmp(username,l_dest)==0 && strcmp(destination,l_username)==0)){
                // fprintf(stderr,"enter\n");
                //send from-name -> to-name -> size of meesage -> message 
                int l_name_n = write(connfd,l_username,40);
                if(l_name_n<0) fprintf(stderr,"write l_username error\n");

                int l_dest_n = write(connfd,l_dest,40);
                if(l_dest_n<0) fprintf(stderr,"write l_dest error\n");

                int l_size_n = write(connfd,size_c,10);
                if(l_size_n<0) fprintf(stderr,"write size_c error\n");

                int l_message_n = write(connfd,l_message,size+2);
                if(l_message_n<0) fprintf(stderr,"write l_message error\n");

            }
            memset(l_username, '\0', sizeof(l_username));
            memset(l_dest, '\0', sizeof(l_dest));
            memset(size_c, '\0', sizeof(size_c));                
            memset(l_message, '\0', sizeof(l_message));
        }
        //write a message to tell the client that this is the end 
        char end[40] = "/end";
        int l_end_n = write(connfd,end,40);
        if(l_end_n<0) fprintf(stderr,"write end error\n");

        fclose(log);
        /*==================================SEND GREETING=========================================*/
        char greeting[40] = "You can start chatting now.\n";
        // fprintf(stderr,"greeting server = %s",greeting);
        send(connfd, greeting, strlen(greeting), 0);

        /* =============================== CHECK UNSENT MESSAGES ========================== */
        // if(validUsername == 1 && validDest == 1) {}
        //check if there is a message to send to the user
        char filetype = '\0';
        char haha[2048] = "";
        char name[20] = "", unsent_msg[2048] = "", source[20] = "";

        // FILE *list = fopen("send.txt", "r");
        // FILE *logfile = fopen("log.txt", "a");
        FILE *messageTosent = fopen("messagefile.txt", "r+");
        // fprintf(stderr,"GOOD UNTILL HERE\n");
        while(fscanf(messageTosent,"%s %s\n",source,name)>0) {
            // fprintf(stderr,"source = %s name = %s\n",source,name);

            char status[3];
            fscanf(messageTosent, "%s", status);
            // fprintf(stderr, "HEY status = %s.\n", status);

            if(strcmp(status, "S0") == 0) {
                // fprintf(stderr, "MASUK STATUS S0\n");

                // fprintf(stderr, "USERNAME %s , NAME = %s\n", username, name);
                // fprintf(stderr, "strcmp  = %d\n", strcmp(username,name));
                if(strcmp(username,name)==0 && strcmp(source, destination)==0) {

                    fseek(messageTosent, -1, SEEK_CUR);

                    // memset(status, '\0', sizeof(status));
                    strcpy(status, "1");
                    fprintf(messageTosent, "%s", status);
                    fseek(messageTosent, 1, SEEK_CUR);

                    fscanf(messageTosent,"%c\n",&filetype);
                    int filetype_n = write(connfd,&filetype,1);
                    if(filetype_n<0) fprintf(stderr,"write filetype unsent message error\n");
                    

                    // fprintf(stderr,"IN STATUS S0, filetype =%c\n",filetype);

                    if(filetype == 'y'){
                    //scan and send file num
                        fprintf(stderr, "UNSENT MESSAGES HAVE BEEN SENT TO THE ONLINE USER.\n");
                        char filenum_tmp[10];
                        fscanf(messageTosent,"%s\n",filenum_tmp);
                        filenum_tmp[strlen(filenum_tmp)] = '\0';
                        // fprintf(stderr, "OFFLINE, FILENUM_TMP = %s\n",filenum_tmp);
                        int filenum_n = write(connfd,filenum_tmp,10);
                        if(filenum_n<0) fprintf(stderr,"write filenum unsent message error\n");

                        int filenum = 0;
                        int i = 0;
                        while(isdigit(filenum_tmp[i])) {
                            filenum *= 10;
                            filenum += (filenum_tmp[i]-'0');
                            i++;
                        }

                        for(int i=0;i<filenum;i++){
                            //scanf and write filename to the client
                            char filename[40];
                            fscanf(messageTosent,"%s\n",filename);

                            // fprintf(stderr, "OFFLINE, FILENAME = %s\n", filename);
                            int filename_n = write(connfd,filename,20);
                            if(filename_n<0) fprintf(stderr,"write filename unsent message error\n");

                            //scanf and write the file size to the client
                            char sizex[10];
                            fscanf(messageTosent,"%s\n",sizex);
                            int size_n = write(connfd,sizex,10);
                            if(size_n<0) fprintf(stderr,"write size unsent message error\n");

                            //scan and write the file to the client

                            int size = 0;
                            int i = 0;
                            while(isdigit(sizex[i])) {
                                size *= 10;
                                size += (sizex[i]-'0');
                                i++;
                            }

                            char unsent_buffer[size];
                            fread(unsent_buffer,1,size,messageTosent);

                            // fprintf(stderr, "OFFLINE, UNSENT_BUFFER = %s\n", unsent_buffer);
                            int unsent_n = write(connfd,unsent_buffer,size);
                            if(unsent_n<0) fprintf(stderr,"write unsent file error\n");

                            //print the history to the log.txt
                            // FILE *logfile = fopen("log.txt", "a");
                            // fprintf(logfile,"%s %s %s\n",source,name,filename);
                            // fclose(logfile);
                            memset(sizex, '\0', sizeof(sizex));
                            memset(filename,'\0',sizeof(filename));
                            memset(unsent_buffer,'\0',sizeof(unsent_buffer));

                        }
                        memset(filenum_tmp, '\0', sizeof(filenum_tmp));
                        

                    }
                    else if(filetype =='n'){
                        //scan message and write it to the client
                        // fprintf(stderr, "MASUK SINI filetype = n\n");
                        // char u_message[200];
                        char u_size[10];
                        fscanf(messageTosent, "%s\n", u_size);
                        u_size[strlen(u_size)] = '\0';
                        // fprintf(stderr, "u_size here =  %s\n", u_size);
                        int u_size_num = 0;
                        int i = 0;
                        while(isdigit(u_size[i])) {
                            u_size_num *= 10;
                            u_size_num += (u_size[i]-'0');
                            i++;
                        }
                        // fprintf(stderr, "U_SIZE_NUM = %d\n", u_size_num);

                        int u_size_n = write(connfd,u_size,10);
                        if(u_size_n<0) fprintf(stderr, "u_size error");
                        char u_message[u_size_num];
                        fread(u_message,1,u_size_num,messageTosent);
                        u_message[u_size_num]='\0';
                        // fprintf(stderr, "message in filetype = n, %s\n", u_message);
                        int u_message_n = write(connfd,u_message,u_size_num);
                        if(u_message<0) fprintf(stderr,"write u message error\n");

                        FILE *logfile = fopen("log.txt", "a");
                        // fprintf(logfile,"%s %s %lu %s\n",source,name,strlen(u_message),u_message);
                        fclose(logfile);
                        memset(u_message,'\0',sizeof(u_message));
                        memset(u_size,'\0',sizeof(u_size));
                    }

                }
                else {

                    fscanf(messageTosent,"\n%c\n",&filetype);
                    // fprintf(stderr, "In else, filetype = %c\n", filetype );
                     if(filetype == 'y'){
                        
                        char filenum_tmp[10];
                        fscanf(messageTosent,"%s\n",filenum_tmp);
                        filenum_tmp[strlen(filenum_tmp)] = '\0';
                        

                        int filenum = 0;
                        int i = 0;
                        while(isdigit(filenum_tmp[i])) {
                            filenum *= 10;
                            filenum += (filenum_tmp[i]-'0');
                            i++;
                        }

                        for(int i=0;i<filenum;i++){
                            
                            char filename[40];
                            fscanf(messageTosent,"%s\n",filename);
                            char sizex[10];
                            fscanf(messageTosent,"%s\n",sizex);
                            int size = 0;
                            int i = 0;
                            while(isdigit(sizex[i])) {
                                size *= 10;
                                size += (sizex[i]-'0');
                                i++;
                            }
                            
                            char unsent_buffer[size];
                            fread(unsent_buffer,1,size,messageTosent);

                            memset(sizex, '\0', sizeof(sizex));
                            memset(filename,'\0',sizeof(filename));
                            memset(unsent_buffer,'\0',sizeof(unsent_buffer));
                            memset(filenum_tmp,'\0',sizeof(filenum_tmp));


                        }
                    }
                    else if(filetype =='n'){
                        char y_size[10];
                        fscanf(messageTosent, "%s\n", y_size);
                        y_size[strlen(y_size)] = '\0';
                        // fprintf(stderr, "u_size here =  %d\n", u_size);
                        int y_size_num = 0;
                        int i = 0;
                        while(isdigit(y_size[i])) {
                            y_size_num *= 10;
                            y_size_num += (y_size[i]-'0');
                            i++;
                        }
                        char y_message[y_size_num];
                        fread(y_message,1,y_size_num,messageTosent);
                    
                        y_message[y_size_num]='\0';
                        // fprintf(stderr, "In else, y_message = %s\n", y_message);
                        memset(y_message,'\0',sizeof(y_message));
                    }


                }

            }   //end if strcmp
            else if(strcmp(status, "S1") == 0) {

                fscanf(messageTosent,"\n%c\n",&filetype);
                // fprintf(stderr, "In Status S1, filetype = %c\n", filetype );
                if(filetype == 'y'){
                    
                    char filenum_tmp[10];
                    fscanf(messageTosent,"%s\n",filenum_tmp);
                    filenum_tmp[strlen(filenum_tmp)] = '\0';
                    

                    int filenum = 0;
                    int i = 0;
                    while(isdigit(filenum_tmp[i])) {
                        filenum *= 10;
                        filenum += (filenum_tmp[i]-'0');
                        i++;
                    }

                    for(int i=0;i<filenum;i++){
                        
                        char filename[40];
                        fscanf(messageTosent,"%s\n",filename);
                        char sizex[10];
                        fscanf(messageTosent,"%s\n",sizex);

                        int size = 0;
                        int i = 0;
                        while(isdigit(sizex[i])) {
                            size *= 10;
                            size += (sizex[i]-'0');
                            i++;
                        }

                        char unsent_buffer[size];
                        fread(unsent_buffer,1,size,messageTosent);
                        memset(sizex, '\0', sizeof(sizex));
                        memset(filename,'\0',sizeof(filename));
                        memset(unsent_buffer,'\0',sizeof(unsent_buffer));

                    }
                }
                else if(filetype =='n'){
                    char u_size[10];
                    fscanf(messageTosent, "%s\n", u_size);
                    u_size[strlen(u_size)] = '\0';

                    int u_size_num = 0;
                    int i = 0;

                    while(isdigit(u_size[i])) {
                        u_size_num *= 10;
                        u_size_num += (u_size[i]-'0');
                        i++;
                    }
                    // fprintf(stderr, "In Status S1, u_size = %d\n", u_size);
                    char u_message[u_size_num];
                    fread(u_message,1,u_size_num,messageTosent);
                    u_message[u_size_num]='\0';
                    // fprintf(stderr, "In Status S1, u_message = %s\n", u_message);

                    memset(u_message,'\0',sizeof(u_message));
                }

            }

            filetype = '\0';
            memset(source,'\0',sizeof(source));
            memset(name,'\0',sizeof(name));
            memset(status, '\0', sizeof(status));

        }   //end of while

        // fclose(logfile);
        fclose(messageTosent);
        
        //store the online client's socket number to array
        strcpy(ONLINE[n].username, username);
        ONLINE[n].sockno = connfd;  
        n++;
        
        struct client_info cl;
        cl.first = 1;
        cl.socketno = connfd;
        strcpy(cl.username, username);
        strcpy(cl.dest, destination);


        /* ========================== FILE TRANSFER ============================ */
        
        // fprintf(stderr, "n = %d\n", n);
        int mark = 0;
        
        char isFile = '\0';
        char no = 'n';
        char yes = 'y';


        while(1) {
            
            isFile = '\0';
            int len = 0;
            char msg[2048] = "";
            int mark = recv(cl.socketno, &isFile,1, 0);
            if(mark > 0) {
               

                if(isFile == no) {
                    fprintf(stderr,"receiving a message from %s.\n", username);
                    len = recv(cl.socketno, msg, 2048, 0);
                    msg[len] = '\0';
                    
                    if(len < 0) {
                        perror("error in receiving the message.\n");
                        exit(1);
                    }
                    
                    // fprintf(stderr, "Message received = %s\n", msg);


                    //check whether the dest_client is online by looking at ONLINE[]
                    int dest_socket = -1;
                    for(int i = 0; i < n; i++) {
                        if(strcmp(cl.dest, ONLINE[i].username) == 0) {
                            dest_socket = ONLINE[i].sockno;
                            break;
                        }
                    }

                    if(dest_socket != -1) {
                        fprintf(stderr, "IN MESSAGE, THE DESTINATION CLIENT %s IS ONLINE.\n", cl.dest);

                        FILE *log1 = fopen("log.txt", "a");
                        // fprintf(stderr, "AT ONLINE, MESSAGE YANG MAU DIKIRIM = %s\n", msg);

                        int sizey = strlen(msg);
                        char size_m[10];
                        sprintf(size_m,"%d", sizey);

                        int ret_b  = send(dest_socket, &isFile, sizeof(char), 0);
                        if(ret_b < 0) fprintf(stderr, "error in sending the isfile in online\n");

                        // fprintf(stderr, "SEND MESSAGE SIZE = %s\n", size_m);
                        int ret_c = send(dest_socket, size_m, 10,0);
                        if(ret_c < 0) fprintf(stderr, "error in sending size online\n");
                        int ret = send(dest_socket, msg, sizey, 0);
                        if(ret > 0) fprintf(log1, "%s %s %lu %s\n", cl.username, cl.dest, strlen(msg), msg);
                        else if(ret < 0) {
                            perror("sending message to client error.\n");
                            // exit(1);
                        }
                        
                        ret = 0;
                        memset(msg,'\0',sizeof(msg));
                        memset(size_m,'\0',sizeof(size_m));
                        fclose(log1);
                    }
                    else {
                        fprintf(stderr, "IN MESSAGE, THE DESTINATION CLIENT %s IS OFFLINE.\n", cl.dest);
                        FILE *toSent = fopen("messagefile.txt","a");
                        
                        
                        // fprintf(stderr, "AT OFFLINE, MESSAGE YANG MAU DIKIRIM = %s.\n", msg);
                        fprintf(toSent,"%s %s\n",cl.username, cl.dest);
                        fprintf(toSent, "S0\n");
                        fprintf(toSent, "%c\n", isFile);
                        fprintf(toSent, "%lu\n",strlen(msg));
                        fprintf(toSent,"%s\n",msg);

                        //write to the log.txt
                        FILE *logyou = fopen("log.txt", "a");
                        fprintf(logyou, "%s %s %lu %s\n", cl.username, cl.dest, strlen(msg), msg);
                        memset(msg,'\0',sizeof(msg));
                        fclose(toSent); 
                        fclose(logyou);

                    }

                }
                else if(isFile == yes) {
                    fprintf(stderr, "receiving files from %s.\n", username);
                    //check whether the dest_client is online by looking at ONLINE[]
                    int dest_socket = -1;
                    for(int i = 0; i < n; i++) {
                        if(strcmp(cl.dest, ONLINE[i].username) == 0) {
                            dest_socket = ONLINE[i].sockno;
                            break;
                        }
                    }

                    int filenum = 0;
                    
                    if(dest_socket != -1) {
                        fprintf(stderr, "IN FILE, THE DESTINATION CLIENT %s IS ONLINE.\n", cl.dest);
                        FILE *log1 = fopen("log.txt", "a");

                        //FIRST YOU RECEIVE ISFILE AND FILENUM
                        char filenumx[10];

                        int ret_num = read(cl.socketno,filenumx,10);
                        filenumx[strlen(filenumx)] = '\0';

                        if(ret_num < 0) fprintf(stderr,"recv filenum error in filetransfer.\n");
                        // fprintf(stderr,"filenum langsung = %s.\n",filenumx);

                    /* ========= SEND THE ISFILE AND FILENUMX ========= */
                        
                        int i = 0;
                        while(isdigit(filenumx[i])) {
                            filenum *= 10;
                            filenum += (filenumx[i]-'0');
                            i++;
                        }

                        //send the isfile
                        send(dest_socket, &isFile, sizeof(char), 0);
                        // fprintf(stderr, "ISFILE = %c\n",isFile);

                        //send the filenumx
                        send(dest_socket, filenumx, 10, 0);
                        // fprintf(stderr, "FILENUMX = %s\n", filenumx);
                        
                        // fprintf(stderr,"hello\n");
                        fclose(log1);
                    }
                    else {
                        fprintf(stderr, "IN FILE, THE DESTINATION CLIENT %s IS OFFLINE.\n", cl.dest);
                        FILE *toSent = fopen("messagefile.txt","a");

                        //FIRST YOU RECEIVE ISFILE AND FILENUM
                        char filenumx[10];

                        int ret_num = read(cl.socketno,filenumx,10);
                        filenumx[strlen(filenumx)] = '\0';

                        if(ret_num < 0) fprintf(stderr,"recv filenum error in filetransfer.\n");
                        // fprintf(stderr,"filenum langsung = %s.\n",filenumx);

                        int i = 0;
                        while(isdigit(filenumx[i])) {
                            filenum *= 10;
                            filenum += (filenumx[i]-'0');
                            i++;
                        }
                        /* ========= WRITE THE ISFILE AND FILENUMX INTO MESSAGEFILE.TXT ========= */

                        fprintf(toSent,"%s %s\n",cl.username, cl.dest);
                        fprintf(toSent, "S0\n");
                        fprintf(toSent, "%c\n", isFile);
                        fprintf(toSent, "%s\n", filenumx);
                        fclose(toSent);
                    }

                    //SEND THE FILENAME AND ITS CONTENT

                    char filename[20];

                    for(int i = 0; i < filenum; i++) {
                        // fprintf(stderr, "i = %d\n", i);
                        
                        if(dest_socket != -1) {

                            FILE *log10 = fopen("log.txt", "a");
                            
                            int recv_filename = recv(cl.socketno, filename, 20, 0);
                            if(recv_filename < 0) fprintf(stderr,"recv error in filetransfer.\n");
                            filename[strlen(filename)]='\0';


                            //send filename
                            // fprintf(stderr, "FILENAME = %s\n", filename);
                            int ret0 = send(dest_socket, filename, 20, 0);
                            // if(ret0 > 0) fprintf(log10, "%s %s %s\n", cl.username, cl.dest, filename);



                            //read the size 
                            char size_temp[10];
                            int recv_size = recv(cl.socketno,size_temp,10,0);
                            if(recv_size<0) fprintf(stderr,"recv_size error\n");
                            // fprintf(stderr, "SIZE_TEMP = %s\n", size_temp);
                            // size = atoi(size_temp);

                            //send the size of the file 
                            // fprintf(stderr, "FILESIZE = %s\n",size_temp);
                            // char w_size_temp[10];
                            // sprintf(w_size_temp,"%d",size);
                            // int send_size = send(dest_socket,w_size_temp,10,0);
                            // if(send_size<0) fprintf(stderr,"send_size error\n");

                            int send_size = send(dest_socket,size_temp,10,0);
                            if(send_size<0) fprintf(stderr,"send_size error\n");


                            int size = 0;
                            int i = 0;
                            while(isdigit(size_temp[i])) {
                                size *= 10;
                                size += (size_temp[i]-'0');
                                i++;
                            }


                            //read the content of the file
                            char buffer[size];
                            int readsize = recv(cl.socketno,buffer,size,0);
                            buffer[size]='\0';

                            if(readsize < 0) fprintf(stderr, "error recv file buffer.\n");

                            //send message
                            // fprintf(stderr, "FILE MESSAGE = %s\n",buffer);
                            int send_message = send(dest_socket,buffer,size,0);
                            if(send_message<0) fprintf(stderr,"send_message error\n");

                            memset(filename,'\0',sizeof(filename));
                            memset(size_temp,'\0',sizeof(size_temp));
                            memset(buffer,'\0',sizeof(buffer));
                            fclose(log10);
                        }
                        else {

                            FILE *toSent1 = fopen("messagefile.txt","a");


                            int recv_filename = recv(cl.socketno, filename, 20, 0);
                            if(recv_filename < 0) fprintf(stderr,"recv error in filetransfer.\n");
                            filename[strlen(filename)]='\0';
                            // fprintf(stderr,"hello\n");
                            
                            //read the size 
                            char size_temp[10];
                            int recv_size = recv(cl.socketno,size_temp,10,0);
                            if(recv_size<0) fprintf(stderr,"recv_size error\n");
                            // fprintf(stderr, "SIZE_TEMP = %s\n", size_temp);
                            // size = atoi(size_temp);

                            int size = 0;
                            int i = 0;
                            while(isdigit(size_temp[i])) {
                                size *= 10;
                                size += (size_temp[i]-'0');
                                i++;
                            }

                            //read the content of the file
                            char buffer[size];
                            int readsize = recv(cl.socketno,buffer,size,0);
                            buffer[size]='\0';

                            if(readsize < 0) fprintf(stderr, "error recv file buffer.\n");

                            /* ===== WRITE FILENAME, FILESIZE, CONTENT INTO MESSAGEFILE.TXT ======== */
                            
                            fprintf(toSent1, "%s\n", filename);
                            fprintf(toSent1, "%s\n", size_temp);
                            fprintf(toSent1,"%s\n",buffer);

                            memset(filename,'\0',sizeof(filename));
                            memset(size_temp,'\0',sizeof(size_temp));
                            memset(buffer,'\0',sizeof(buffer));
                            fclose(toSent1);
                        }     

                    }   //end for filenum
                }
            }   // end of if mark > 0
            // fprintf(stderr, "client %s MARK %d\n", cl.username, mark);
            if(mark == 0) {
                // fprintf(stderr, "Client disconnected.\n");
                if(cl.first == 1) {
                    fprintf(stderr, "%s is disconnected.\n", cl.username);
                    cl.first = 0;
                }
                //close the connections
                int enter_loop = 0;
                // fprintf(stderr, "n = %d\n", n);
                for(int i = 0; i < n; i++) {
                    // fprintf(stderr, "masuk for\n");
                    // fprintf(stderr, "%s is disconnected.\n", cl.username);
                    if(strcmp(ONLINE[i].username, cl.username) == 0) {
                        enter_loop = 1;
                        for(int j = i; j < n-1; j++) {
                            ONLINE[j] = ONLINE[j+1];
                        }
                    }
                }
                if(enter_loop == 1) {
                    n--;
                    enter_loop = 0;
                }
                fflush(stdout);
            }
            else if(mark == -1) {
                perror("recv failed");
                // exit(1);
            }
        } 
        // fprintf(stderr, "%s is disconnected.\n", cl.username);


    }   // if validUser && validDest

    // pthread_detach(pthread_self());
    return NULL;
}