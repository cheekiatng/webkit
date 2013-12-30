package org.webkit.javascriptcore.test262;

// This is a silly little class to store data needed from returning from evaulating a JavaScript script.
// Because Java/JNI does not allow multiple return values, we need to call into Java to pass more data.
// This class provides a way to store return values on set per-instance basis.
public class ReturnDataObject extends Object
{
	private String exceptionString = null;
	private String stackString = null;

	public void setExceptionString(String exception_string)
	{
		exceptionString = exception_string;
	}

	public String getExceptionString()
	{
		return exceptionString;
	}

	public void setStackString(String stack_string)
	{
		stackString = stack_string;
	}

	public String getStackString()
	{
		return stackString;
	}
}

