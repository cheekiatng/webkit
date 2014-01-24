#include "SocketServer.h"

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

#include "SimpleThread.h"

//#include <sys/wait.h>
//#include <signal.h>

//#define PORT "3490"  // the port users will be connecting to
#define PORT "0"  // the port users will be connecting to

//#define BACKLOG 10     // how many pending connections queue will hold
#define BACKLOG 4     // how many pending connections queue will hold


static void (*s_uploadFileCallback)(int accepted_socket, uint16_t http_server_port, void* user_data) = NULL;
static void* s_uploadFileCallbackUserData = NULL;

void SocketServer_SetUploadFileCallback(void (*upload_file_callback)(int accepted_socket, uint16_t http_server_port, void* user_data), void* user_data)
{
	s_uploadFileCallback = upload_file_callback;
	s_uploadFileCallbackUserData = user_data;
}

static void (*s_openLogStreamCallback)(int accepted_socket, void* log_wrapper, void* user_data) = NULL;
static void* s_opaquelogWrapper = NULL;
static void* s_openLogStreamCallbackUserData = NULL;

void SocketServer_SetOpenLogStreamCallback(void (*open_log_stream_callback)(int accepted_socket, void* log_wrapper, void* user_data), void* log_wrapper, void* user_data)
{
	s_openLogStreamCallback = open_log_stream_callback;
	s_opaquelogWrapper = log_wrapper;
	s_openLogStreamCallbackUserData = user_data;
}



/*
void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

*/
// get sockaddr, IPv4 or IPv6:
static void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
/*
static uint16_t get_in_port(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return (((struct sockaddr_in*)sa)->sin_port);
    }

    return (((struct sockaddr_in6*)sa)->sin6_port);
}
*/

static uint16_t SocketServer_GetPortFromSocket(int sock_fd)
{
	struct sockaddr_in sock_addr_in;
	struct sockaddr_in* sock_addr_in_ptr = &sock_addr_in;
	struct sockaddr* sock_addr = (struct sockaddr*)sock_addr_in_ptr;
	socklen_t size_of_sock_addr;
	/* We must put the length in a variable.              */
	size_of_sock_addr = sizeof(sock_addr_in);
	/* Ask getsockname to fill in this socket's local     */
	/* address.                                           */
	if (getsockname(sock_fd, sock_addr, &size_of_sock_addr) == -1) {
		perror("getsockname() failed");
		return 0;
	}

	/* Print it. The IP address is often zero beacuase    */
	/* sockets are seldom bound to a specific local       */
	/* interface.                                         */
//	printf("Local IP address is: %s\n", inet_ntoa(sa.sin_addr));
//	printf("Local port is: %d\n", (int) ntohs(sa.sin_port));

	return ntohs(sock_addr_in.sin_port);
}


// Returns 0 on sucess, other numbers for error
// Must pass in a socket_fd and out_port or things will crash.
int SocketServer_CreateSocketAndListen(int* out_socket_fd, uint16_t* out_port)
{
	int sock_fd;  // listen on sock_fd
	struct addrinfo hints, *servinfo, *p;
	//    struct sigaction sa;
	int yes=1;
	int rv;
	uint16_t chosen_port_host;

	memset(&hints, 0, sizeof hints);

//	hints.ai_family = AF_INET;
//	hints.ai_family = AF_INET6;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sock_fd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			return(2);
		}

		if (bind(sock_fd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sock_fd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 3;
	}

/* // doesn't work
	chosen_port_host = ntohs(get_in_port(p->ai_addr));
*/
	chosen_port_host = SocketServer_GetPortFromSocket(sock_fd);
	*out_port = chosen_port_host;

	*out_socket_fd = sock_fd;

    freeaddrinfo(servinfo); // all done with this structure

    if (listen(sock_fd, BACKLOG) == -1) {
        perror("listen");
        return 4;
    }
	return 0;
}

// Intended to be run in a thread
int SocketServer_RunAcceptLoop(void* user_data)
{
	struct SocketServer_UserData* socket_userdata = (struct SocketServer_UserData*)user_data;
	int sock_fd = socket_userdata->serverSocket;

	struct sockaddr_storage their_addr; // connector's address information
	int new_fd;  // new connection on new_fd
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];

/*
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
*/
    printf("server: waiting for connections...\n");


    while(1 == socket_userdata->shouldKeepRunning)
	{  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sock_fd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);


		// I really only need 2 bytes for the port/command
		const size_t MAX_RECV_BUFFER_SIZE = 2;
		char recv_buffer[MAX_RECV_BUFFER_SIZE];
		ssize_t num_bytes = recv(new_fd, recv_buffer, MAX_RECV_BUFFER_SIZE, 0);
		if(0 == num_bytes)
		{
			printf("error with recv (no data received), errno:(%d) %s", errno, strerror(errno));
			// Not sure to stop or continue?
			continue;
		}
		else if(num_bytes < 0)
		{
			printf("error with recv, errno:(%d) %s", errno, strerror(errno));
			// Not sure to stop or continue?
			continue;
		}

		uint16_t command_or_port_received_network = *(uint16_t*)&recv_buffer[0];
		uint16_t command_or_port_received_host = ntohs(command_or_port_received_network);

		if(0 == command_or_port_received_host)
		{
			printf("LogStream command received");
			if(NULL != s_openLogStreamCallback)
			{
				// disable SIGPIPE errors (crashes) on writing to a dead socket. I need this since I'm blindly throwing stuff at Logger.
				// decided to move to platform specific code:
				// http://stackoverflow.com/questions/108183/how-to-prevent-sigpipes-or-handle-them-properly
//				int n = 1;
//				setsockopt(new_fd, SOL_SOCKET, SO_NOSIGPIPE, &n, sizeof(n));
				s_openLogStreamCallback(new_fd, s_opaquelogWrapper, s_openLogStreamCallbackUserData);
			}
			else
			{
				printf("OpenLogStreamCallback has not been set");
			}
			// TODO: probably should continue listening to the socket to listen for close messages or special commands
		}
		else
		{
			if(NULL != s_uploadFileCallback)
			{
				s_uploadFileCallback(new_fd, command_or_port_received_host, s_uploadFileCallbackUserData);
			}
			else
			{
				printf("UploadFileCallback has not been set");
			}
			// assumption is that the uploadFileCallback is done with the socket so I can close it for all platforms.
			close(new_fd);
		}




/*
        if (!fork()) { // this is the child process
            close(sock_fd); // child doesn't need the listener
            if (send(new_fd, "Hello, world!", 13, 0) == -1)
                perror("send");
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
*/
    }

    return 0;
}

SimpleThread* SocketServer_RunAcceptLoopInThread(struct SocketServer_UserData* user_data)
{
	return SimpleThread_CreateThread(SocketServer_RunAcceptLoop, user_data);
}
