#include <string>
#include <iostream>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>		// for memset
#include <stdexcept>	// invalid_argument
#include <fcntl.h>		// for getting a file descriptor
#include <unistd.h>		// for opening a file
#include <sys/sendfile.h>	// for sendfile
#include <sys/stat.h>	// for fstat to get the file size
#include <sys/select.h>	// using select for timeout
#include <sys/time.h>	// for the timeval structure
#include <stdio.h>		// for FILE objects
#include <cstdio>		// for file reading
using namespace std;

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
	if (argc != 4) {
		cerr << "ERROR: You must give three arguments when initializing the server.\n";
		exit(EXIT_FAILURE);
	}
	
	int port_num = 0;
	
	try {
		port_num = stoi(argv[2]);
	}
	catch(const invalid_argument& ia) {
		cerr << "ERROR: invalid argument - " << ia.what() << '\n';
		exit(EXIT_FAILURE);
	}
	
	if (port_num < 1023 || port_num > 65535) {
		cerr << "ERROR: You must choose a port number between 1024 and 65535.\n";
		exit(EXIT_FAILURE);
	}
	
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;		// will point to the results
	
	memset(&hints, 0, sizeof hints);	// make sure the struct is empty
	hints.ai_family = AF_INET;			// support only IPv4 (36th post in piazza)
	hints.ai_socktype = SOCK_STREAM;	// TCP stream sockets
	
	// get ready to connect
	status = getaddrinfo(argv[1], argv[2], &hints, &servinfo);
	
	if (status != 0) {
		cerr << "ERROR: hostname or ip address is invalid\n";
		exit(EXIT_FAILURE);
	}
	
	// make a socket
	int sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (sockfd < 0) {
		cerr << "ERROR: unable to make a socket" << endl;
		freeaddrinfo(servinfo);
		exit(EXIT_FAILURE);
	}
	
	// Set non-blocking
	long arg = fcntl(sockfd, F_GETFL, NULL);
	arg |= O_NONBLOCK;
	fcntl(sockfd, F_SETFL, arg);
	
	// connect
	status = connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
		
	if (status < 0) {
		if (errno == EINPROGRESS) {
			struct timeval tv;
			fd_set writefds;
	
			tv.tv_sec = 10;
			tv.tv_usec = 0;
			
			FD_ZERO(&writefds);
			FD_SET(sockfd, &writefds);
			
			select(sockfd + 1, NULL, &writefds, NULL, &tv);
			
			if (!FD_ISSET(sockfd, &writefds)) {
				cerr << "ERROR: timeout connecting to server" << endl;
				freeaddrinfo(servinfo);
				close(sockfd);
				exit(EXIT_FAILURE);
			}
		}
		else {
			cerr << "ERROR: " << strerror(errno) << endl;
			freeaddrinfo(servinfo);
			close(sockfd);
			exit(EXIT_FAILURE);
		}
	}
	
	arg = fcntl(sockfd, F_GETFL, NULL);
	arg &= (~O_NONBLOCK);
	fcntl(sockfd, F_SETFL, arg);
	
	FILE* filp = fopen(argv[3], "rb");
	if (!filp) {
		cerr << "ERROR: could not open file " << argv[3] << endl;
		freeaddrinfo(servinfo);
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	
	char* buffer = new char[BUFFER_SIZE];
	int num_bytes = fread(buffer, sizeof(char), BUFFER_SIZE, filp);
	
	while (num_bytes > 0) {
		struct timeval tv;
		fd_set writefds;

		tv.tv_sec = 10;
		tv.tv_usec = 0;
		
		send(sockfd, buffer, num_bytes, 0);
		
		FD_ZERO(&writefds);
		FD_SET(sockfd, &writefds);
		
		select(sockfd + 1, NULL, &writefds, NULL, &tv);
		
		if (!FD_ISSET(sockfd, &writefds)) {
			cerr << "ERROR: timeout writing to socket in send" << endl;
			delete buffer;
			fclose(filp);
			freeaddrinfo(servinfo);
			close(sockfd);
			exit(EXIT_FAILURE);
		}
		
		num_bytes = fread(buffer, sizeof(char), BUFFER_SIZE, filp);
	}
	
	delete buffer;
	fclose(filp);
	
	freeaddrinfo(servinfo);
	close(sockfd);
	
	cerr << "SUCCESS!" << endl;
	return 0;
}
