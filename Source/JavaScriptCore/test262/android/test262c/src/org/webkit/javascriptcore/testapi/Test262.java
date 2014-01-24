package org.webkit.javascriptcore.test262;

import android.app.Activity;
import android.app.ActivityManager;

import android.app.AlertDialog;
import android.content.DialogInterface;

import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;

import android.content.res.AssetManager;
import android.util.Log;
import android.view.View;
import android.widget.TextView;
import android.widget.Button;
import android.widget.ProgressBar;


import java.io.File;


import net.playcontrol.logger.CLogger;
import net.playcontrol.logger.Logger;



public class Test262 extends Activity
{
	static
	{
		System.loadLibrary("JavaScriptCore");
		System.loadLibrary("Test262");
	}
    
	private TextView runningTestNumberStatusLabel;
	private TextView currentTestNameStatusLabel;
	private TextView totalFailedStatusLabel;
	private TextView logFileLocationLabel;
	private TextView totalTimeLabel;
	private Button runTestButton;
	private ProgressBar testProgressBar;



	private Thread javaScriptThread;
	private boolean javaScriptThreadIsRunning;
	private boolean javaScriptThreadShouldContinueRunning;
	// used by JNI to get value
	public boolean getJavaScriptThreadShouldContinueRunning()
	{
		return javaScriptThreadShouldContinueRunning;
	}

	private org.webkit.javascriptcore.test262.MemoryInfo myMemoryInfo;
	private boolean isAdbEchoingEnabled = true;
	private boolean isMemoryInfoLoggingEnabled = false;


  	public final static native long getNativeLoggerFile();
	public static net.playcontrol.logger.CLogger GetNativeLoggerFileWithClassWrapper()
	{
		long c_ptr = getNativeLoggerFile();
		return (c_ptr == 0) ? null : new net.playcontrol.logger.CLogger(c_ptr, false);
	}
  	public final static native long getNativeLoggerNative();
	public static net.playcontrol.logger.CLogger GetNativeLoggerNativeWithClassWrapper()
	{
		long c_ptr = getNativeLoggerNative();
		return (c_ptr == 0) ? null : new net.playcontrol.logger.CLogger(c_ptr, false);
	}
  	public final static native long getNativeLoggerSocket();
	public static net.playcontrol.logger.CLogger GetNativeLoggerSocketWithClassWrapper()
	{
		long c_ptr = getNativeLoggerSocket();
		return (c_ptr == 0) ? null : new net.playcontrol.logger.CLogger(c_ptr, false);
	}
  

	private NetworkHelper networkHelper;

	public native boolean doInit(AssetManager java_asset_manager);
	public native void doPause();
	public native void doResume();
	public native void doDestroy();
	public native void runTests(AssetManager java_asset_manager);


	// I really should bind LogWrapper instead of the individual pieces since LoggerSocket can be recreated behind the scenes on new connections.
	// This would allow me to cache the LogWrapper instance and not have to keep creating a new instance every time which should help GC.
	// But this isn't really being used right now, so I'll skip it for now.
	private void writeMemoryInfoToLogFile(String log_string)
	{
		if(!isMemoryInfoLoggingEnabled)
		{
			return;
		}
		final String memory_info_string = myMemoryInfo.createMemoryInfoStringForAll();

		CLogger file_logger = GetNativeLoggerFileWithClassWrapper();
		if(file_logger != null)
		{
			Logger.LogEvent(file_logger, 3, "Test262", "MemoryInfo", log_string, "\n", memory_info_string);
		}

		if(isAdbEchoingEnabled)
		{
			CLogger native_logger = GetNativeLoggerNativeWithClassWrapper();
			if(native_logger != null)
			{
				Logger.LogEvent(native_logger, 3, "Test262", "MemoryInfo", log_string, "\n", memory_info_string);
			}
		}
		CLogger socket_logger = GetNativeLoggerSocketWithClassWrapper();
		if(socket_logger != null)
		{
			Logger.LogEvent(socket_logger, 3, "Test262", "MemoryInfo", log_string, "\n", memory_info_string);
		}
	}

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);

		// invoke C-side initialization
		AssetManager java_asset_manager = this.getAssets();
		doInit(java_asset_manager);

		ActivityManager activity_manager = (ActivityManager)this.getSystemService(ACTIVITY_SERVICE);
		myMemoryInfo = new org.webkit.javascriptcore.test262.MemoryInfo(activity_manager);

			
		// get the Progress Bar component from the XML layout
		runTestButton = (Button)findViewById(R.id.submit_button);
		testProgressBar = (ProgressBar)findViewById(R.id.test_progress_bar);
		runningTestNumberStatusLabel = (TextView)findViewById(R.id.runningTestNumberStatusLabel);
		currentTestNameStatusLabel = (TextView)findViewById(R.id.currentTestNameStatusLabel);
		totalFailedStatusLabel = (TextView)findViewById(R.id.totalFailedStatusLabel);
		logFileLocationLabel = (TextView)findViewById(R.id.logFileLocationLabel);
		totalTimeLabel = (TextView)findViewById(R.id.totalTimeLabel);
		testProgressBar.setMax(11000);
		testProgressBar.setProgress(0);
		testProgressBar.setEnabled(false);


		networkHelper = new NetworkHelper(getApplicationContext());
		networkHelper.startServer();

		writeMemoryInfoToLogFile("Memory at the end of onCreate");		
	}


	/** Called when the activity is about to be paused. */
	@Override
	protected void onPause()
	{
		Log.i("Test262", "calling onPause");
		
		doPause();
		// Even though the logging service itself may still be up, disable the advertising because 
		// I'm hitting an Android bug where if the app crashes or is killed via adb (re)install,
		// the old service persists and never gets reaped, even when the can't connect failure
		// should propagate and remove itself. 
		// Additionally, the service stays until the device is rebooted.
		// So something in the OS is not cleaning up correctly.
		networkHelper.unregisterService();
		super.onPause();
	}

	@Override
	protected void onResume()
	{
		Log.i("Test262", "calling onResume");
		
		super.onResume();
		networkHelper.registerService();
		doResume();
	}

	/** Called when the activity is about to be destroyed. */
	@Override
	protected void onDestroy()
	{
		Log.i("Test262", "calling onDestroy");

		abortTests();

		// Try not to tear this down until after the log writing is finished
		networkHelper.stopServer();

		doDestroy();

		super.onDestroy();
		Log.i("Test262", "finished calling onDestroy");		
	}


	@Override
	public void onBackPressed()
	{
		if(javaScriptThreadIsRunning)
		{
			AlertDialog.Builder builder = new AlertDialog.Builder(this);
			builder.setTitle("Exit Test262");
			builder.setMessage("This will abort the current running tests. Are you sure you want to exit?");

			builder.setPositiveButton("Exit", new DialogInterface.OnClickListener()
				{
					@Override
					public void onClick(DialogInterface dialog, int which)
					{
						dialog.dismiss();
						finish();
					}
				}
			);

			builder.setNegativeButton("Stay", new DialogInterface.OnClickListener()
				{
					@Override
					public void onClick(DialogInterface dialog, int which)
					{
						dialog.dismiss();
					}
				}
			);
			AlertDialog alert = builder.create();
			alert.show();
		}
		else
		{
			finish();
		}
	}

	public void abortTests()
	{
		javaScriptThreadShouldContinueRunning = false;
		if(null != javaScriptThread && javaScriptThreadIsRunning)
		{
			// Note: A few of the tests can take a few minutes to run.
			// There isn't a good way to interrupt the Javascript engine, so this will block.
			// This will be noticable on relaunch.
			// thread.stop() will crash the app.
			// I'm a little tempted to call exit().
			try
			{
				javaScriptThread.join();
			}
			catch(InterruptedException e)
			{
				Log.i("Test262", "Failed to join javaScriptThread");
			}
			
			javaScriptThread = null;
		}
	}

    public void myClickHandler(View the_view)
	{
		switch(the_view.getId())
		{
			case R.id.submit_button:
			{
				if(javaScriptThreadIsRunning)
				{
					AlertDialog.Builder builder = new AlertDialog.Builder(this);
					builder.setTitle("Abort running tests?");
					builder.setMessage("This will abort the current running tests. Are you sure you want to abort?");

					builder.setPositiveButton("Abort Tests", new DialogInterface.OnClickListener()
						{
							@Override
							public void onClick(DialogInterface dialog, int which)
							{
								dialog.dismiss();
								runTestButton.setEnabled(false);
								abortTests();
								runTestButton.setText(getApplicationContext().getString(R.string.submit_press));
								runTestButton.setEnabled(true);
								testProgressBar.setEnabled(false);
								
							}
						}
					);

					builder.setNegativeButton("Continue Running", new DialogInterface.OnClickListener()
						{
							@Override
							public void onClick(DialogInterface dialog, int which)
							{
								dialog.dismiss();
							}
						}
					);
					AlertDialog alert = builder.create();
					alert.show();
				}
				else
				{
					// runTestButton.setEnabled(false);
						
					runTestButton.setText("Abort tests");
					startTests();
				}
				break;
			}
			default:
			{
				break;
			}
		}
	} 

	public void startTests()
	{
		testProgressBar.setEnabled(true);
		runningTestNumberStatusLabel.setText("Starting tests...");
		currentTestNameStatusLabel.setText("");
		totalFailedStatusLabel.setText("");
		logFileLocationLabel.setText("");
		totalTimeLabel.setText("");

		Thread thread = new Thread(new Runnable()
		{
			@Override
			public void run()
			{
				AssetManager java_asset_manager = Test262.this.getAssets();
				javaScriptThreadIsRunning = true;				
				runTests(java_asset_manager);
				javaScriptThreadIsRunning = false;	
				javaScriptThread = null;
			}

		});
		javaScriptThreadShouldContinueRunning = true;
		javaScriptThread = thread;
		thread.start();

	}

	public void callbackForAllTestsStarting(final long total_number_of_tests, final long user_data)
	{	
		runOnUiThread(new Runnable()
		{
			
			@Override
			public void run()
			{
				final int max_progess = (int)total_number_of_tests;			
				testProgressBar.setMax(max_progess);
			}
		});
	}
	
	public void callbackForBeginningTest(final String test_file, final long total_number_of_tests, final long current_test_number, final long user_data)
	{
		runOnUiThread(new Runnable()
		{
			@Override
			public void run()
			{
				final int currnet_progess = (int)current_test_number;			
				testProgressBar.setProgress(currnet_progess);
				runningTestNumberStatusLabel.setText("Running test " + current_test_number + " of " + total_number_of_tests);
				currentTestNameStatusLabel.setText(test_file);
			}
		});
	}

	public void callbackForEndingTest(final String test_file, final long total_number_of_tests, final long current_test_number, final long total_number_of_tests_failed, final boolean did_pass, final String exception_string, final String stack_string, final long user_data)
	{
		runOnUiThread(new Runnable()
		{
			@Override
			public void run()
			{
				totalFailedStatusLabel.setText("Total failed: " + total_number_of_tests_failed);
			}
		});
	}

	public void callbackForAllTestsFinished(final long total_number_of_tests, final long number_of_tests_run, final long total_number_of_tests_failed, final double diff_time_in_double_seconds, final long user_data)
	{
		runOnUiThread(new Runnable()
			{
				@Override
				public void run()
				{
					final int currnet_progess = (int)number_of_tests_run;			
					testProgressBar.setProgress(currnet_progess);
					testProgressBar.setEnabled(false);

					logFileLocationLabel.setText(Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "test262_runlog.txt");
					totalTimeLabel.setText("Total execution time: " + diff_time_in_double_seconds + " seconds");
//					runTestButton.setEnabled(true);
					runTestButton.setText(getApplicationContext().getString(R.string.submit_press));
				}
			}
		);
	}


}
