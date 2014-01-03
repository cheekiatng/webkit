
#include "Logger.h"

#include <string.h> /* for memcpy */
#include <stdlib.h> /* for exit */

#include <netdb.h>

/* inet_aton needs socket.h, types.h, in.h, inet.h */
/* This order (listed in inet_aton man page) seems to matter, or 
 * I get compiler warnings about incompatible types in arg2 of inet_aton)
 */
#include <sys/types.h>
/* for socket() */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
/* #include <sys/un.h> */
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

int g_Socket_For_Talk;

int main(int argc, char* argv[])
{
	Logger* logger;
	LoggerVersion compiled;
	LoggerVersion linked;
	unsigned int some_priority = 3;
	int i;
	int ret_val;

	logger = Logger_CreateWithName("MyLog.log");
	if(NULL == logger)
	{
		fprintf(stderr, "Error: Logger creation failed, quiting program.\n");
		return 0;
	}

	LOGGER_GET_COMPILED_VERSION(&compiled);
	Logger_GetLinkedVersion(&linked);
	
	/* Setting to 1 will print everything. */
	Logger_SetThresholdPriority(logger, 1);
	
	/* Will evaluate Pri=0 to what is set here. */
	Logger_SetDefaultPriority(logger, 5);

	/* Turn on echoing */
	Logger_EchoOn(logger);

	/* Set echoing to stderr */
	Logger_SetEchoToStderr(logger);
	Logger_EnableAutoFlush(logger);

	/* Turn on segmenting to an extreme value for testing. */
//	Logger_SetMinSegmentSize(logger, 1);

//	Logger_SetSegmentFormatString(logger, ".%03d");


	Logger_LogEvent(logger, 0, "Initialization", "main.c", "We are compiled against Logger version %d.%d.%d", compiled.major, compiled.minor, compiled.patch);

	Logger_LogEvent(logger, 0, "Initialization", "main.c", "We are linked against Logger version %d.%d.%d", linked.major, linked.minor, linked.patch);


	
//#define IPADDRESS "192.168.0.5"
#define IPADDRESS "localhost"
unsigned short SERVERPORT=12345;
   int sockfd;
/* 
   struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
	*/
    struct sockaddr_in servname;
    struct hostent *servent;
/*
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(IPADDRESS, htons(SERVERPORT), &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }




    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    }
 */
    /* Get server addr and connect */
    if (NULL==(servent=gethostbyname(IPADDRESS))) {
        perror("gethostbyname");
        exit(2);
    } else  if (AF_INET!=servent->h_addrtype) {
        fprintf(stderr,"gethostbyname: not an AF_INET address\n");
        exit(2);
    }

    
    if (-1==(sockfd=socket(AF_INET, SOCK_STREAM, 0))) {
        perror("socket");
        exit(2);
    }
	  servname.sin_family=AF_INET;
    servname.sin_port=htons(SERVERPORT);
    bcopy(servent->h_addr, &(servname.sin_addr.s_addr), servent->h_length);
 
    if (-1==connect(sockfd, (struct sockaddr *)&servname, sizeof(servname))) {
        fprintf(stderr, 
                "connect: connection to %s[%s], port %d failed: %s\n",
                IPADDRESS, inet_ntoa(servname.sin_addr), SERVERPORT, strerror(errno));
        exit(2);
    }
     



#if 0
	FILE* socket_file = fdopen(sockfd, "w");

	fputs("Testing connection", socket_file);	
/*
    if ((numbytes = sendto(sockfd, "Testing connection", strlen("Testing connection"), 0,
             p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }
*/


	if(socket_file == NULL)
	{
		ret_val = Logger_LogEvent(logger, 0, "Initialization", "socket", "Failed to create socket");

	}
	else
	{
		ret_val = Logger_LogEvent(logger, 0, "Initialization", "socket", "create socket: %d", sockfd);

	}

	if(ret_val < 0)
	{
		fprintf(stderr, "Error: Initializing: %s\n", strerror( errno ));
	}

	// Need API?
	logger->echoHandle = socket_file;
#endif
	Logger_SetSocket(logger, sockfd);



	for(i=0; i<101; i++)
	{
		ret_val = Logger_LogEvent(logger, some_priority, "Debug", "main.c", "Testing a message with priority=%d", some_priority);

		if(ret_val < 0)
		{
			fprintf(stderr, "Error: in logger, aborting: %s\n", strerror( errno ));
			exit(1);
		}



//		usleep(200*1000);
		sleep(1);
	}

	ret_val = Logger_LogEventNoFormat(logger, 0, "Shutdown", "main.c", "Reached end of logging test.");
		if(ret_val < 0)
		{
			fprintf(stderr, "Error: in logger, aborting: %s\n", strerror( errno ));
			exit(2);
		}


	Logger_Free(logger);

	return 0;
}

