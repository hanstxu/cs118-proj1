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
using namespace std;

int main(int argc, char* argv[]) {
	if (argc != 4) {
		cerr << "Error: You must give three arguments when initializing the server.\n";
		exit(EXIT_FAILURE);
	}
	
	int port_num = 0;
	
	try {
		port_num = stoi(argv[2]);
	}
	catch(const invalid_argument& ia) {
		cerr << "Error: invalid argument - " << ia.what() << '\n';
		exit(EXIT_FAILURE);
	}
	
	if (port_num < 1023 || port_num > 65535) {
		cerr << "Error: You must choose a port number between 1024 and 65535.\n";
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
		cerr << "Error: hostname or ip address is invalid\n";
		exit(EXIT_FAILURE);
	}
	
	// make a socket
	int sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (sockfd < 0) {
		cerr << "Error: unable to make a socket" << endl;
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
	
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			
			FD_ZERO(&writefds);
			FD_SET(sockfd, &writefds);
			
			select(sockfd + 1, NULL, &writefds, NULL, &tv);
			
			if (!FD_ISSET(sockfd, &writefds)) {
				cerr << "Error: timeout connecting to server" << endl;
				freeaddrinfo(servinfo);
				close(sockfd);
				exit(EXIT_FAILURE);
			}
		}
		else {
			cerr << "Error: " << strerror(errno) << endl;
			freeaddrinfo(servinfo);
			close(sockfd);
			exit(EXIT_FAILURE);
		}
	}
	
	int fd = open(argv[3], O_RDONLY);
	if (fd <0) {
		cerr << "Error: couldn't open file" << endl;
		freeaddrinfo(servinfo);
		close(fd);
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	
	struct stat st;
	fstat(fd, &st);
	sendfile(sockfd, fd, NULL, st.st_size);
	close(fd);
	
	freeaddrinfo(servinfo);
	close(sockfd);
	
	return 0;
}
