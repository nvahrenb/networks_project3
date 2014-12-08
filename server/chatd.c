/*
Chris Ray
Nathan Vahrenberg

CSE30264 - Computer Networks

Project 3 - P2P Chat Server
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define DPORT 3490
#define BUFSIZE 4096

int main(int argc, char *argv[]){

	// initialize

	int sockfd;
	int port = DPORT;
	char recvBuffer[BUFSIZE];
	int recvlen;
	
	struct sockaddr_in clientAddr, serverAddr;
	socklen_t len = sizeof(clientAddr);
	int r;
	int yes = 1;
	
	#ifdef DEBUG
		printf("Opening socket...\n");
	#endif

	if( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		perror("unable to open socket");
		return 0;
	}

	bzero(&serverAddr,sizeof(serverAddr));
	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(port);
	
	if( bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0){
		perror("unable to bind");
		return 0;
	}

	while(1){
	
		// begin listening for connections
		if((recvlen = recvfrom(sockfd,recvBuffer,BUFSIZE,0,(struct sockaddr *)&clientAddr,&len)) < 0){
			perror("unable to receive message");
			return 0;
		}
		recvBuffer[recvlen] = 0;
	
		// if a client connects, save their info
		
	
		// send client list of groups
	
	
		// wait for response
	
	
		// send client list of others in group
	
	
		// disconnect and wait for new connection
	
	
	}
}
