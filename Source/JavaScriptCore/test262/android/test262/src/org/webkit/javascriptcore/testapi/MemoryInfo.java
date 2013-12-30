package org.webkit.javascriptcore.test262;

import android.app.ActivityManager;
//import android.app.ActivityManager.MemoryInfo;
//import android.os.Debug.MemoryInfo;
import android.os.Debug;

import android.app.ActivityManager.RunningAppProcessInfo;
import android.os.Process;


import java.util.List; 
import java.util.TreeMap;
import java.util.Map;
import java.util.Collection;

// This is a silly little class to store data needed from returning from evaulating a JavaScript script.
// Because Java/JNI does not allow multiple return values, we need to call into Java to pass more data.
// This class provides a way to store return values on set per-instance basis.
public class MemoryInfo extends Object
{

	private ActivityManager activityManager;
	ActivityManager.MemoryInfo memoryInfo;
	int myPid;
    int pidsArray[] = new int[1];
	
	MemoryInfo(ActivityManager activity_manager)
	{
		activityManager = activity_manager;
		memoryInfo = new ActivityManager.MemoryInfo();
		myPid = android.os.Process.myPid();
		pidsArray[0] = myPid;
	}

	public String createMemoryInfoStringForActivityManager()
	{
		activityManager.getMemoryInfo(memoryInfo);

		StringBuilder concat_string = new StringBuilder();
		concat_string.append("ActivityManager.MemoryInfo.availMem: ");
		concat_string.append(memoryInfo.availMem);
		concat_string.append("\n");

		concat_string.append("ActivityManager.MemoryInfo.lowMemory: ");
		concat_string.append(memoryInfo.availMem);
		concat_string.append("\n");

		concat_string.append("ActivityManager.MemoryInfo.threshold: ");
		concat_string.append(memoryInfo.availMem);
		concat_string.append("\n");

		return concat_string.toString();
	}

	
	public String createMemoryInfoStringForRuntime()
	{
		Runtime rt = Runtime.getRuntime();

		StringBuilder concat_string = new StringBuilder();

		concat_string.append("Runtime.getRuntime().maxMemory(): ");
		concat_string.append(rt.maxMemory());
		concat_string.append("\n");
		
		concat_string.append("Runtime.getRuntime().freeMemory(): ");
		concat_string.append(rt.freeMemory());
		concat_string.append("\n");

		concat_string.append("Runtime.getRuntime().totalMemory(): ");
		concat_string.append(rt.totalMemory());
		concat_string.append("\n");
		
		return concat_string.toString();
	}

		
	public String createMemoryInfoStringForDebug()
	{
		Runtime rt = Runtime.getRuntime();

		StringBuilder concat_string = new StringBuilder();

		concat_string.append("android.os.Debug.getNativeHeapAllocatedSize(): ");
		concat_string.append(android.os.Debug.getNativeHeapAllocatedSize());
		concat_string.append("\n");
		
		concat_string.append("android.os.Debug.getNativeHeapSize(): ");
		concat_string.append(android.os.Debug.getNativeHeapSize());
		concat_string.append("\n");

		concat_string.append("android.os.Debug.getNativeHeapFreeSize(): ");
		concat_string.append(android.os.Debug.getNativeHeapFreeSize());
		concat_string.append("\n");

		concat_string.append("android.os.Debug.getPss(): ");
		concat_string.append(android.os.Debug.getPss());
		concat_string.append("\n");


		android.os.Debug.MemoryInfo[] debug_memory_info_array = activityManager.getProcessMemoryInfo(pidsArray);
		android.os.Debug.MemoryInfo pid_memory_info = debug_memory_info_array[0];


		concat_string.append("android.os.Debug.MemoryInfo[PID].getTotalPrivateClean(): ");
		concat_string.append(pid_memory_info.getTotalPrivateClean());
		concat_string.append("\n");

		concat_string.append("android.os.Debug.MemoryInfo[PID].getTotalPrivateDirty(): ");
		concat_string.append(pid_memory_info.getTotalPrivateDirty());
		concat_string.append("\n");

		concat_string.append("android.os.Debug.MemoryInfo[PID].getTotalPss(): ");
		concat_string.append(pid_memory_info.getTotalPss());
		concat_string.append("\n");

		concat_string.append("android.os.Debug.MemoryInfo[PID].getTotalSharedClean(): ");
		concat_string.append(pid_memory_info.getTotalSharedClean());
		concat_string.append("\n");

		concat_string.append("android.os.Debug.MemoryInfo[PID].getTotalSharedDirty(): ");
		concat_string.append(pid_memory_info.getTotalSharedDirty());
		concat_string.append("\n");

		concat_string.append("android.os.Debug.MemoryInfo[PID].getTotalSwappablePss(): ");
		concat_string.append(pid_memory_info.getTotalSwappablePss());
		concat_string.append("\n");


		concat_string.append("android.os.Debug.MemoryInfo[PID].dalvikPrivateDirty: ");
		concat_string.append(pid_memory_info.dalvikPrivateDirty);
		concat_string.append("\n");

		concat_string.append("android.os.Debug.MemoryInfo[PID].dalvikPss: ");
		concat_string.append(pid_memory_info.dalvikPss);
		concat_string.append("\n");

		concat_string.append("android.os.Debug.MemoryInfo[PID].dalvikSharedDirty: ");
		concat_string.append(pid_memory_info.dalvikSharedDirty);
		concat_string.append("\n");

		concat_string.append("android.os.Debug.MemoryInfo[PID].nativePrivateDirty: ");
		concat_string.append(pid_memory_info.nativePrivateDirty);
		concat_string.append("\n");

		concat_string.append("android.os.Debug.MemoryInfo[PID].nativePss: ");
		concat_string.append(pid_memory_info.nativePss);
		concat_string.append("\n");

		concat_string.append("android.os.Debug.MemoryInfo[PID].nativeSharedDirty: ");
		concat_string.append(pid_memory_info.nativeSharedDirty);
		concat_string.append("\n");

		concat_string.append("android.os.Debug.MemoryInfo[PID].otherPrivateDirty: ");
		concat_string.append(pid_memory_info.otherPrivateDirty);
		concat_string.append("\n");

		concat_string.append("android.os.Debug.MemoryInfo[PID].otherPss: ");
		concat_string.append(pid_memory_info.otherPss);
		concat_string.append("\n");

		concat_string.append("android.os.Debug.MemoryInfo[PID].otherSharedDirty: ");
		concat_string.append(pid_memory_info.otherSharedDirty);
		concat_string.append("\n");



		return concat_string.toString();
	}

	public String createMemoryInfoStringForAll()
	{
		StringBuilder concat_string = new StringBuilder();
		
		concat_string.append(createMemoryInfoStringForActivityManager());
		concat_string.append(createMemoryInfoStringForRuntime());
		concat_string.append(createMemoryInfoStringForDebug());

		return concat_string.toString();
	}

	/*
	{

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
	*/
}
