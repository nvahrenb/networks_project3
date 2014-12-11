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
	char username[16];

};

int main(int argc, char *argv[]){

	// kinda temporary, want to do a list eventually but
	// i'll add that after we make sure everything else
	// works
	struct client client_list[MAX_CLIENTS];
	int i, j, k;
	for(i = 0; i < MAX_CLIENTS; i++){
		client_list[i].addr.sin_family = AF_INET;
		strcpy(client_list[i].username, "EMPTY");
	}
	
	// initialize network stuff
	
	int sockfd;
	char recvBuffer[BUFSIZE];
	char sendBuffer[BUFSIZE];
	char tempBuffer1[BUFSIZE];
	char tempBuffer2[BUFSIZE];
	char input[64];
	int recvlen;
	int port = DPORT;
	
	#ifdef DEBUG
		printf("Opening socket...\n");
	#endif
	
	struct sockaddr_in serverAddr, clientAddr;
	socklen_t len = sizeof(serverAddr);
	
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
	
	/*// user port override
	if(argc >= 3){
		port = atoi(argv[2]);
	}
	
	// get host address from argument
	struct hostent *hostn = gethostbyname(argv[1]);
	if(hostn == 0){
		perror("unable to get address of host");
		return 0;
	}*/

	struct hostent *hostn;
	if(argc <= 1)
	{
		perror("not enough arguments\n");
		return 0;
	}
	else
	{
		strcpy(tempBuffer1, argv[1]);
		for(i = 0; tempBuffer1[i] != ':'; i++)
		{
			tempBuffer2[i] = tempBuffer1[i];
		}
		tempBuffer2[i] = '\0';
		hostn = gethostbyname(tempBuffer2);
		if(hostn == 0)
		{
			perror("unable to get address of host");
			return 0;
		}

		for(j = i, k = 0; tempBuffer1[j] != '\0'; j++, k++)
		{
			tempBuffer2[k] = tempBuffer1[j];
		} 
		tempBuffer2[k] = '\0';
		port = atoi(tempBuffer2);
		if(port != DPORT)
		{
			port = DPORT;
		}
	}
	
	// set server address info
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	memcpy((void *)&serverAddr.sin_addr, hostn->h_addr_list[0], hostn->h_length);
	
	#ifdef DEBUG
		printf("Connecting to server at %s (%s) on port %d\n",argv[1],inet_ntoa(serverAddr.sin_addr),port);
	#endif
	
	// request list from server
	while(1)
	{
		printf("P2PChat>");
		fgets (input, 64, stdin);
		if(strcmp(input, "list") == 0)
		{
			strcpy(sendBuffer,"L:");
			#ifdef DEBUG
				printf("Sending to server: %s\n",sendBuffer);
			#endif
			int sentBytes = sendto(sockfd, sendBuffer, strlen(sendBuffer), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	
			// listen for response - list of groups
			recvlen = recvfrom(sockfd, recvBuffer, BUFSIZE, 0, (struct sockaddr *)&serverAddr, &len);
			recvBuffer[recvlen] = 0;
			#ifdef DEBUG
				printf("Received response: %s\n",recvBuffer);
			#endif
	
			// parse response and print to screen
			//printf("Current groups:\n");
			for(i = 2; recvBuffer[i]; i++){
				if(recvBuffer[i] == ':'){
					printf("\n");
				}else{
					printf("%c",recvBuffer[i]);
				}
			}
		}
		else if(strcmp(input, "join") == 0)
		{
			// get user input
			printf("Please type: join group username\n > ");
			fgets (input, 64, stdin);
			printf("input: %s\n",input);
			strcpy(sendBuffer, "J:");
			i = 0;
			while(input[i] != ' '){
				i++;
			}
			i++;
			for(j = 2, k = 0; input[i] != ' '; i++, j++, k++){
				tempBuffer1[k] = input[i];
				sendBuffer[j] = input[i];
			}
			tempBuffer1[k] = '\0';
			sendBuffer[j] = ':';
			i++; j++;
			for(; input[i] && input[i] != ' ' && input[i] != '\n'; i++, j++){
				sendBuffer[j] = input[i];
			}
			sendBuffer[j] = ':';
			sendBuffer[j+1] = ':';
	
			// request join
			#ifdef DEBUG
				printf("Sending to server: %s\n",sendBuffer);
			#endif
			sendto(sockfd, sendBuffer, strlen(sendBuffer), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	
	
			// listen for response
			recvlen = recvfrom(sockfd, recvBuffer, BUFSIZE, 0, (struct sockaddr *)&clientAddr, &recvlen);
			recvBuffer[recvlen] = 0;
			#ifdef DEBUG
				printf("Received response: %s\n", recvBuffer);
			#endif
	
			// save list of clients
			i = 0; j = 1;
			char temp[16];
			memset((char *)&temp, 0, sizeof(temp));
			while(recvBuffer[j] != ':' || recvBuffer[j+1] != ':'){
				while(strcmp(client_list[i].username, "EMPTY") != 0){
					i++;
				}

				// save username
				j++;
				for(k = 0; recvBuffer[j] != ':'; j++, k++){
					temp[k] = recvBuffer[j];
				}
				strcpy(client_list[i].username, temp);
				memset((char *)&temp, 0, sizeof(temp));
		
				// save address
				j++;
				for(k = 0; recvBuffer[j] != ':'; j++, k++){
					temp[k] = recvBuffer[j];
				}
				inet_aton(temp, &client_list[i].addr.sin_addr);
				memset((char *)&temp, 0, sizeof(temp));
		
				// save port
				j++;
				for(k = 0; recvBuffer[j] != ':'; j++, k++){
					temp[k] = recvBuffer[j];
				}
				client_list[i].addr.sin_port = atoi(temp);
				memset((char *)&temp, 0, sizeof(temp));
			}
	
			#ifdef DEBUG
				printf("Other clients in group:\n");
				for(i = 0; i < MAX_CLIENTS; i++){
					if(strcmp(client_list[i].username, "EMPTY") != 0){
						printf("%s    %s:%d\n",
						client_list[i].username,
						inet_ntoa(client_list[i].addr.sin_addr),
						client_list[i].addr.sin_port);
					}
				}
			#endif
			
			while(1)
			{
				printf("%s>", tempBuffer1);
				fgets (tempBuffer2, BUFSIZE, stdin);
				if(strcmp(input, "leave") == 0)
				{
					sendto(sockfd, "D::", 3, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
					break;
				}
				else if(strcmp(input, "quit") == 0)
				{
					sendto(sockfd, "D::", 3, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
					return 0;
				}
				for(i = 0; i < MAX_CLIENTS; i++)
				{
					if(strcmp(client_list[i].username, "EMPTY") != 0)
					{
						printf("%s    %s:%d\n",
						client_list[i].username,
						inet_ntoa(client_list[i].addr.sin_addr),
						client_list[i].addr.sin_port);
					}
				}
			}
		}
	
	sendto(sockfd, "D::", 3, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

		// notify other clients that this user has joined
	
		// begin chat loop
	}
	
}
