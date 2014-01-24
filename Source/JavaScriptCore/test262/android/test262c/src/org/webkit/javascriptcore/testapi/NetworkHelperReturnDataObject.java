package org.webkit.javascriptcore.test262;

// This is a silly little class to store data needed from returning from evaulating a JavaScript script.
// Because Java/JNI does not allow multiple return values, we need to call into Java to pass more data.
// This class provides a way to store return values on set per-instance basis.
public class NetworkHelperReturnDataObject extends Object
{
	private int serverSocket = 0;
	private short serverPort = 0;
	private long serverUserData = 0;
	private long serverAcceptThread = 0;


	public void setServerSocket(int server_socket)
	{
		serverSocket = server_socket;
	}

	public int getServerSocket()
	{
		return serverSocket;
	}

	public void setServerPort(short server_port)
	{
		serverPort = server_port;
	}

	public short getServerPort()
	{
		return serverPort;
	}

	public void setServerUserData(long server_user_data)
	{
		serverUserData = server_user_data;
	}

	public long getServerUserData()
	{
		return serverUserData;
	}

	public void setServerAcceptThread(long server_accept_thread)
	{
		serverAcceptThread = server_accept_thread;
	}

	public long getServerAcceptThread()
	{
		return serverAcceptThread;
	}


	public void setFields(int server_socket, short server_port, long server_user_data, long server_accept_thread)
	{
		serverSocket = server_socket;
		serverPort = server_port;
		serverUserData = server_user_data;
		serverAcceptThread = server_accept_thread;
	}

}

