package de.tubs.ibr.dtn.service;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

import android.util.Log;

public class StreamLogger implements Runnable {
	String TAG = null;
	InputStream stream = null;
	boolean errorOnExit = false;
	StreamLoggerListener listener = null;
	
	public interface StreamLoggerListener {
		public void onLog(String TAG, String log);
	}
	
	public StreamLogger(String tag, InputStream stream, StreamLoggerListener listener) {
		this.TAG = tag;
		this.stream = stream;
		this.listener = listener;
	}
	
	public boolean hasErrorOnExit() {
		return errorOnExit;
	}

	public void run() {
		try {
			// buffer for a line
		    String line = null;
		    
		    BufferedReader input = new BufferedReader( new InputStreamReader( stream ) );
		
			while ((line = input.readLine()) != null)
			{
				if (listener != null) listener.onLog(TAG, line);
				Log.v(TAG, line);
			}
		} catch (IOException e) {
			this.errorOnExit = true;
		}
	}	
}
