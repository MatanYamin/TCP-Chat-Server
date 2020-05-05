//Matan Yamin, ID: 311544407
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <sys/types.h>
#include <netdb.h>
#include <sys/select.h>
#include <errno.h>
#define MAXSIZE 1024
//Matan Yamin, ID: 311544407
void errFunc(int socket, char *funcName);//private func that closes socket and sends relevant error
void checkArgs(int argc, char *argv[]);//private func that checks the propriety of args

int main(int argc, char *argv[]){
    checkArgs(argc,argv);//check
    int maxClients = atoi(argv[2]);//will be the max clients that can run in same time
    int port = atoi(argv[1]);//will be the port that enterd in place 2
    int welcomeSocket;//will be the main socket we gonna use
    if ((welcomeSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1){ //creating our socket
        perror("error: socket() failed"); //check failure
        exit(EXIT_FAILURE);
        }
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);
    if (bind(welcomeSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
        errFunc(welcomeSocket, "bind");
        }
    if (listen(welcomeSocket, maxClients) == -1){
        errFunc(welcomeSocket,"listen");
        }
    char data[MAXSIZE];//will be the read outcome
    int rc,curNumOfClients = 0, servingSocket, flagFd;
    fd_set writeFds,readFds, generalFds;//crating the relevants fds
    FD_ZERO(&generalFds);//zero the gen fds array
    FD_ZERO(&writeFds);//zero the write fds array
    FD_SET(welcomeSocket, &generalFds);//put the welcome socket into the gen
    
    while(1){
        readFds = generalFds;//every itteration we will copy to the readFds
        flagFd = 0;
        int sel = select(getdtablesize(), &readFds, &writeFds, NULL, NULL);
        if (sel == -1){//if select has failed
            errFunc(welcomeSocket,"select");}//ex rules
        for (int fd = welcomeSocket; fd < getdtablesize(); fd++){
            if (FD_ISSET(fd, &readFds)){//we check if sock i is in read_fd, if so, ready to read
                printf("fd %d is ready to read\n", fd);//ex rules
                if (flagFd == 0){//if flag is 0 so the fd is welcomeSocket
                    servingSocket = accept(welcomeSocket, NULL, NULL);//adding the new client
                    if (servingSocket == -1){//if accept failed
                        errFunc(welcomeSocket,"accept");}
                    if (maxClients > curNumOfClients){//check the amount if clients
                        FD_SET(servingSocket, &generalFds);//if we have place, so add the new client
                        curNumOfClients++;//decrease the current clients num
                        if (maxClients == curNumOfClients){//when it happends, we clear the welc socket
                            FD_CLR(welcomeSocket, &generalFds);}
                    }
                }
                else{//if the fd is not the welcomeSOCK so we want to try and read from it
                    strncpy(data,"/0",MAXSIZE);
                    rc = read(fd, data, MAXSIZE);//read MAX (1024)
                    if (rc == 0){
                        close(fd);//if its zero we want to close the fd and clear freom general
                        FD_CLR(fd, &generalFds);
                        curNumOfClients--;//we closed the fd so decrease by 1 the current num
                        if (maxClients > curNumOfClients && !FD_ISSET(welcomeSocket,&generalFds) ){
                            FD_SET(welcomeSocket,&generalFds);}//if WelSocket not in general, add it(after we removed it)
                    }
                    else if(rc == -1){//incase read failed
                        errFunc(welcomeSocket,"read");}
                    else if(rc > 0){
                        for(int wfd = welcomeSocket; wfd < getdtablesize(); wfd++){//here we add the active fds to the writefd
                            if (FD_ISSET(wfd, &generalFds)){
                                if (wfd != welcomeSocket){//if the writeFD is not the welcome, so add it to writeFds
                                    printf("fd %d is ready to write\n", wfd);
                                    FD_SET(wfd, &writeFds);}
                                    }
                                }
                            }
                        }
                    }
                    flagFd =1;
            }
        for (int fd = welcomeSocket; fd < getdtablesize(); fd++){//write loop, at first, wont write anything cuz there nothing to read
            if (FD_ISSET(fd, &writeFds)){//if fd is in 'wtireFds' so ready to write to server
                rc = write(fd, data, MAXSIZE);//written to all clients that are active
                if (rc == -1){//check if failed
                    errFunc(welcomeSocket,"write");
                }
            FD_CLR(fd, &writeFds);//after we wrote, we clear it from the write fd
            }
        }
    }
    return 0;
}

void checkArgs(int argc, char *argv[]){

    if(argc != 3){//we must have 3
        perror("Usage: chatserver <port> <max-clients>\n");
        exit(EXIT_FAILURE);
    }
    for(int i = 1; i < argc; i++){//we must have port and num of clients
        if(atoi(argv[i]) <= 0){
            perror("Usage: chatserver <port> <max-clients>\n");
            exit(EXIT_FAILURE);
        }
    }
}

void errFunc(int socket, char *funcName){//this func closed socket that comes from an error and sends error message
    close(socket);
    printf("%s Failed()", funcName);
    exit(EXIT_FAILURE);
}