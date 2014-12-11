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

#define DEBUG
#define DPORT 9421
#define BUFSIZE 4096
#define MAX_CLIENTS 20

struct client{

	struct sockaddr_in addr;
	char group[16];
	char username[16];

};

int main(int argc, char *argv[]){

	// kinda temporary, want to do a list eventually but
	// i'll add that after we make sure everything else
	// works
	struct client client_list[MAX_CLIENTS];
	int i, j;
	for(i = 0; i < MAX_CLIENTS; i++){
		client_list[i].addr.sin_family = AF_INET;
		client_list[i].addr.sin_addr.s_addr = htonl(INADDR_ANY);
		client_list[i].addr.sin_port = DPORT;
		strcpy(client_list[i].username, "NULL");;
		strcpy(client_list[i].group, "NULL");
	}

	// initialize network stuff
	
	int sockfd;
	char recvBuffer[BUFSIZE];
	char sendBuffer[BUFSIZE];
	int recvlen;
	
	int port = DPORT;
	
	if(argc >= 2){
		port = atoi(argv[1]);
	}
	
	struct sockaddr_in clientAddr, serverAddr;
	socklen_t len = sizeof(clientAddr);
	
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

	// loop indefinitely
	while(1){
	
		#ifdef DEBUG
			printf("Listening for connections on port %d\n",ntohs(serverAddr.sin_port));
		#endif
	
		// begin listening for connections
		memset((char *)&recvBuffer, 0, sizeof(recvBuffer));
		memset((char *)&clientAddr, 0, sizeof(clientAddr));
		recvlen = recvfrom(sockfd,recvBuffer,BUFSIZE,0,(struct sockaddr *)&clientAddr, &len);
		recvBuffer[recvlen] = 0;
		perror("recvfrom");
		
		#ifdef DEBUG
			printf("Received request: %s from %s\n",recvBuffer, inet_ntoa(clientAddr.sin_addr));
		#endif
		
		if(recvBuffer[0] == 'L'){
		// list request
		#ifdef DEBUG
			printf("Client requested group list\n");
		#endif
	
			// send client list of groups
			// format "G:group1:group2:group3::
			strcpy(sendBuffer, "G:");
			char used_groups[16][16];
			for(j = 0; j < 16; j++){
				memset(used_groups[j], 0, sizeof(used_groups[j]));
			}
			int isNewGroup;
			for(i = 0; i < MAX_CLIENTS; i++){
				if(strcmp(client_list[i].group, "NULL") != 0){
					// check if duplicate
					isNewGroup = 1;
					for(j = 0; j < 16; j++){
						if (strcmp(client_list[i].group, used_groups[j]) == 0){
							isNewGroup = 0;
						}
					}
					// add to sendBuffer
					if(isNewGroup){
						strcat(sendBuffer, client_list[i].group);
						strcat(sendBuffer, ":");
						j = 0;
						while(used_groups[j][0] != 0){
							j++;
						}
						strcpy(used_groups[j], client_list[i].group);
					}
				}
			}
			strcat(sendBuffer, ":");
			#ifdef DEBUG
				printf("Sending response: %s to %s\n",sendBuffer, inet_ntoa(clientAddr.sin_addr));
			#endif
			sendto(sockfd, sendBuffer, strlen(sendBuffer), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
		// end list
		
		
		
		}else if(recvBuffer[0] == 'J'){
			//join request
			#ifdef DEBUG
				printf("Client requested to join ");
			#endif
			// string format J:group:username::
			char *newGroup = malloc(16*sizeof(char));
			char *newUser = malloc(16*sizeof(char));
				
			// determine what group the user requested
			for(i = 2, j = 0; recvBuffer[i] != ':'; i++, j++){
				newGroup[j] = recvBuffer[i];
			}
			
			#ifdef DEBUG
				printf("%s as ",newGroup);
			#endif
		
			// get username
			i++;
			
			for(j = 0; recvBuffer[i] != ':'; i++, j++){
				newUser[j] = recvBuffer[i];
			}
			
			#ifdef DEBUG
				printf("%s\n",newUser);
			#endif
			
			
		
			// send client list of others in group
			strcpy(sendBuffer, "C:");
			for(i = 0; i < MAX_CLIENTS; i++){
				if(strcmp(client_list[i].group, "NULL") != 0 && strcmp(client_list[i].group, newGroup) == 0){
					// send other clients' address info
					// format: C:c1_username:c1_address:c1_port:c2_hostname...cn_port::
					strcat(sendBuffer, client_list[i].username);
					strcat(sendBuffer, ":");
					strcat(sendBuffer, inet_ntoa(client_list[i].addr.sin_addr));
					strcat(sendBuffer, ":");
					char *temp = malloc(6*sizeof(char));
					sprintf(temp,"%d",client_list[i].addr.sin_port);
					strcat(sendBuffer, temp);
					strcat(sendBuffer, ":");
					free(temp);
				}
			}
			strcat(sendBuffer, ":");
			#ifdef DEBUG
				printf("Sending response: %s\n",sendBuffer);
			#endif
			sendto(sockfd, sendBuffer, strlen(sendBuffer), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
			
	
	
			// save this new client
			#ifdef DEBUG
				printf("Adding %s at %s:%d to %s\n",newUser,inet_ntoa(clientAddr.sin_addr),clientAddr.sin_port,newGroup);
			#endif
			i = 0;
			while(strcmp(client_list[i].group, "NULL") != 0){
				i++;
			}
			client_list[i].addr.sin_addr.s_addr = clientAddr.sin_addr.s_addr;
			int temp_port = clientAddr.sin_port;
			client_list[i].addr.sin_port = temp_port;
			strcpy(client_list[i].username, newUser);
			strcpy(client_list[i].group, newGroup);
	
		free(newGroup);
		free(newUser);
	
		// end join
		
		
		
		}else if(recvBuffer[0] == 'D'){
			// disconnect request
			#ifdef DEBUG
				printf("Client requested to disconnect\n");
			#endif
		
			i = 0;
			while(client_list[i].addr.sin_addr.s_addr != clientAddr.sin_addr.s_addr){
				i++;
			}
			
			strcpy(sendBuffer, "D:");
			strcat(sendBuffer, client_list[i].username);	
			strcat(sendBuffer, "::");
			
			// notify all clients about disconnect
			for(j = 0; j < MAX_CLIENTS; j++){ // for all clients
				if(strcmp(client_list[j].group, client_list[i].group) == 0){ // if client is in group
					// send disconnect message
					//format D:username::
					#ifdef DEBUG
						printf("Sending %s to %s\n",sendBuffer, inet_ntoa(client_list[j].addr.sin_addr));
					#endif
					sendto(sockfd, sendBuffer, strlen(sendBuffer), 0, (struct sockaddr *)&client_list[j].addr.sin_addr.s_addr, sizeof(client_list[j].addr));
				}
			}
			
			strcpy(client_list[i].username, "NULL");
			strcpy(client_list[i].group, "NULL");
		
		// end disconnect
		}else{
			printf("Unknown command: %s\n",recvBuffer);
		}
		
	}
}
