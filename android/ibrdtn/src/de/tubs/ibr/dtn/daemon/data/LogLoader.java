/*
 * LogLoader.java
 * 
 * Copyright (C) 2013 IBR, TU Braunschweig
 * 
 * Written-by: Dominik Schürmann <dominik@dominikschuermann.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

package de.tubs.ibr.dtn.daemon.data;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

import de.tubs.ibr.dtn.daemon.LogMessage;
import android.content.Context;
import android.os.AsyncTask;
import android.support.v4.content.Loader;
import android.util.Log;

public class LogLoader extends Loader<LogMessage> {
    private static final String TAG = "LogLoader";
    private AsyncTask<Void, LogMessage, Void> mLogcatTask;

    public LogLoader(Context context) {
        super(context);
    }

    private class LogcatTask extends AsyncTask<Void, LogMessage, Void> {

        @Override
        protected Void doInBackground(Void... params) {
            Process process = null;
            try {
                // show only logs from IBR-DTN daemon
                process = Runtime.getRuntime().exec("/system/bin/logcat -v time *:V");
            } catch (IOException e) {
                Log.e(TAG, "Problem starting logcat in new Process!", e);
            }

            BufferedReader reader = null;

            try {
                reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
                String line = null;
                while (!isCancelled() && (line = reader.readLine()) != null) {
                    try {
                        LogMessage msg = new LogMessage(line);
                        publishProgress(msg);
                    } catch (IllegalStateException e) {
                        // Does not match!
                    }
                }

                reader.close();
                process.destroy();
                process = null;
                reader = null;
            } catch (IOException e) {
                Log.e(TAG, "Reading from logcat failed!", e);
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(LogMessage... values) {
            // deliver result to callback implementation of this loader
            deliverResult(values[0]);
        }
    };

    @Override
    protected void onReset() {
    	super.onReset();
        onStopLoading();
        
    	if (mLogcatTask != null) {
    		mLogcatTask.cancel(false);
    		mLogcatTask = null;
    	}
    }

    @Override
    protected void onStartLoading() {
        // start thread that continuously gets logcat's output
        mLogcatTask = new LogcatTask().execute();
    }
}
