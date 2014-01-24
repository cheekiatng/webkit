package org.webkit.javascriptcore.test262;
import java.io.File;
import android.os.Environment;
import java.io.FileNotFoundException;
import java.io.FileInputStream;
import java.net.URL;
import java.net.MalformedURLException;

import android.util.Log;


public class NetworkHelper
{
	private android.content.Context applicationContext;
	// This is only used to get the service name
	private NsdHelper zeroconfHelper;
	private int serverSocket; // this is the C socket which happens to also be an int
	private short serverPort; // this is the C socket port (16-bit integer)
	private long serverUserData; // this is the pointer to the C struct instance userdata
	private long serverAcceptThread; // this is the pointer to the C thread data structure
	private boolean isServerStarted = false;

	public native boolean startServerNative(NetworkHelperReturnDataObject return_data_object);
	public native void stopServerNative(int server_socket, short server_port, long server_user_data, long server_accept_thread);
	
	/*
	private void setZeroconfHelper(NsdHelper zeroconf_helper)
	{
		zeroconfHelper = zeroconf_helper;
	}
	*/

	NetworkHelper(android.content.Context application_context)
	{
		applicationContext = application_context;
	}

	void startServer()
	{
		if(isServerStarted)
		{
			return;
		}
		
		NetworkHelperReturnDataObject return_data_object = new NetworkHelperReturnDataObject();
		
		startServerNative(return_data_object);
		// The return_data_object was filled during the previous function call.
		serverSocket = return_data_object.getServerSocket();
		serverPort = return_data_object.getServerPort();
		serverUserData = return_data_object.getServerUserData();
		serverAcceptThread = return_data_object.getServerAcceptThread();

			Log.i("startServer","serverAcceptThread " + serverAcceptThread);
		zeroconfHelper = new NsdHelper(applicationContext, "_test262logging._tcp.");
        zeroconfHelper.initializeNsd();
		zeroconfHelper.registerService(serverPort);
		
		isServerStarted = true;
	}

	void stopServer()
	{
		if(!isServerStarted)
		{
			return;
		}
		zeroconfHelper.tearDown();
			Log.i("stopServer","serverAcceptThread " + serverAcceptThread);
		
		stopServerNative(serverSocket, serverPort, serverUserData, serverAcceptThread);
		isServerStarted = false;
	}
	
	void registerService()
	{
		if(!isServerStarted)
		{
			zeroconfHelper.registerService(serverPort);		
		}
	}

	void unregisterService()
	{
		// Even though the logging service itself may still be up, disable the advertising because 
		// I'm hitting an Android bug where if the app crashes or is killed via adb (re)install,
		// the old service persists and never gets reaped, even when the can't connect failure
		// should propagate and remove itself. 
		// Additionally, the service stays until the device is rebooted.
		// So something in the OS is not cleaning up correctly.
		if(isServerStarted)
		{
			zeroconfHelper.tearDown();		
		}
	}

	void uploadFileCallbackFromNative(String host_address_string, short http_server_port)
	{
		URL url_to_connect_to;		
		String service_name = zeroconfHelper.getServiceName();
		
		try
		{
			url_to_connect_to = new URL("http", host_address_string, http_server_port, service_name + "-test262_runlog.txt");

		}
		catch(MalformedURLException ex)
		{
//			Log.i("HttpFileUpload","URL Malformatted");
			return;
		}


		File log_file = new File(Environment.getExternalStorageDirectory(), "test262_runlog.txt");

		// Set your file path here
		//FileInputStream fstrm = new FileInputStream(log_file);

		// Set your server page url (and the file title/description)
		//HttpFileUpload hfu = new HttpFileUpload("http://www.myurl.com/fileup.aspx", "my file title","my file description");
		HttpFileUpload hfu = new HttpFileUpload(url_to_connect_to);

		//	hfu.Send_Now(fstrm);
		hfu.Send_Now(log_file);
	}
}

