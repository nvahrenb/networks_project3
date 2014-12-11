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
#include <time.h>
#include <stdint.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>

#define DEBUG
#define DPORT 9421
#define BUFSIZE 4096
#define MAX_CLIENTS 20

struct client{

	struct sockaddr_in addr;
	char username[16];

};

struct param
{
	int s;
	struct sockaddr_in addr;
	struct client list[MAX_CLIENTS];
	char g[64];
};

void copyList(struct client * list, struct client * target)
{
	int i = 0;
	for(i = 0; i < MAX_CLIENTS; i++)
	{
		target[i] = list[i];
	}
}

void * p2p(void * p_input)
{
	printf("inside thread\n");
	struct param p = *(struct param * ) p_input;
	char buffer[BUFSIZE];
	char temp[BUFSIZE];
	char group[64];
	char user[64];
	char last_user[64] = "foo";
	uint32_t ID;
	uint32_t last_ID = 0;
	uint32_t mlength;
	int recvBytes, sendBytes, i, j;
	socklen_t len = sizeof(p.addr);
	while(1)
	{
		recvBytes = recvfrom(p.s, buffer, BUFSIZE, 0, (struct sockaddr *)&(p.addr), &len);
		if(recvBytes > 0)
		{
			buffer[recvBytes] = 0;

			if(buffer[0] == 'D')
			{
				i = 0;
				for(j = 2; buffer[j] != ':'; j++, i++)
				{
					temp[i] = buffer[j];
				}
				temp[i] = '\0';
				for(i = 0; i < MAX_CLIENTS; i++)
				{
					if(strcmp(p.list[i].username, temp) == 0)
					{
						strcpy(p.list[i].username, "EMPTY");
						break;
					}
				}

				printf("A client left.  Other clients in group:\n");
				for(i = 0; i < MAX_CLIENTS; i++)
				{
					if(strcmp(p.list[i].username, "EMPTY") != 0)
					{
						printf("%s    %s:%d\n",
						p.list[i].username,
						inet_ntoa(p.list[i].addr.sin_addr),
						p.list[i].addr.sin_port);
					}
				}
			}
			else if(buffer[0] == 'T')
			{
				i = 0;
				for(j = 2; buffer[j] != ':'; j++, i++)
				{
					temp[i] = buffer[j];
				}
				temp[i] = '\0';
				strcpy(group, temp);
				if(strcmp(group, p.g) != 0)
				{
					continue;
				}
		
				for(i = 0; buffer[j] != ':'; j++, i++)
				{
					temp[i] = buffer[j];
				}
				temp[i] = '\0';
				strcpy(user, temp);

				for(i = 0; buffer[j] != ':'; j++, i++)
				{
					temp[i] = buffer[j];
				}
				temp[i] = '\0';
				ID = ntohl(atoi(temp));

				if((strcmp(user, last_user) == 0) && (ID == last_ID))
				{
					continue;
				}

				for(i = 0; buffer[j] != ':'; j++, i++)
				{
					temp[i] = buffer[j];
				}
				temp[i] = '\0';
				mlength = ntohl(atoi(temp));

				for(i = 0; i < mlength; j++, i++)
				{
					temp[i] = buffer[j];
				}
				temp[i] = '\0';

				printf("From:  %s>%s\n", user, temp);

				for(i = 0; i < MAX_CLIENTS; i++)
				{
					if(strcmp(p.list[i].username, "EMPTY") != 0)
					{
						sendto(p.s, buffer, strlen(buffer), 0, (struct sockaddr *)&(p.list[i].addr.sin_addr.s_addr), sizeof(p.list[i].addr));
					}
				}

				for(i = 0; i < MAX_CLIENTS; i++)
				{
					if(strcmp(p.list[i].username, user) == 0)
					{
						j = 0;
						break;
					}
					j = 1;
				}
				if(j == 1)
				{
					for(i = 0; i < MAX_CLIENTS; i++)
					{
						if(strcmp(p.list[i].username, "EMPTY") == 0)
						{
							strcpy(p.list[i].username, user);
							p.list[i].addr.sin_addr.s_addr = p.addr.sin_addr.s_addr;
							p.list[i].addr.sin_port = p.addr.sin_port;
							break;
						}
					}
				}
			}
		}
		
	}
	
}

int main(int argc, char *argv[]){

	// kinda temporary, want to do a list eventually but
	// i'll add that after we make sure everything else
	// works
	struct client client_list[MAX_CLIENTS];
	srand(time(NULL));
	struct timeval myTime;
	struct param myParam;
	myTime.tv_sec = 1;
	myTime.tv_usec = 0;
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
	char username[BUFSIZE];
	char input[64];
	char user[64];
	char lastUser[64] = "foo";
	char group[64];
	uint32_t mID;
	uint32_t  last_mID = 1337;
	uint32_t  mLength;
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
	int optval = 1;
	//setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &myTime, sizeof(myTime));
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if( bind(sockfd, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) < 0){
		perror("unable to bind");
		return 0;
	}


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
		if(strcmp(input, "list\n") == 0)
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
		else if(strcmp(input, "join\n") == 0)
		{
			// get user input
			printf("Please type: join group username\n > ");
			fgets (input, 64, stdin);
			//printf("input: %s\n",input);
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
			for(k = 0; input[i] && input[i] != ' ' && input[i] != '\n'; i++, j++, k++){
				sendBuffer[j] = input[i];
				username[k] = input[i];
			}
			username[k] = '\0';
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

			myParam.s = sockfd;
			strcpy(myParam.g, tempBuffer1);
			copyList(client_list, myParam.list);
			
			pthread_t id1;
			if(pthread_create(&id1, NULL, p2p, (void *) &myParam) < 0)
			{
				perror("Could not create thread");
				return 0;
			}
			while(1)
			{
				printf("%s>", tempBuffer1);
				fgets (tempBuffer2, BUFSIZE, stdin);
				if(strcmp(tempBuffer2, "leave\n") == 0)
				{
					sendto(sockfd, "D::", 3, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
					break;
				}
				else if(strcmp(tempBuffer2, "quit\n") == 0)
				{
					sendto(sockfd, "D::", 3, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
					close(sockfd);
					return 0;
				}
				else if(strcmp(tempBuffer2, "list\n") == 0)
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
					continue;
				}
				else if(strcmp(tempBuffer2, "join\n") == 0)
				{
					printf("Please exit this group first.\n");
					continue;
				}
				else if(strcmp(tempBuffer2, "send\n") == 0)
				{
					printf(">");
					fgets (tempBuffer2, BUFSIZE, stdin);
					strcpy(sendBuffer, "T:");
					strcat(sendBuffer, tempBuffer1);
					strcat(sendBuffer, ":");
					strcat(sendBuffer, username);
					strcat(sendBuffer, ":");
					mID = htonl(rand()%100);
					char *temp1 = malloc(6*sizeof(char));
					sprintf(temp1,"%d",mID);
					strcat(sendBuffer, temp1);
					strcat(sendBuffer, ":");
					free(temp1);
					mLength = htonl(strlen(tempBuffer2));
					char *temp2 = malloc(6*sizeof(char));
					sprintf(temp2,"%d",mLength);
					strcat(sendBuffer, temp2);
					strcat(sendBuffer, ":");
					free(temp2);
					strcat(sendBuffer, tempBuffer2);
					sendBuffer[strlen(sendBuffer)] = '\0';
					
					for(i = 0; i < MAX_CLIENTS; i++)
					{
						if(strcmp(client_list[i].username, "EMPTY") != 0)
						{
							printf("%s    %s:%d\n",
								client_list[i].username,
								inet_ntoa(client_list[i].addr.sin_addr),
								client_list[i].addr.sin_port);
							
							sendto(sockfd, sendBuffer, strlen(sendBuffer), 0, (struct sockaddr *)&client_list[i].addr.sin_addr.s_addr, sizeof(client_list[i].addr));
						}
					}
					continue;
				}
				else if(strcmp(tempBuffer2, "\n") == 0)
				{
					continue;
				}
				else
				{
					printf("Invalid command.\n");
					continue;
				}
			}
		}

		else if(strcmp(input, "quit\n") == 0)
		{
			close(sockfd);
			return 0;
		}
		else
		{
			printf("Invalid command.\n");
		}
		// notify other clients that this user has joined
	
		// begin chat loop
	}
	
}
