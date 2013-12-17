package org.webkit.javascriptcore.testapi;

import android.app.Activity;
import android.os.Bundle;
import android.content.res.AssetManager;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import android.view.KeyEvent;
import android.view.View.OnKeyListener;
import android.widget.EditText;
import android.widget.Toast;

public class TestAPI extends Activity
{
	static
	{
		System.loadLibrary("JavaScriptCore");
		System.loadLibrary("TestAPI");
	}
    
	public native boolean doInit(AssetManager java_asset_manager);
	public native void doPause();
	public native void doResume();
	public native void doDestroy();
	public native void playSound(int sound_id);
	public native String evaluateScript(String script_string);

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
    }


	
	/** Called when the activity is about to be paused. */
	@Override
	protected void onPause()
	{
		Log.i("TestAPI", "calling onPause");
		
		doPause();
		super.onPause();
	}

	@Override
	protected void onResume()
	{
		Log.i("TestAPI", "calling onResume");
		
		super.onResume();
		doResume();
	}

	/** Called when the activity is about to be destroyed. */
	@Override
	protected void onDestroy()
	{
		Log.i("TestAPI", "calling onDestroy");
		doDestroy();
		
		super.onDestroy();
		Log.i("TestAPI", "finished calling onDestroy");		
	}



    public void myClickHandler(View the_view)
	{
		switch(the_view.getId())
		{
			case R.id.submit_button:
			{
				Log.i("TestAPI", "calling: " + inputTextField.getText());		
				String result_string = evaluateScript(inputTextField.getText().toString());
				Log.i("TestAPI", "result: " + result_string);		

				// display a floating message
				Toast.makeText(TestAPI.this, result_string, Toast.LENGTH_LONG).show();

				TextView result_text_view = (TextView)this.findViewById(R.id.result_text);
				result_text_view.setText(result_string);
				
				break;
			}
			default:
			{
				break;
			}
		}
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
						Toast.makeText(TestAPI.this,
							inputTextField.getText(), Toast.LENGTH_LONG).show();

						Log.i("TestAPI", "calling: " + inputTextField.getText());		
						String result_string = evaluateScript(inputTextField.getText().toString());

						TextView result_text_view = (TextView)TestAPI.this.findViewById(R.id.result_text);
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
