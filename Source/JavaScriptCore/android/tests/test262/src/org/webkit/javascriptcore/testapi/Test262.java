package org.webkit.javascriptcore.test262;

import android.app.Activity;
import android.os.Bundle;
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

public class Test262 extends Activity
{
	static
	{
		System.loadLibrary("JavaScriptCore");
		System.loadLibrary("Test262");
	}
    

	String testHarnessString;
	String[] testFileList;

	public native boolean doInit(AssetManager java_asset_manager);
	public native void doPause();
	public native void doResume();
	public native void doDestroy();
	public native void playSound(int sound_id);
	public native boolean evaluateScript(String script_string, String file_name, ReturnDataObject return_data_object);

	private EditText inputTextField;
	
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
		for(int i=0; i < file_list.length; i++)
		{
			Log.v("Test262", file_list[i]);
			
		}
		testFileList = file_list;
		testHarnessString = loadTestHarnessScripts();

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

/* AssetManager list() is unusable. Just to list (not open or read) a file, it 
 * takes about 1 second per file. This is apparently a known Android bug that Google doesn't care to fix.
 * So we need to find another solution.
 */
/*
	public void displayFiles(AssetManager mgr, String path, int level)
	{

		Log.v("Test262","enter displayFiles("+path+")");
		try {
			String list[] = mgr.list(path);
			Log.v("Test262","L"+level+": list:"+ Arrays.asList(list));

			if (list != null)
				for (int i=0; i<list.length; ++i)
				{
					if(level>=1){
						displayFiles(mgr, path + "/" + list[i], level+1);
					}else{
						displayFiles(mgr, list[i], level+1);
					}
				}
		} catch (IOException e) {
			Log.v("Test262","List error: can't list" + path);
		}

	}
*/

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
		
		super.onDestroy();
		Log.i("Test262", "finished calling onDestroy");		
	}



    public void myClickHandler(View the_view)
	{
		switch(the_view.getId())
		{
			case R.id.submit_button:
			{
//				Log.i("Test262", "calling: " + inputTextField.getText());		
				//String result_string = evaluateScript(inputTextField.getText().toString());
				
				Button submit_button = (Button)Test262.this.findViewById(R.id.submit_button);
				submit_button.setEnabled(false);
/*
				Thread thread = new Thread(new Runnable()
				{
					@Override
					public void run()
					{

						final String resultString = evaluateScript("testapi.js", Test262.this.getAssets());
						runOnUiThread(new Runnable()
						{
							@Override
							public void run()
							{
								Log.i("Test262", "result: " + resultString);		

								// display a floating message
								Toast.makeText(Test262.this, resultString, Toast.LENGTH_LONG).show();

								TextView result_text_view = (TextView)Test262.this.findViewById(R.id.result_text);
								result_text_view.setText(resultString);

								Button submit_button = (Button)Test262.this.findViewById(R.id.submit_button);
								submit_button.setEnabled(true);
								

							}
						});
					}
					
				});
				thread.start();
				*/
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



	public void startTests()
	{
		Thread thread = new Thread(new Runnable()
		{
			@Override
			public void run()
			{
				final String[] file_list = testFileList;
//				for(int i=0; i < file_list.length; i++)
				for(int i=0; i < 100; i++)
				{
					Log.v("Test262", file_list[i]);

					final String current_file_name = file_list[i];
					String raw_test_script = loadFileIntoString(Test262.this.getAssets(), file_list[i]);
					final boolean is_positive_test = determineIfPositiveTestFromScript(raw_test_script);
					final String full_script = determineStrictModeStringFromScript(raw_test_script) + testHarnessString + raw_test_script;

					final ReturnDataObject return_data_object = new ReturnDataObject();

					final boolean is_success = evaluateScript(full_script, current_file_name, return_data_object);
					
final String resultString = "no-op";
					runOnUiThread(new Runnable()
					{
						@Override
						public void run()
						{


							// FIXME: This likely deserves a better check.
							if(is_positive_test != is_success)
							{
//                    			this.failedTests.Add(result);
								Log.i("Test262", "Test failed: " + current_file_name + ", " + return_data_object.getExceptionString() + ", " + return_data_object.getStackString());		
							}
							else
							{
								Log.i("Test262", "Test passed: " + current_file_name);		
							}

							// display a floating message
//							Toast.makeText(Test262.this, resultString, Toast.LENGTH_LONG).show();

//							TextView result_text_view = (TextView)Test262.this.findViewById(R.id.result_text);
//							result_text_view.setText(resultString);

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

							Button submit_button = (Button)Test262.this.findViewById(R.id.submit_button);
							submit_button.setEnabled(true);
							

						}
					});

			}
				
		});
		thread.start();
	}

	
		/*
	public void addKeyListener()
	{
		// add a keylistener to keep track user input
		inputTextField.setOnKeyListener(
			new OnKeyListener()
			{
				public boolean onKey(View the_view, int key_code, KeyEvent key_event)
				{
					// if keydown and "enter" is pressed
					if((key_event.getAction() == key_event.ACTION_DOWN)
						&& (key_code == key_event.KEYCODE_ENTER))
					{
						// display a floating message
						Toast.makeText(Test262.this,
							inputTextField.getText(), Toast.LENGTH_LONG).show();

						Log.i("Test262", "calling: " + inputTextField.getText());		
						String result_string = evaluateScript(inputTextField.getText().toString());

						TextView result_text_view = (TextView)Test262.this.findViewById(R.id.result_text);
						result_text_view.setText(result_string);

						return true;
					}
					return false;
				}
			}
		);
	}
	*/
}
