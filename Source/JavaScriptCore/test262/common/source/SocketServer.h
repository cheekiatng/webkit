//
//  SocketServer.h
//  Test262
//
//  Created by Eric Wing on 1/1/14.
//  Copyright (c) 2014 JavaScriptCore. All rights reserved.
//

#ifndef Test262_SocketServer_h
#define Test262_SocketServer_h

#include <stdint.h>
#include "SimpleThread.h"

struct SocketServer_UserData
{
	int serverSocket;
	int shouldKeepRunning;
};

int SocketServer_CreateSocketAndListen(int* out_socket_fd, uint16_t* out_port);
int SocketServer_RunAcceptLoop(void* user_data);
SimpleThread* SocketServer_RunAcceptLoopInThread(struct SocketServer_UserData* user_data);

// Function pointer used to allow platform specific code to hook into generic C socket server code, i.e. call NSURLSession.
// It is expected you have only one function you set once for the entire life of the program
void SocketServer_SetUploadFileCallback(void (*upload_file_callback)(int accepted_socket, uint16_t http_server_port, void* user_data), void* user_data);

void SocketServer_SetOpenLogStreamCallback(void (*open_log_stream_callback)(int accepted_socket, void* log_wrapper, void* user_data), void* log_wrapper, void* user_data);

#endif
