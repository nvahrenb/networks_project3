/*
Chris Ray
Nathan Vahrenberg

CSE30264 - Computer Networks

Project 3 - P2P Chat Client
*/

// A general idea of program flow:

/*** Initial connection ***/

// contact rendezvous server

// send 'request list'
// format "L:"

// wait for response from server
// format "G:group1:group2:group3::" (arbitrary no of groups)

// send 'request to join'
// format "J:groupname:username::"

// receive arbitrary number of other clients
// format: C:c1_username:c1_address:c1_port:c2_hostname...cn_port::

// send message to other clients that you have joined the group
// format undecided

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

#define DPORT 9765
#define BUFSIZE 4096
#define MAX_CLIENTS 20

struct client{

	struct sockaddr_in addr;
	char *group;
	char *username;

};

int main(int argc, char *argv[]){

	// kinda temporary, want to do a list eventually but
	// i'll add that after we make sure everything else
	// works
	struct client client_list[MAX_CLIENTS];
	int i, j;
	for(i = 0; i < MAX_CLIENTS; i++){
		client_list[i].addr.sin_family = AF_INET;
		client_list[i].username = "NULL";
		client_list[i].group = "NULL";
	}
	
	// initialize network stuff
	
	int sockfd;
	char recvBuffer[BUFSIZE];
	char sendBuffer[BUFSIZE];
	int recvlen;
	
	int port = DPORT;
	
	#ifdef DEBUG
		printf("Opening socket...\n");
	#endif
	
	struct sockaddr_in serverAddr, clientAddr;
	socklen_t len = sizeof(clientAddr);
	
	// open socket
	if( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		perror("unable to open socket");
		return 0;
	}
	
	bzero(&serverAddr,sizeof(serverAddr));
	
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	clientAddr.sin_port = htons(port);
	
	// bind socket
	if( bind(sockfd, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) < 0){
		perror("unable to bind");
		return 0;
	}
	
	// user port override
	if(argc >= 3){
		port = atoi(argv[2]);
	}
	
	// get host address from argument
	struct hostent *hostn = gethostbyname(argv[1]);
	if(hostn == 0){
		perror("unable to get address of host");
		return 0;
	}
	
	// set server address info
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	memcpy((void *)&serverAddr.sin_addr, hostn->h_addr_list[0], hostn->h_length);
	
	#ifdef DEBUG
		printf("Connecting to server at %s (%s) on port %d\n",argv[1],inet_ntoa(serverAddr.sin_addr),port);
	#endif
	
	// request list from server
	strcpy(sendBuffer,"L:");
	#ifdef DEBUG
		printf("Sending to server:\n");
		fputs(sendBuffer, stdout);
	#endif
	sendto(sockfd, sendBuffer, strlen(sendBuffer), 0, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr *));
	
	// listen for response - list of groups
	recvlen = recvfrom(sockfd, recvBuffer, BUFSIZE, 0, (struct sockaddr *)&clientAddr, &len);
	recvBuffer[recvlen] = 0;
	#ifdef DEBUG
		printf("Received response:\n");
		fputs(recvBuffer, stdout);
	#endif
	
	// parse response and print to screen
	printf("Current groups:\n");
	for(i = 2; recvBuffer[i]; i++){
		if(recvBuffer[i] == ':'){
			printf("\n");
		}else{
			printf("%c",recvBuffer[i]);
		}
	}
	// get user input
	char input[64];
	scanf("%s",input);
	strcpy(sendBuffer, "J:");
	i = 0;
	while(input[i] != ' '){
		i++;
	}
	for(j = 2; input[i] != ' '; i++, j++){
		sendBuffer[j] = input[i];
	}
	sendBuffer[j] = ':';
	i++; j++;
	for(; input[i] && input[i] != ' '; i++, j++){
		sendBuffer[j] = input[i];
	}
	sendBuffer[j] = ':';
	sendBuffer[j+1] = ':';
	
	// request join
	#ifdef DEBUG
		printf("Sending to server:\n");
		fputs(sendBuffer, stdout);
	#endif
	sendto(sockfd, sendBuffer, strlen(sendBuffer), 0, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr *));
	
	
	// listen for response
	recvlen = recvfrom(sockfd, recvBuffer, BUFSIZE, 0, (struct sockaddr *)&clientAddr, &len);
	recvBuffer[recvlen] = 0;
	#ifdef DEBUG
		printf("Received response:\n");
		fputs(sendBuffer, stdout);
	#endif
	
	// save list of clients
	i = 0; j = 2;
	char *temp = malloc(16*sizeof(char));
	memset((char *)&temp, 0, sizeof(temp));
	while(recvBuffer[j] != ':' || recvBuffer[j+1] != ':'){
		while(client_list[i].username == "NULL"){
			i++;
		}
		
		// save username
		int k;
		for(k = 0; recvBuffer[j] != ':'; j++, k++){
			temp[k] = recvBuffer[j];
		}
		strcpy(client_list[i].username, temp);
		memset((char *)&temp, 0, sizeof(temp));
		
		// save address
		for(k = 0; recvBuffer[j] != ':'; j++, k++){
			temp[k] = recvBuffer[j];
		}
		inet_aton(temp, &client_list[i].addr.sin_addr);
		memset((char *)&temp, 0, sizeof(temp));
		
		// save port
		for(k = 0; recvBuffer[j] != ':'; j++, k++){
			temp[k] = recvBuffer[j];
		}
		client_list[i].addr.sin_port = atoi(temp);
		memset((char *)&temp, 0, sizeof(temp));
	}
	free(temp);
	
	printf("end");

	// notify other clients that this user has joined
	
	// begin chat loop
	
	
}
