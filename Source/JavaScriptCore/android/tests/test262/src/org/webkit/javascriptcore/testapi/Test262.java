package org.webkit.javascriptcore.test262;

import android.app.Activity;
import android.os.Bundle;
import android.os.Environment;
import android.content.res.AssetManager;
import android.util.Log;
import android.view.View;
import android.widget.TextView;
import android.widget.Button;

import android.view.KeyEvent;
import android.view.View.OnKeyListener;
import android.widget.EditText;
import android.widget.Toast;

import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.IOException;

import java.util.Arrays;
import java.util.ArrayList;
import java.io.InputStream;
import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.OutputStreamWriter;
import java.io.FileOutputStream;
import java.io.File;
import java.io.BufferedWriter;
import java.io.FileWriter;

import java.text.DateFormat;
import java.util.Date;
import java.text.SimpleDateFormat;

//import android.media.MediaScannerConnection;
import android.app.ActivityManager;
//import android.app.ActivityManager.MemoryInfo;
//import android.os.Debug.MemoryInfo;
import android.os.Debug;

import java.util.List; 
import java.util.TreeMap;
import java.util.Map;
import java.util.Collection;

import android.app.ActivityManager.RunningAppProcessInfo;
import android.os.Process;



public class Test262 extends Activity
{
	static
	{
		System.loadLibrary("JavaScriptCore");
		System.loadLibrary("Test262");
	}
    

	private String testHarnessString;
	private String[] testFileList;
	private int numberOfTests = 0;
	private int numberOfFailedTests = 0;
	
	private long millisecondsStartBenchmark;
	private EditText inputTextField;

	private BufferedWriter outputStreamWriter;
//	private OutputStreamWriter outputStreamWriter;
	private File logFile;
	private FileWriter logFileWriter;

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
	
	private void writeToLogFile(String log_string)
	{
//		Log.i("Test262", "entered writeToLogFile " + log_string);
		
		if(outputStreamWriter != null)
		{
			try
			{
//		Log.i("Test262", "try writeToLogFile " + log_string);
				
				outputStreamWriter.write(log_string);
				outputStreamWriter.write("\n");
			}
			catch(IOException e)
			{
				e.printStackTrace(); 
			}

		}
	}

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		// get EditText component
		inputTextField = (EditText)findViewById(R.id.input_text);
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
		
		TextView result_text_view = (TextView)Test262.this.findViewById(R.id.result_text);
//		result_text_view.setText("Total number of tests: " + file_list.length);
		result_text_view.setText("Total number of tests: " + file_list.length + "\nExternal Storage Path " + external_path);

		openLogFile();
		writeToLogFile("Message1");
		writeToLogFile("Message2");
		closeLogFile();


		Runtime rt = Runtime.getRuntime();
Log.d("Test262", 
	String.format("maxMemory:%d, freeMemory:%d,totalMemory:%d, allocatedMemory:%d, nativeHeapAllocated:%d, nativeHeap:%d,", 
		rt.maxMemory(),
		rt.freeMemory(),
		rt.totalMemory(),
		rt.totalMemory() - rt.freeMemory(),
		android.os.Debug.getNativeHeapAllocatedSize(),
		android.os.Debug.getNativeHeapSize()
	)
);

ActivityManager activityManager = (ActivityManager)this.getSystemService(ACTIVITY_SERVICE);
ActivityManager.MemoryInfo memoryInfo = new ActivityManager.MemoryInfo();
activityManager.getMemoryInfo(memoryInfo);


Log.i("Test262", " memoryInfo.availMem " + memoryInfo.availMem + "\n" );
Log.i("Test262", " memoryInfo.lowMemory " + memoryInfo.lowMemory + "\n" );
Log.i("Test262", " memoryInfo.threshold " + memoryInfo.threshold + "\n" );

int my_pid = android.os.Process.myPid();


List<RunningAppProcessInfo> runningAppProcesses = activityManager.getRunningAppProcesses();

Map<Integer, String> pidMap = new TreeMap<Integer, String>();
for (RunningAppProcessInfo runningAppProcessInfo : runningAppProcesses)
{
    pidMap.put(runningAppProcessInfo.pid, runningAppProcessInfo.processName);
}
Collection<Integer> keys = pidMap.keySet();

for(int key : keys)
{
    int pids[] = new int[1];
    pids[0] = key;
    android.os.Debug.MemoryInfo[] memoryInfoArray = activityManager.getProcessMemoryInfo(pids);
    for(android.os.Debug.MemoryInfo pidMemoryInfo: memoryInfoArray)
    {
        Log.i("Test262", String.format("** MEMINFO in pid %d [%s] **\n",pids[0],pidMap.get(pids[0])));
        Log.i("Test262", " pidMemoryInfo.getTotalPrivateDirty(): " + pidMemoryInfo.getTotalPrivateDirty() + "\n");
        Log.i("Test262", " pidMemoryInfo.getTotalPss(): " + pidMemoryInfo.getTotalPss() + "\n");
        Log.i("Test262", " pidMemoryInfo.getTotalSharedDirty(): " + pidMemoryInfo.getTotalSharedDirty() + "\n");
    }
}
        Log.i("Test262", "my pid: " + my_pid);

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
		
		Log.i("Test262", "calling onDestroy");
		doDestroy();
		
		closeLogFile();
		super.onDestroy();
		Log.i("Test262", "finished calling onDestroy");		
	}



    public void myClickHandler(View the_view)
	{
		switch(the_view.getId())
		{
			case R.id.submit_button:
			{
				Button submit_button = (Button)Test262.this.findViewById(R.id.submit_button);
				submit_button.setEnabled(false);

				startTests();
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
		// docs say SimpleDataFormat is not thread safe.
		SimpleDateFormat simple_date_format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS");
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
		writeToLogFile(generateTimeStamp() + " Test262: Starting tests");



		Thread thread = new Thread(new Runnable()
		{
			@Override
			public void run()
			{
				final String[] file_list = testFileList;
//				for(int i=0; i < file_list.length; i++)
				for(int i=0; i < numberOfTests; i++)
				{
//					Log.v("Test262", file_list[i]);

					final String current_file_name = file_list[i];
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
					writeToLogFile(generateTimeStamp() + " Test262: Evaluating Script: " + current_file_name);

					final boolean is_success = evaluateScript(full_script, current_file_name, return_data_object);
					final int current_test_index = i;

					runOnUiThread(new Runnable()
					{
						@Override
						public void run()
						{

							/*
								TextView result_text_view = (TextView)Test262.this.findViewById(R.id.result_text);
							result_text_view.setText(full_script);
		EditText edit_text = (EditText)findViewById(R.id.input_text);
							edit_text.setText(full_script);
*/

							// FIXME: This likely deserves a better check.
							if(is_positive_test != is_success)
							{
								numberOfFailedTests = numberOfFailedTests + 1;

//
//                    			this.failedTests.Add(result);
								if(is_success && !is_positive_test)
								{
									Log.i("Test262", "Test failed: (script passed but negative test means it should have not have passed): " + current_file_name + ", " + return_data_object.getExceptionString() + ", " + return_data_object.getStackString() + "\n" + raw_test_script);		
									writeToLogFile(generateTimeStamp() + " Test262: Test failed: (script passed but negative test means it should have not have passed): " + current_file_name + ", " + return_data_object.getExceptionString() + ", " + return_data_object.getStackString() + "\n" + raw_test_script);
								}
								else
								{
									Log.i("Test262", "Test failed: " + current_file_name + ", " + return_data_object.getExceptionString() + ", " + return_data_object.getStackString() + "\n" + raw_test_script);
									writeToLogFile(generateTimeStamp() + " Test262: Test failed: " + current_file_name + ", " + return_data_object.getExceptionString() + ", " + return_data_object.getStackString() + "\n" + raw_test_script);
								}
							}
							else
							{
//								Log.i("Test262", "Test passed: " + current_file_name);
								writeToLogFile(generateTimeStamp() + " Test262: Test passed: " + current_file_name);
							}

							// display a floating message
//							Toast.makeText(Test262.this, resultString, Toast.LENGTH_LONG).show();

							TextView result_text_view = (TextView)Test262.this.findViewById(R.id.result_text);
							result_text_view.setText("Running test " + current_test_index + " of " + numberOfTests + "\nTotal failed: " + numberOfFailedTests);

//							Button submit_button = (Button)Test262.this.findViewById(R.id.submit_button);
//							submit_button.setEnabled(true);
							

						}
					});
				}

				runOnUiThread(new Runnable()
					{
						@Override
						public void run()
						{
							Log.i("Test262", "finished ");		

							// display a floating message
//							Toast.makeText(Test262.this, resultString, Toast.LENGTH_LONG).show();

//							TextView result_text_view = (TextView)Test262.this.findViewById(R.id.result_text);
//							result_text_view.setText(resultString);

							
							long total_execution_time_in_milliseconds = System.currentTimeMillis() - millisecondsStartBenchmark;
							double total_execution_time_in_seconds = total_execution_time_in_milliseconds / 1000.0;
							TextView result_text_view = (TextView)Test262.this.findViewById(R.id.result_text);
							result_text_view.setText(numberOfTests + "complete.\nTotal failed: " + numberOfFailedTests + "\nTotal execution time: " + total_execution_time_in_seconds + "seconds\nadb pull " + Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "test262_runlog.txt");




							Button submit_button = (Button)Test262.this.findViewById(R.id.submit_button);
							submit_button.setEnabled(true);
							closeLogFile();
							

						}
					});

			}
				
		});
		thread.start();
	}


}
