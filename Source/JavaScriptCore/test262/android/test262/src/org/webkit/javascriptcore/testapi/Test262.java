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


import android.view.KeyEvent;
import android.view.View.OnKeyListener;

import java.io.FileNotFoundException;
import java.io.IOException;

import java.util.Arrays;
import java.util.ArrayList;
import java.io.InputStream;
//import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;
//import java.io.Reader;
//import java.io.OutputStreamWriter;
//import java.io.FileOutputStream;
import java.io.File;
import java.io.BufferedWriter;
import java.io.FileWriter;

//import java.text.DateFormat;
import java.util.Date;
import java.text.SimpleDateFormat;



public class Test262 extends Activity
{
	static
	{
		System.loadLibrary("JavaScriptCore");
		System.loadLibrary("Test262");
	}
    
	private TextView resultStatusLabel;
	private Button runTestButton;
	private ProgressBar testProgressBar;


	private String testHarnessString;
	private String[] testFileList;
	private int numberOfTests = 0;
	private int numberOfFailedTests = 0;
	
	private long millisecondsStartBenchmark;

	private BufferedWriter outputStreamWriter;
//	private OutputStreamWriter outputStreamWriter;
	private File logFile;
	private FileWriter logFileWriter;

	private Thread javaScriptThread;
	private boolean javaScriptThreadIsRunning;
	private boolean javaScriptThreadShouldContinueRunning;

	private org.webkit.javascriptcore.test262.MemoryInfo myMemoryInfo;
	private boolean isAdbEchoingEnabled = true;
	private boolean isMemoryInfoLoggingEnabled = true;


	private Handler networkLoggingHandler;
	private NsdHelper zeroconfHelper;
	private LoggingConnection loggingConnection;

	public native boolean doInit(AssetManager java_asset_manager);
	public native void doPause();
	public native void doResume();
	public native void doDestroy();
	public native boolean evaluateScript(String script_string, String file_name, ReturnDataObject return_data_object);

	private boolean openLogFile()
	{
//		OutputStreamWriter output_stream_writer = null;
		BufferedWriter output_stream_writer = null;
		String full_path = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "test262_runlog.txt";
//		File out_file = new File(full_path);
		File out_file = new File(Environment.getExternalStorageDirectory(), "test262_runlog.txt");
logFile = out_file;

		try
		{
//		out_file.createNewFile();
			
			logFileWriter = new FileWriter(out_file);
			output_stream_writer = new BufferedWriter(logFileWriter);
			//output_stream_writer = new OutputStreamWriter(new FileOutputStream(out_file));
//		output_stream_writer = new OutputStreamWriter(openFileOutput("test262_runlog.txt", MODE_WORLD_READABLE));

		}
		catch(Exception e)
		{
			e.printStackTrace(); 
			return false;
		}
		outputStreamWriter = output_stream_writer;
		Log.i("Test262", "entered openLogFile " + outputStreamWriter);
		
		return true;
	}
	
	private void closeLogFile()
	{
		if(outputStreamWriter != null)
		{
			try
			{
				outputStreamWriter.flush();
				outputStreamWriter.close();
			}
			catch(IOException e)
			{
				e.printStackTrace(); 
			}

			outputStreamWriter = null;

			// Android Nexus USB MTP bug? Need to force refresh?
//					MediaScannerConnection.scanFile(this, new String[] { Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "test262_runlog.txt" }, null, null);
			
		}
	}
	
	private void writeToLogFile(String subkeyword, String log_string)
	{
		String time_stamp = generateTimeStamp();
		String space_padding = "    ";		
		String log_header = time_stamp + space_padding + "Test262:" + space_padding + subkeyword + ":" + space_padding + "PRI=3\n";
		String log_end = "\n^@END:$\n";

		if(outputStreamWriter != null)
		{

			try
			{
				outputStreamWriter.write(log_header);
				outputStreamWriter.write(log_string);
				outputStreamWriter.write(log_end);
				outputStreamWriter.write("\n");
			}
			catch(IOException e)
			{
				e.printStackTrace(); 
			}
		}

		if(isAdbEchoingEnabled)
		{
			Log.i("Test262", log_header);
			Log.i("Test262", log_string);
			Log.i("Test262", log_end);
		}
		loggingConnection.writeToSocketDataOutputStream(log_header);
		loggingConnection.writeToSocketDataOutputStream(log_string);
		loggingConnection.writeToSocketDataOutputStream(log_end);
		loggingConnection.writeToSocketDataOutputStream("\n");
	}

	private void writeMemoryInfoToLogFile(String log_string)
	{
		if(!isMemoryInfoLoggingEnabled)
		{
			return;
		}
		String time_stamp = generateTimeStamp();
		String space_padding = "    ";
		String log_header = time_stamp + space_padding + "Test262:" + space_padding + "MemoryInfo:" + space_padding + "PRI=3\n";
		// memoryinfo has a trailing \n, so don't need one for end
		String log_end = "^@END:$\n";

		String memory_info_string = log_string + "\n" + myMemoryInfo.createMemoryInfoStringForAll();

		if(outputStreamWriter != null)
		{
			try
			{
				outputStreamWriter.write(log_header);
				outputStreamWriter.write(memory_info_string);
				outputStreamWriter.write(log_end);
				outputStreamWriter.write("\n");
			}
			catch(IOException e)
			{
				e.printStackTrace(); 
			}
		}

		if(isAdbEchoingEnabled)
		{
			Log.i("Test262", log_header);
			Log.i("Test262", memory_info_string);
			Log.i("Test262", log_end);
		}
		loggingConnection.writeToSocketDataOutputStream(log_header);
		loggingConnection.writeToSocketDataOutputStream(memory_info_string);
		loggingConnection.writeToSocketDataOutputStream(log_end);
		loggingConnection.writeToSocketDataOutputStream("\n");
	}

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);

		ActivityManager activity_manager = (ActivityManager)this.getSystemService(ACTIVITY_SERVICE);
		myMemoryInfo = new org.webkit.javascriptcore.test262.MemoryInfo(activity_manager);

			
		// get the Progress Bar component from the XML layout
		runTestButton = (Button)findViewById(R.id.submit_button);
		testProgressBar = (ProgressBar)findViewById(R.id.test_progress_bar);
		resultStatusLabel = (TextView)findViewById(R.id.result_text);
		testProgressBar.setEnabled(false);

		// addKeyListener();
		//
		//
//		displayFiles(this.getAssets(), "", 0);

		String[] file_list = getFileList();
		/*
		for(int i=0; i < file_list.length; i++)
		{
			Log.v("Test262", file_list[i]);
			
		}
		*/
		testFileList = file_list;
		testHarnessString = loadTestHarnessScripts();

		String external_path = Environment.getExternalStorageDirectory().getAbsolutePath();


		numberOfTests = file_list.length;
		
		resultStatusLabel.setText("Total number of tests: " + file_list.length + "\nExternal Storage Path " + external_path);


		testProgressBar.setMax(numberOfTests);
		
		// Zeroconf
        networkLoggingHandler = new Handler()
		{
			@Override
			public void handleMessage(Message msg)
			{
				Log.v("Test262", "In handleMessage: " + msg);
				
//				String chatLine = msg.getData().getString("msg");
//				addChatLine(chatLine);
			}
		};

		loggingConnection = new LoggingConnection(networkLoggingHandler);
		zeroconfHelper = new NsdHelper(this, "_test262logging._tcp.");
        zeroconfHelper.initializeNsd();
        // Register service
		zeroconfHelper.registerService(loggingConnection.getLocalPort());
		loggingConnection.setZeroconfHelper(zeroconfHelper);


		writeMemoryInfoToLogFile("Memory at the end of onCreate");		
	}

	/* Because AssetManager list() is unusable, this is a workaround.
	 * We pre-generate a file containing the entire list of files.
	 * We read the file and put each into an array.
	 */
	public final String[] getFileList()
	{
		ArrayList<String> string_list = new ArrayList<String>();

		try
		{
			InputStreamReader input_stream_reader = new InputStreamReader(this.getAssets().open("test262_filelist.txt"));
			BufferedReader buffered_reader = new BufferedReader(input_stream_reader);

			String current_line;
			//now loop through and check if we have input, if so append it to list
			while((current_line=buffered_reader.readLine()) != null)
			{
				string_list.add(current_line);
			}

		}
		catch (IOException e)
		{
			e.printStackTrace();
		}

		final String[] ret_array = string_list.toArray(new String[string_list.size()]);
		return ret_array;
	}

	static public final String loadFileIntoString(AssetManager asset_manager, String file_name)
	{
		ArrayList<String> string_list = new ArrayList<String>();
		StringBuilder concat_string = new StringBuilder();

		try
		{
			InputStreamReader input_stream_reader = new InputStreamReader(asset_manager.open(file_name));
			BufferedReader buffered_reader = new BufferedReader(input_stream_reader);

			String current_line;
			//now loop through and check if we have input, if so append it to list
			while((current_line=buffered_reader.readLine()) != null)
			{
				concat_string.append(current_line);
				// readLine stripped the newline. 
				// We need to keep it, otherwise a precedeing line that was a // comment causes the following line(s) to continue to be a comment.
				concat_string.append("\n");
			}

		}
		catch (IOException e)
		{
			e.printStackTrace();
		}

		return concat_string.toString();
	}

	// Returns a concatonated string of all the test harness scripts
	public final String loadTestHarnessScripts()
	{
		StringBuilder concat_string = new StringBuilder();		
		concat_string.append(loadFileIntoString(this.getAssets(), "harness/cth.js"));
		concat_string.append(loadFileIntoString(this.getAssets(), "harness/sta.js"));
		concat_string.append(loadFileIntoString(this.getAssets(), "harness/ed.js"));
		concat_string.append(loadFileIntoString(this.getAssets(), "harness/testBuiltInObject.js"));
		concat_string.append(loadFileIntoString(this.getAssets(), "harness/testIntl.js"));
		return concat_string.toString();
	}

	/** Called when the activity is about to be paused. */
	@Override
	protected void onPause()
	{
		Log.i("Test262", "calling onPause");
		
		doPause();
		super.onPause();
	}

	@Override
	protected void onResume()
	{
		Log.i("Test262", "calling onResume");
		
		super.onResume();
		doResume();
	}

	/** Called when the activity is about to be destroyed. */
	@Override
	protected void onDestroy()
	{
        zeroconfHelper.tearDown();
		
		Log.i("Test262", "calling onDestroy");
		doDestroy();

		abortTests();

		// Try not to tear this down until after the log writing is finished
		loggingConnection.tearDown();

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
		
		closeLogFile();
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

	static final String determineStrictModeStringFromScript(String raw_script)
	{
		if(raw_script.contains("@onlyStrict"))
		{
			return "\"use strict\";\nvar strict_mode = true;\n";			
		}
		else
		{
			return "var strict_mode = false; \n";			
		}
	}

	static final boolean determineIfPositiveTestFromScript(String raw_script)
	{
		if(raw_script.contains("@negative"))
		{
			return false;
		}
		else
		{
			return true;
		}
	}


	public String generateTimeStamp()
	{
		// Docs say SimpleDataFormat is not thread safe. So I'm not caching this for reuse right now.
		SimpleDateFormat simple_date_format = new SimpleDateFormat("yyyy-MM-dd_HH:mm:ss.SSS");
		return simple_date_format.format(new Date());
	}

	public void startTests()
	{
		millisecondsStartBenchmark = System.currentTimeMillis();

		numberOfFailedTests = 0;

		boolean log_opened = openLogFile();
		if(!log_opened)
		{
			Log.i("Test262", "Log file could not be opened");
		}
		writeToLogFile("Starting Tests", "Starting Tests at time: " + generateTimeStamp());
		testProgressBar.setEnabled(true);



		Thread thread = new Thread(new Runnable()
		{
			@Override
			public void run()
			{
				writeMemoryInfoToLogFile("Memory at the start of tests");
				javaScriptThreadIsRunning = true;
				final String[] file_list = testFileList;
				int number_of_tests_run = 0;
//				for(int i=0; i < file_list.length; i++)
//				for(int i=0; i < 150; i++)
				for(int i=0; i < numberOfTests; i++)
				{

					if(!javaScriptThreadShouldContinueRunning)
					{
						break;
					}

//					Log.v("Test262", file_list[i]);
					final int current_test_index = i;
					final String current_file_name = file_list[i];

					writeMemoryInfoToLogFile("Memory at the beginning of loop[" + i + "] :" + current_file_name);
					
					runOnUiThread(new Runnable()
					{
						@Override
						public void run()
						{

							testProgressBar.setProgress(current_test_index);
							resultStatusLabel.setText("Running test " + current_test_index + " of " + numberOfTests + "\n" + current_file_name + "\nTotal failed: " + numberOfFailedTests);

						}
					});

//		Log.i("Test262", "current_file_name: " + current_file_name + ".");
					final String raw_test_script = loadFileIntoString(Test262.this.getAssets(), current_file_name);
//		Log.i("Test262", "raw_test_script: " + raw_test_script);
					
					final boolean is_positive_test = determineIfPositiveTestFromScript(raw_test_script);
//		Log.i("Test262", "testHarnessString: " + testHarnessString + ".");

					StringBuilder concat_string = new StringBuilder();
					concat_string.append(determineStrictModeStringFromScript(raw_test_script));
					concat_string.append("\n");
					concat_string.append(testHarnessString);
					concat_string.append("\n");
					concat_string.append(raw_test_script);

					//final String full_script = determineStrictModeStringFromScript(raw_test_script) + testHarnessString + raw_test_script;
					final String full_script = concat_string.toString();
//		Log.i("Test262", "full_script: " + full_script + ".");

					final ReturnDataObject return_data_object = new ReturnDataObject();
//		Log.i("Test262", "calling into C");
					writeToLogFile("Evaluating Script", "Evaluating Script: " + current_file_name);
					writeMemoryInfoToLogFile("Memory right before (JNI) execution of script " + current_file_name);

					final boolean is_success = evaluateScript(full_script, current_file_name, return_data_object);
					writeMemoryInfoToLogFile("Memory right after (JNI) execution of script " + current_file_name);
					number_of_tests_run = number_of_tests_run + 1;

					// FIXME: This likely deserves a better check.
					if(is_positive_test != is_success)
					{
						numberOfFailedTests = numberOfFailedTests + 1;

//
//                    			this.failedTests.Add(result);
						if(is_success && !is_positive_test)
						{
//									Log.i("Test262", "Test failed: (script passed but negative test means it should have not have passed): " + current_file_name + ", " + return_data_object.getExceptionString() + ", " + return_data_object.getStackString() + "\n" + raw_test_script);		
							writeToLogFile("Test failed", "Test failed: (script passed but negative test means it should have not have passed): " + current_file_name + ", " + return_data_object.getExceptionString() + ", " + return_data_object.getStackString() + "\n" + raw_test_script);
						}
						else
						{
//									Log.i("Test262", "Test failed: " + current_file_name + ", " + return_data_object.getExceptionString() + ", " + return_data_object.getStackString() + "\n" + raw_test_script);
							writeToLogFile("Test failed", "Test failed: " + current_file_name + ", " + return_data_object.getExceptionString() + ", " + return_data_object.getStackString() + "\n" + raw_test_script);
						}
					}
					else
					{
//								Log.i("Test262", "Test passed: " + current_file_name);
						writeToLogFile("Test passed", "Test passed: " + current_file_name);
					}

					/* // this will be updated soon enough in the next loop
					runOnUiThread(new Runnable()
					{
						@Override
						public void run()
						{

							testProgressBar.setProgress(current_test_index + 1);
							resultStatusLabel.setText("Running test " + current_test_index + " of " + numberOfTests + "\n" + current_file_name + "\nTotal failed: " + numberOfFailedTests);
						}
					});
					*/
				}

				final long total_execution_time_in_milliseconds = System.currentTimeMillis() - millisecondsStartBenchmark;
				final double total_execution_time_in_seconds = total_execution_time_in_milliseconds / 1000.0;
				final String completed_string = "Tests completed: " + number_of_tests_run + " of " + numberOfTests + " run.\nTotal failed: " + numberOfFailedTests + "\nTotal execution time: " + total_execution_time_in_seconds + "seconds\n" + "Timestamp: " + generateTimeStamp() + "\n"
					+ Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "test262_runlog.txt";
				final int final_number_of_tests_run = number_of_tests_run;
				writeToLogFile("Tests completed", completed_string);

				runOnUiThread(new Runnable()
					{
						@Override
						public void run()
						{
							Log.i("Test262", "finished ");		

							testProgressBar.setProgress(final_number_of_tests_run);
							
							resultStatusLabel.setText(completed_string);


//							runTestButton.setEnabled(true);
							runTestButton.setText(getApplicationContext().getString(R.string.submit_press));
							

						}
					}
				);

				writeMemoryInfoToLogFile("Memory at the end of tests");

				closeLogFile();

				javaScriptThreadIsRunning = false;	
				javaScriptThread = null;

			}
				
		});
		javaScriptThreadShouldContinueRunning = true;
		javaScriptThread = thread;
		thread.start();
	}


}
