/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// Adapted from Android Network Service Discovery example

package org.webkit.javascriptcore.test262;


import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.InputStream;
import java.io.DataInputStream;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

import java.io.File;
import android.os.Environment;
import java.io.FileNotFoundException;
import java.io.FileInputStream;
import java.net.URL;
import java.net.MalformedURLException;


public class LoggingConnection {

    private Handler mUpdateHandler;
    private LoggingServer mLoggingServer;
//    private LoggingClient mLoggingClient;

    private static final String TAG = "LoggingConnection";

    private Socket mSocket;
    private int mPort = -1;

    public LoggingConnection(Handler handler) {
        mUpdateHandler = handler;
        mLoggingServer = new LoggingServer(handler);
    }

    public void tearDown() {
		if(null != mLoggingServer)
		{
			mLoggingServer.tearDown();
		}
		/*
		if(null != mLoggingClient)
		{
			mLoggingClient.tearDown();
		}
		*/
    }

/*
    public void connectToServer(InetAddress address, int port) {
        mLoggingClient = new LoggingClient(address, port);
    }

    public void sendMessage(String msg) {
        if (mLoggingClient != null) {
            mLoggingClient.sendMessage(msg);
        }
    }
*/    
    public int getLocalPort() {
        return mPort;
    }
    
    public void setLocalPort(int port) {
        mPort = port;
    }
    

/*
    public synchronized void updateMessages(String msg, boolean local) {
        Log.e(TAG, "Updating message: " + msg);

        if (local) {
            msg = "me: " + msg;
        } else {
            msg = "them: " + msg;
        }

        Bundle messageBundle = new Bundle();
        messageBundle.putString("msg", msg);

        Message message = new Message();
        message.setData(messageBundle);
        mUpdateHandler.sendMessage(message);

    }
*/
    private synchronized void setSocket(Socket socket) {
        Log.d(TAG, "setSocket being called.");
        if (socket == null) {
            Log.d(TAG, "Setting a null socket.");
        }
        if (mSocket != null) {
            if (mSocket.isConnected()) {
                try {
                    mSocket.close();
                } catch (IOException e) {
                    // TODO(alexlucas): Auto-generated catch block
                    e.printStackTrace();
                }
            }
        }
        mSocket = socket;
    }

    private Socket getSocket() {
        return mSocket;
    }

    private class LoggingServer {
        ServerSocket mServerSocket = null;
        Thread mThread = null;

        public LoggingServer(Handler handler) {
            mThread = new Thread(new ServerThread());
            mThread.start();
                        Log.d(TAG, "LoggingServer mThread.start() done");
			
        }

        public void tearDown() {
                        Log.d(TAG, "tearDown calling interrupt");
            mThread.interrupt();
                        Log.d(TAG, "tearDown passed interrupt");
				if(mServerSocket != null)
			{
       
				try {
            	    mServerSocket.close();
            	} catch (IOException ioe) {
            	    Log.e(TAG, "Error when closing server socket.");
            	}
			}
        }

		public void UploadFile(InetAddress http_server_address, int http_server_port)
		{
			String host_address_string = http_server_address.getHostAddress();
			URL url_to_connect_to;
			try
			{
				url_to_connect_to = new URL("http", host_address_string, http_server_port, "test262_runlog.txt");

			}
			catch(MalformedURLException ex)
			{
                    Log.i("HttpFileUpload","URL Malformatted");
					return;
			}

			try
			{


				File log_file = new File(Environment.getExternalStorageDirectory(), "test262_runlog.txt");
				
				// Set your file path here
				FileInputStream fstrm = new FileInputStream(log_file);

				// Set your server page url (and the file title/description)
				//HttpFileUpload hfu = new HttpFileUpload("http://www.myurl.com/fileup.aspx", "my file title","my file description");
				HttpFileUpload hfu = new HttpFileUpload(url_to_connect_to);

				hfu.Send_Now(fstrm);

			}
			catch (FileNotFoundException e) 
			{
				// Error: File not found
			}
		}

        class ServerThread implements Runnable {

            @Override
            public void run() {

                try {
                    // Since discovery will happen via Nsd, we don't need to care which port is
                    // used.  Just grab an available one  and advertise it via Nsd.
                    mServerSocket = new ServerSocket(0);
                    setLocalPort(mServerSocket.getLocalPort());
                    
                    while (!Thread.currentThread().isInterrupted()) {
                        Log.d(TAG, "ServerSocket Created, awaiting connection");
                        setSocket(mServerSocket.accept());
                        Log.d(TAG, "Connected.");
						InputStream is = mSocket.getInputStream();
						DataInputStream dis = new DataInputStream(is);
						char port_value = dis.readChar();
						int int_port_value = (int)port_value;
                        Log.d(TAG, "Read port value is: " + int_port_value);

						UploadFile(mSocket.getInetAddress(), int_port_value);


						/*
                        if (mLoggingClient == null) {
                            int port = mSocket.getPort();
                            InetAddress address = mSocket.getInetAddress();
                            connectToServer(address, port);
                        }
						*/
                    }
                } catch (IOException e) {
                    Log.e(TAG, "Error creating ServerSocket: ", e);
                    e.printStackTrace();
                }
                        Log.d(TAG, "ending serverthread");
            }

			
        }
    }

/*
    private class LoggingClient {

        private InetAddress mAddress;
        private int PORT;

        private final String CLIENT_TAG = "LoggingClient";

        private Thread mSendThread;
        private Thread mRecThread;

        public LoggingClient(InetAddress address, int port) {

            Log.d(CLIENT_TAG, "Creating chatClient");
            this.mAddress = address;
            this.PORT = port;

            mSendThread = new Thread(new SendingThread());
            mSendThread.start();
        }

        class SendingThread implements Runnable {

            BlockingQueue<String> mMessageQueue;
            private int QUEUE_CAPACITY = 10;

            public SendingThread() {
                mMessageQueue = new ArrayBlockingQueue<String>(QUEUE_CAPACITY);
            }

            @Override
            public void run() {
                try {
                    if (getSocket() == null) {
                        setSocket(new Socket(mAddress, PORT));
                        Log.d(CLIENT_TAG, "Client-side socket initialized.");

                    } else {
                        Log.d(CLIENT_TAG, "Socket already initialized. skipping!");
                    }

                    mRecThread = new Thread(new ReceivingThread());
                    mRecThread.start();

                } catch (UnknownHostException e) {
                    Log.d(CLIENT_TAG, "Initializing socket failed, UHE", e);
                } catch (IOException e) {
                    Log.d(CLIENT_TAG, "Initializing socket failed, IOE.", e);
                }

                while (true) {
                    try {
                        String msg = mMessageQueue.take();
                        sendMessage(msg);
                    } catch (InterruptedException ie) {
                        Log.d(CLIENT_TAG, "Message sending loop interrupted, exiting");
                    }
                }
            }
        }

        class ReceivingThread implements Runnable {

            @Override
            public void run() {

                BufferedReader input;
                try {
                    input = new BufferedReader(new InputStreamReader(
                            mSocket.getInputStream()));
                    while (!Thread.currentThread().isInterrupted()) {

                        String messageStr = null;
                        messageStr = input.readLine();
                        if (messageStr != null) {
                            Log.d(CLIENT_TAG, "Read from the stream: " + messageStr);
                            updateMessages(messageStr, false);
                        } else {
                            Log.d(CLIENT_TAG, "The nulls! The nulls!");
                            break;
                        }
                    }
                    input.close();

                } catch (IOException e) {
                    Log.e(CLIENT_TAG, "Server loop error: ", e);
                }
            }
        }

        public void tearDown() {
            try {
                getSocket().close();
            } catch (IOException ioe) {
                Log.e(CLIENT_TAG, "Error when closing server socket.");
            }
        }

        public void sendMessage(String msg) {
            try {
                Socket socket = getSocket();
                if (socket == null) {
                    Log.d(CLIENT_TAG, "Socket is null, wtf?");
                } else if (socket.getOutputStream() == null) {
                    Log.d(CLIENT_TAG, "Socket output stream is null, wtf?");
                }

                PrintWriter out = new PrintWriter(
                        new BufferedWriter(
                                new OutputStreamWriter(getSocket().getOutputStream())), true);
                out.println(msg);
                out.flush();
                updateMessages(msg, true);
            } catch (UnknownHostException e) {
                Log.d(CLIENT_TAG, "Unknown Host", e);
            } catch (IOException e) {
                Log.d(CLIENT_TAG, "I/O Exception", e);
            } catch (Exception e) {
                Log.d(CLIENT_TAG, "Error3", e);
            }
            Log.d(CLIENT_TAG, "Client sent message: " + msg);
        }
    }
	*/
}
