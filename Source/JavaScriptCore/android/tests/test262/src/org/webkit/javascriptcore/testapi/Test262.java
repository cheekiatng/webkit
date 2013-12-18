package org.webkit.javascriptcore.test262;

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

public class Test262 extends Activity
{
	static
	{
		System.loadLibrary("JavaScriptCore");
		System.loadLibrary("Test262");
	}
    
	public native boolean doInit(AssetManager java_asset_manager);
	public native void doPause();
	public native void doResume();
	public native void doDestroy();
	public native void playSound(int sound_id);
	public native String evaluateScript(String script_file, AssetManager java_asset_manager);

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
				String result_string = evaluateScript("test262.js", this.getAssets());
				Log.i("Test262", "result: " + result_string);		

				// display a floating message
				Toast.makeText(Test262.this, result_string, Toast.LENGTH_LONG).show();

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
