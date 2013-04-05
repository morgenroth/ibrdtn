/*
 * DaemonMainThread.java
 * 
 * Copyright (C) 2013 IBR, TU Braunschweig
 *
 * Written-by: Dominik Schürmann <dominik@dominikschuermann.de>
 * 	           Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
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
package de.tubs.ibr.dtn.service;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Calendar;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.provider.Settings.Secure;
import android.util.Log;
import de.tubs.ibr.dtn.DaemonState;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.swig.NativeDaemon;
import de.tubs.ibr.dtn.swig.NativeDaemonCallback;
import de.tubs.ibr.dtn.swig.NativeEventCallback;
import de.tubs.ibr.dtn.swig.StringVec;

public class DaemonMainThread {
	private final static String TAG = "DaemonMainThread";

	private NativeDaemon mDaemon = null;
	private DaemonService mService = null;
	private ExecutorService _executor = null;
	private DaemonState _state = DaemonState.OFFLINE;
	
	private final static String GNUSTL_NAME = "gnustl_shared";
	private final static String CRYPTO_NAME = "crypto";
	private final static String SSL_NAME = "ssl";
	private final static String IBRCOMMON_NAME = "ibrcommon";
	private final static String IBRDTN_NAME = "ibrdtn";
	private final static String DTND_NAME = "dtnd";
	private final static String ANDROID_GLUE_NAME = "android-glue";

	/**
	 * Loads all shared libraries in the right order with System.loadLibrary()
	 */
	private static void loadLibraries()
	{
		try
		{
			System.loadLibrary(GNUSTL_NAME);

			// System.loadLibrary(CRYPTO_NAME);
			// System.loadLibrary(SSL_NAME);

			System.loadLibrary(IBRCOMMON_NAME);
			System.loadLibrary(IBRDTN_NAME);
			System.loadLibrary(DTND_NAME);

			System.loadLibrary(ANDROID_GLUE_NAME);
		} catch (UnsatisfiedLinkError e)
		{
			Log.e(TAG, "UnsatisfiedLinkError! Are you running special hardware?", e);
		} catch (Exception e)
		{
			Log.e(TAG, "Loading the libraries failed!", e);
		}
	}
	
	static
	{
		// load libraries on first use of this class
		loadLibraries();
	}

	public DaemonMainThread(DaemonService context) {
		this.mDaemon = new NativeDaemon(mDaemonCallback, mEventCallback);
		this.mService = context;
		this._executor = Executors.newSingleThreadExecutor();
	}
	
	public NativeDaemon getNative() {
		return mDaemon;
	}
	
	public DaemonState getState() {
	    return _state;
	}
	
	public void start() {
	    // submit the startup procedure to the executor
	    _executor.submit(_main_loop);
	}
	
	public void stop() {
	    // stop the running daemon
	    mDaemon.shutdown();
	}

	private Runnable _main_loop = new Runnable() {
        @Override
        public void run() {
            // set state to UNKOWN to indicate that it is currently starting
            changeDaemonState(DaemonState.UNKOWN);
            
            // lower priority of this thread
            android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_BACKGROUND);

            String configPath = mService.getFilesDir().getPath() + "/" + "config";

            // create configuration file
            createConfig(mService, configPath);

            // enable debug based on prefs
            SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(mService);
            int logLevel = Integer.valueOf(preferences.getString("log_options", "0"));
            int debugVerbosity = Integer.valueOf(preferences.getString("pref_debug_verbosity", "0"));

            // loads config and initializes daemon
            DaemonMainThread.this.mDaemon.enableLogging(configPath, "Core", logLevel, debugVerbosity);
            DaemonMainThread.this.mDaemon.initialize();
            
            // blocking main loop
            DaemonMainThread.this.mDaemon.main_loop();
        }	    
	};
	
	private NativeEventCallback mEventCallback = new NativeEventCallback() {
        @Override
        public void eventRaised(String eventName, String action, StringVec data) {
            Intent event = new Intent(de.tubs.ibr.dtn.Intent.EVENT);
            Intent neighborIntent = null;

            event.addCategory(Intent.CATEGORY_DEFAULT);
            event.putExtra("name", eventName);

            if (eventName.equals("NodeEvent")) {
                neighborIntent = new Intent(de.tubs.ibr.dtn.Intent.NEIGHBOR);
                neighborIntent.addCategory(Intent.CATEGORY_DEFAULT);
            }

            // place the action into the intent
            if (action.length() > 0)
            {
                event.putExtra("action", action);

                if (neighborIntent != null) {
                    if (action.equals("available")) {
                        neighborIntent.putExtra("action", "available");
                    }
                    else if (action.equals("unavailable")) {
                        neighborIntent.putExtra("action", "unavailable");
                    }
                    else {
                        neighborIntent = null;
                    }
                }
            }

            // put all attributes into the intent
            for (int i = 0; i < data.size(); i++) {
                String entry = data.get(i);
                String entry_data[] = entry.split(": ", 2);
                
                // skip invalid entries
                if (entry_data.length < 2) continue;

                event.putExtra("attr:" + entry_data[0], entry_data[1]);
                if (neighborIntent != null) {
                    neighborIntent.putExtra("attr:" + entry_data[0], entry_data[1]);
                }
            }

            // send event intent
            mService.sendBroadcast(event);

            if (neighborIntent != null) {
                mService.sendBroadcast(neighborIntent);
                mService.onNeighborhoodChanged();
            }

            if (Log.isLoggable(TAG, Log.DEBUG)) {
                Log.d(TAG, "EVENT intent broadcasted: " + eventName + "; Action: " + action);
            }
        }
	};
	
	private NativeDaemonCallback mDaemonCallback = new NativeDaemonCallback() {
		@Override
		public void stateChanged(States state) {
			if (States.STARTUP_COMPLETED.equals(state)) {
			    changeDaemonState(DaemonState.ONLINE);
			}
			else if (States.SHUTDOWN_INITIATED.equals(state)) {
			    changeDaemonState(DaemonState.OFFLINE);
			}
		}
	};
	
	private void changeDaemonState(DaemonState newState) {
	    _state = newState;
	    
	    // broadcast state change
	    Intent broadcastOfflineIntent = new Intent();
        broadcastOfflineIntent.setAction(de.tubs.ibr.dtn.Intent.STATE);
        broadcastOfflineIntent.putExtra("state", newState.name());
        broadcastOfflineIntent.addCategory(Intent.CATEGORY_DEFAULT);
        mService.sendBroadcast(broadcastOfflineIntent);
	}

	/**
	 * Create Hex String from byte array
	 * 
	 * @param data
	 * @return
	 */
	private static String toHex(byte[] data)
	{
		StringBuffer hexString = new StringBuffer();
		for (int i = 0; i < data.length; i++)
			hexString.append(Integer.toHexString(0xFF & data[i]));
		return hexString.toString();
	}

	/**
	 * Build unique endpoint id from Secure.ANDROID_ID
	 * 
	 * @param context
	 * @return
	 */
	public static SingletonEndpoint getUniqueEndpointID(Context context)
	{
		final String androidId = Secure.getString(context.getContentResolver(), Secure.ANDROID_ID);
		MessageDigest md;
		try {
			md = MessageDigest.getInstance("MD5");
			byte[] digest = md.digest(androidId.getBytes());
			return new SingletonEndpoint("dtn://android-" + toHex(digest).substring(4, 12) + ".dtn");
		} catch (NoSuchAlgorithmException e) {
			Log.e(TAG, "md5 not available");
		}
		return new SingletonEndpoint("dtn://android-" + androidId.substring(4, 12) + ".dtn");
	}

	/**
	 * Creates config for dtnd in specified path
	 * 
	 * @param context
	 */
	private void createConfig(Context context, String configPath)
	{
		// load preferences
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
		File config = new File(configPath);

		// remove old config file
		if (config.exists()) {
			config.delete();
		}

		try {
			FileOutputStream writer = context.openFileOutput("config", Context.MODE_PRIVATE);

			// initialize default values if configured set already
			de.tubs.ibr.dtn.daemon.Preferences.initializeDefaultPreferences(context);

			// set EID
			PrintStream p = new PrintStream(writer);
			p.println("local_uri = " + preferences.getString("endpoint_id", getUniqueEndpointID(context).toString()));
			p.println("routing = " + preferences.getString("routing", "default"));

			if (preferences.getBoolean("constrains_lifetime", false)) {
				p.println("limit_lifetime = 1209600");
			}

			if (preferences.getBoolean("constrains_timestamp", false)) {
				p.println("limit_predated_timestamp = 1209600");
			}

			// limit block size to 50 MB
			p.println("limit_blocksize = 50M");

			String secmode = preferences.getString("security_mode", "disabled");

			if (!secmode.equals("disabled")) {
				File sec_folder = new File(context.getFilesDir().getPath() + "/bpsec");
				if (!sec_folder.exists() || sec_folder.isDirectory()) {
					p.println("security_path = " + sec_folder.getPath());
				}
			}

			if (secmode.equals("bab")) {
				// write default BAB key to file
				String bab_key = preferences.getString("security_bab_key", "");
				File bab_file = new File(context.getFilesDir().getPath() + "/default-bab-key.mac");

				// remove old key file
				if (bab_file.exists()) bab_file.delete();

				FileOutputStream bab_output = context.openFileOutput("default-bab-key.mac", Context.MODE_PRIVATE);
				PrintStream bab_writer = new PrintStream(bab_output);
				bab_writer.print(bab_key);
				bab_writer.flush();
				bab_writer.close();

				if (bab_key.length() > 0) {
					// enable security extension: BAB
					p.println("security_level = 1");

					// add BAB key to the configuration
					p.println("security_bab_default_key = " + bab_file.getPath());
				}
			}

			if (preferences.getBoolean("checkIdleTimeout", false)) {
				p.println("tcp_idle_timeout = 30");
			}

			// set multicast address for discovery
			p.println("discovery_address = ff02::142 224.0.0.142");

			if (preferences.getBoolean("discovery_announce", true)) {
				p.println("discovery_announce = 1");
			} else {
				p.println("discovery_announce = 0");
			}

			String internet_ifaces = "";
			String ifaces = "";

			Map<String, ?> prefs = preferences.getAll();
			for (Map.Entry<String, ?> entry : prefs.entrySet()) {
				String key = entry.getKey();
				if (key.startsWith("interface_")) {
					if (entry.getValue() instanceof Boolean) {
						if ((Boolean) entry.getValue()) {
							String iface = key.substring(10, key.length());
							ifaces = ifaces + " " + iface;

							p.println("net_" + iface + "_type = tcp");
							p.println("net_" + iface + "_interface = " + iface);
							p.println("net_" + iface + "_port = 4556");
							internet_ifaces += iface + " ";
						}
					}
				}
			}

			p.println("net_interfaces = " + ifaces);
			p.println("net_internet = " + internet_ifaces);

			// storage path
			File blobPath = DaemonStorageUtils.getStoragePath("blob");
			if (blobPath != null) {
				p.println("blob_path = " + blobPath.getPath());

				// flush storage path
				File[] files = blobPath.listFiles();
				if (files != null) {
					for (File f : files) {
						f.delete();
					}
				}
			}

			File bundlePath = DaemonStorageUtils.getStoragePath("bundles");
			if (bundlePath != null) {
				p.println("storage_path = " + bundlePath.getPath());
			}

			boolean logToFile = preferences.getBoolean("log_enable_file", false);
			if (logToFile) {
    			File logPath = DaemonStorageUtils.getStoragePath("logs");
                if (logPath != null) {
                    logPath.mkdirs();
                    Calendar cal = Calendar.getInstance();
                    String time = "" + cal.get(Calendar.YEAR) + cal.get(Calendar.MONTH) + cal.get(Calendar.DAY_OF_MONTH) + cal.get(Calendar.DAY_OF_MONTH)
                            + cal.get(Calendar.HOUR) + cal.get(Calendar.MINUTE) + cal.get(Calendar.SECOND);
                    p.println("logfile = " + logPath.getPath() + File.separatorChar + "ibrdtn_" + time + ".log");
                }
			}

			/*
			 * if (preferences.getBoolean("connect_static", false)) { // add
			 * static connection p.println("static1_uri = " +
			 * preferences.getString("host_name", "dtn:none"));
			 * p.println("static1_address = " +
			 * preferences.getString("host_address", "0.0.0.0"));
			 * p.println("static1_proto = tcp"); p.println("static1_port = " +
			 * preferences.getString("host_port", "4556"));
			 * 
			 * // p.println("net_autoconnect = 120"); }
			 */

			// enable interface rebind
			p.println("net_rebind = yes");

			// flush the write buffer
			p.flush();

			// close the filehandle
			writer.close();
		} catch (IOException e) {
			Log.e(TAG, "Problem writing config", e);
		}
	}
}
