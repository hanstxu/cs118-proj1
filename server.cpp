#include <string>
#include <thread>
#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {
	if (argc != 3) {
		cerr << "Error: You must give two arguments when initializing the server." << endl;
		exit(EXIT_FAILURE);
	}
	
	int port_num = stoi(argv[1]);
	string directory_name = argv[2];
	
	cout << port_num << endl;
	cout << directory_name << endl;
}
