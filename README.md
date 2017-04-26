# CS118 Project 1

Steven Xu, 604450388

## High-level design of server and client

Both the server and client are implemented using BSD sockets.
Both applications deal with improper command-line arguments.
Both applications handle connection and transmission errors by setting
a timeout at 10 seconds.

### server.cpp

server.cpp compiles into a server binary that accepts only two command-line
arguments. This server listens for any incoming connections and saves any
files received from a client and stores them in the specified directory.

$ ./server `<PORT>` `<FILE-DIR>`

The first argument is a port number to listen to and the second
argument is a file directory where to save any files that the server receives
while listening and accepting any incoming connections.

Through multithreading and the use of the thread class, the server can accept
and process multiple connections at the same time. This is done by having one
main thread listening for any incoming connection requests. Once the main
thread receives and accepts a new connection, it will create a child thread
that actually performs the work of storing and saving the incoming data over
the connection into a file in the specified directory.

### client.cpp

client.cpp compiles into a client binary that accepts only three command-line
arguments. This client attempts to make a connection to a specified server and
port, transfer a specified file over the connection to the server, and close
the connection immediately afterwards.

$ ./client `<HOSTNAME-OR-IP>` `<PORT>` `<FILENAME>`

The first argument is the hostname or IP address of the server to connect to.
The second argument is the port number of the server to connect to. The third
argument is the path of the file that the client is trying to transfer to the
server.

## Problems I ran into

My two biggest problems were getting vagrant to work on my Windows machine and
implementing timeout on the client side for "not being able to send more data
to the server (not being able to write to send buffer)".

For vagrant, it turns out that I needed to enter the BIOS of my computer and
enable virtualization extensions (credits to Ramiya Satish of piazza post 58).

For the timeout on the send buffer on client, I originally was using sendfile()
system call in client.cpp as this higher-level abstraction made sending the
data easier. However, I was unable to detect any blocks on writing to the send
buffer (i.e. sendfile simply skips writing to a buffer and writes directly to
the socket) so I had to rewrite that portion of client.cpp using the send()
system call.

Other than those setbacks, implementing the rest of the project was simply a
matter of taking the time understanding the appropriate sections in Beej's
Guide to Network Programming.

## References

To understand BSD sockets, I used Beej's Guide to Network Programming.
For any other libraries I used, I referenced documentation on
cplusplus.com, man7.org, and linux.die.net.

## Extra Credit

For docker, I hardcoded the port to be 2017 and the directory to be ./save.
Files should be saved in the same directory as the Dockerfile.

In bash, run the following commands:

$ sudo docker build -t server .

$ sudo docker run -d -v /vagrant:/save -p 2017:2017 server

You can now connect to the server. One such example:

$ telnet localhost 2017
