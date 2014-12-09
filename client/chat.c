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
	int i;
	for(i = 0; i < MAX_CLIENTS; i++){
		client_list[i].addr.sin_family = AF_INET;
		client_list[i].username = "NULL";
		client_list[i].group = "NULL";
	}
