#include <string>
#include <thread>
#include <iostream>
#include <sys/types.h>	// socket libraries
#include <sys/socket.h>	// socket libraries
#include <netdb.h>		// socket libraries
#include <string.h>		// for memset
#include <signal.h>		// for signal handling
#include <fstream>		// for file handling
#include <thread>		// for multi-threading
#include <vector>		// for an arbitrary number of threads
using namespace std;

#define BUFFER_SIZE 1024

void sig_handler(int sig_num) {
	if (sig_num == SIGQUIT)
		cout << "Received SIGQUIT.\n";
	else if (sig_num == SIGTERM)
		cout << "Received SIGTERM.\n";
	exit(0);
}

void write_file(string dir, int count, int read_fd) {
	char* buffer = (char*)malloc(BUFFER_SIZE * sizeof(char));
	memset(buffer, 0, BUFFER_SIZE * sizeof(char));
	
	ofstream file;
	file.open(dir + "/" + to_string(count), ios::out | ios::binary);
	
	int size = recv(read_fd, buffer, BUFFER_SIZE, 0);
	
	while (size > 0) {
		file.write(buffer, size);
		size = recv(read_fd, buffer, BUFFER_SIZE, 0);
	}

	file.close();
	free(buffer);
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		cerr << "Error: You must give two arguments when initializing the server.\n";
		exit(EXIT_FAILURE);
	}
	
	int port_num = stoi(argv[1]);
	
	if (port_num < 1024 || port_num > 65535) {
		cerr << "Error: You must choose a port number between 1024 and 65535.\n";
		exit(EXIT_FAILURE);
	}
	
	string directory = argv[2];
	
	struct sigaction new_signal;
	new_signal.sa_handler = sig_handler;
	
	sigaction(SIGQUIT, &new_signal, NULL);
	sigaction(SIGTERM, &new_signal, NULL);
	
	int status;
	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	struct addrinfo hints, *res;
	int sockfd, new_fd;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;		// don't care about IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;	// TCP stream sockets
	hints.ai_flags = AI_PASSIVE;		// fill the IP in for me
	
	if ((status = getaddrinfo(NULL, argv[1], &hints, &res)) != 0) {
		cerr << "Error: getaddrinfo error - " << gai_strerror(status) << endl;
		exit(EXIT_FAILURE);
	}
	
	// make a socket, bind it, listen on it
	
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	
	if (sockfd < 0) {
		cerr << "Error: Cannot open socket" << endl;
		exit(EXIT_FAILURE);
	}
	
	if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        cerr << "Error: bind didn't work" << endl;
        exit(EXIT_FAILURE);
	}
	listen(sockfd, 1);
	
	addr_size = sizeof their_addr;
	vector<thread> threads;
	int num_connections = 0;
	
	new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &addr_size);
	
	while (new_fd != -1) {
		threads.push_back(thread(write_file, directory,  ++num_connections, new_fd));
		new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &addr_size);
	}
	
	for (size_t i = 0; i < threads.size(); i++) {
		threads[i].join();
	}
	
	freeaddrinfo(res);		// free the linked list
}