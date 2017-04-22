#include <string>
#include <iostream>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>		// for memset
using namespace std;

int main(int argc, char* argv[]) {
	if (argc != 4) {
		cerr << "Error: You must give three arguments when initializing the server.\n";
		exit(EXIT_FAILURE);
	}
	
	int port_num = stoi(argv[2]);
	
	if (port_num < 1024 || port_num > 65535) {
		cerr << "Error: You must choose a port number between 1024 and 65535.\n";
		exit(EXIT_FAILURE);
	}

	string filename = argv[3];
	
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;		// will point to the results
	
	memset(&hints, 0, sizeof hints);	// make sure the struct is empty
	hints.ai_family = AF_UNSPEC;		// don't care about IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;	// TCP stream sockets
	
	// get ready to connect
	status = getaddrinfo(argv[1], argv[2], &hints, &servinfo);
	
	if (status < 0) {
		cerr << "Error: hostname or ip address is invalid";
		exit(EXIT_FAILURE);
	}
}
