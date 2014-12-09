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
	int recvlen;
	
	int port = DPORT;
	
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
	
		// begin listening for connections
		recvlen = recvfrom(sockfd,recvBuffer,BUFSIZE,0,(struct sockaddr *)&clientAddr, &len);
		recvBuffer[recvlen] = 0;
		
		if(recvBuffer[0] == 'J'){
		// join request
	
			// send client list of groups
			char *newGroup = malloc(16*sizeof(char));
			char toSend[4096];
			char *newUser = malloc(16*sizeof(char));
			
			for(i = 0; i < MAX_CLIENTS; i++){
				if(client_list[i].group != "NULL"){
					strcpy(toSend, "G:");
					// format "G:group1:group2:group3::
					strcat(toSend, client_list[i].group);
					strcat(toSend, ":");
				}
			}
			strcat(toSend, ":");
			sendto(sockfd, toSend, strlen(toSend), 0, (struct sockaddr *)&clientAddr, sizeof(struct sockaddr *));
	
			// wait for response
			// string format J:group:username::
			int recvlen = recvfrom(sockfd, recvBuffer, BUFSIZE, 0, (struct sockaddr *)&clientAddr, &len);
			recvBuffer[recvlen] = 0;
				
			// determine what group the user requested
			for(i = 2, j = 0; recvBuffer[i] != ':'; i++, j++){
				newGroup[j] = recvBuffer[i];
			}
		
			// get username
			i++;
			
			for(j = 0; recvBuffer[i] != ':'; i++, j++){
				newUser[j] = recvBuffer[i];
			}
		
			// send client list of others in group
			strcpy(toSend, "C:");
			for(i = 0; i < MAX_CLIENTS; i++){
				if(client_list[i].group != "NULL" && client_list[i].group == newGroup){
					// send other clients' address info
					// format: C:c1_username:c1_address:c1_port:c2_hostname...cn_port::
					strcat(toSend, client_list[i].username);
					strcat(toSend, ":");
					strcat(toSend, inet_ntoa(client_list[i].addr.sin_addr));
					strcat(toSend, ":");
					char *temp = malloc(6*sizeof(char));
					sprintf(temp,"%d",client_list[i].addr.sin_port);
					strcat(toSend, temp);
					strcat(toSend, ":");
					free(temp);
				}
			}
			strcat(toSend, ":");
			sendto(sockfd, toSend, strlen(toSend), 0, (struct sockaddr *)&clientAddr, sizeof(struct sockaddr *));
			
	
	
			// save this new client
			i = 0;
			while(client_list[i].group == "NULL"){
				i++;
			}
		
			client_list[i].addr.sin_addr.s_addr = clientAddr.sin_addr.s_addr;
			client_list[i].addr.sin_port = clientAddr.sin_port;
			client_list[i].username = newUser;
			client_list[i].group = newGroup;
	
		free(newGroup);
		free(newUser);
	
		// end join	
		}else if(recvBuffer[0] == 'D'){
			// disconnect request
		
			i = 0;
			while(client_list[i].addr.sin_addr.s_addr != clientAddr.sin_addr.s_addr){
				i++;
			}
			
			client_list[i].username = "NULL";
			client_list[i].group = "NULL";
		
		// end disconnect
		}
		
	}
}
