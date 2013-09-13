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
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.wifi.WifiManager;
import android.preference.PreferenceManager;
import android.provider.Settings.Secure;
import android.util.Log;
import de.tubs.ibr.dtn.DaemonState;
import de.tubs.ibr.dtn.api.Node;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.swig.DaemonRunLevel;
import de.tubs.ibr.dtn.swig.NativeDaemon;
import de.tubs.ibr.dtn.swig.NativeDaemonCallback;
import de.tubs.ibr.dtn.swig.NativeDaemonException;
import de.tubs.ibr.dtn.swig.NativeEventCallback;
import de.tubs.ibr.dtn.swig.NativeNode;
import de.tubs.ibr.dtn.swig.NativeStats;
import de.tubs.ibr.dtn.swig.StringVec;

public class DaemonProcess {
	private final static String TAG = "DaemonProcess";

	private NativeDaemon mDaemon = null;
	private DaemonProcessHandler mHandler = null;
	private Context mContext = null;
	private DaemonState _state = DaemonState.OFFLINE;
	
    private WifiManager.MulticastLock _mcast_lock = null;

	private final static String GNUSTL_NAME = "gnustl_shared";
	private final static String CRYPTO_NAME = "crypto";
	private final static String SSL_NAME = "ssl";
	private final static String IBRCOMMON_NAME = "ibrcommon";
	private final static String IBRDTN_NAME = "ibrdtn";
	private final static String DTND_NAME = "dtnd";
	private final static String ANDROID_GLUE_NAME = "android-glue";

    // CloudUplink Parameter
    private static final SingletonEndpoint __CLOUD_EID__ = new SingletonEndpoint(
            "dtn://cloud.dtnbone.dtn");
    private static final String __CLOUD_PROTOCOL__ = "tcp";
    private static final String __CLOUD_ADDRESS__ = "134.169.35.130"; // quorra.ibr.cs.tu-bs.de";
    private static final String __CLOUD_PORT__ = "4559";
    
    public interface OnRestartListener {
        public void OnStop();
        public void OnReloadConfiguration();
        public void OnStart();
    };

	/**
	 * Loads all shared libraries in the right order with System.loadLibrary()
	 */
	private static void loadLibraries()
	{
		try
		{
			System.loadLibrary(GNUSTL_NAME);

			System.loadLibrary(CRYPTO_NAME);
			System.loadLibrary(SSL_NAME);

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

	public DaemonProcess(Context context, DaemonProcessHandler handler) {
		this.mDaemon = new NativeDaemon(mDaemonCallback, mEventCallback);
		this.mContext = context;
		this.mHandler = handler;
	}

	public String[] getVersion() {
        StringVec version = mDaemon.getVersion();
        return new String[] { version.get(0), version.get(1) };
	}
	
	public synchronized NativeStats getStats() {
	    return mDaemon.getStats();
	}
	
	public synchronized List<Node> getNeighbors() {
        List<Node> ret = new LinkedList<Node>();
        StringVec neighbors = mDaemon.getNeighbors();
        for (int i = 0; i < neighbors.size(); i++) {
        	String eid = neighbors.get(i);
        	
        	try {
            	// get extended info
				NativeNode nn = mDaemon.getInfo(eid);
				
            	Node n = new Node();
            	n.endpoint = new SingletonEndpoint(eid);
            	n.type = nn.getType().toString();
                ret.add(n);
			} catch (NativeDaemonException e) { }
        }

        return ret;
	}
	
	public synchronized void clearStorage() {
		mDaemon.clearStorage();
	}
	
	public DaemonState getState() {
	    return _state;
	}
	
    private void setState(DaemonState newState) {
        if (_state.equals(newState)) return;
        _state = newState;
        mHandler.onStateChanged(_state);
    }
    
    public synchronized void initiateConnection(String endpoint) {
    	if (getState().equals(DaemonState.ONLINE)) {
    		mDaemon.initiateConnection(endpoint);
    	}
    }
    
    public synchronized void initialize() {
    	// lower the thread priority
    	android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_BACKGROUND);
    	
    	// get daemon preferences
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this.mContext);
        
        // listen to preference changes
        preferences.registerOnSharedPreferenceChangeListener(_pref_listener);

        // enable debug based on prefs
        int logLevel = 0;
        try {
            logLevel = Integer.valueOf(preferences.getString("log_options", "0"));
        } catch (java.lang.NumberFormatException e) {
            // invalid number
        }
        
        int debugVerbosity = 0;
        try {
            debugVerbosity = Integer.valueOf(preferences.getString("log_debug_verbosity", "0"));
        } catch (java.lang.NumberFormatException e) {
            // invalid number
        }
        
        // disable debugging if the log level is lower than 3
        if (logLevel < 3) debugVerbosity = 0;
        
        // set logging options
        mDaemon.setLogging("Core", logLevel);

        // set logfile options
        String logFilePath = null;
        
        if (preferences.getBoolean("log_enable_file", false)) {
            File logPath = DaemonStorageUtils.getStoragePath("logs");
            if (logPath != null) {
                logPath.mkdirs();
                Calendar cal = Calendar.getInstance();
                String time = "" + cal.get(Calendar.YEAR) + cal.get(Calendar.MONTH) + cal.get(Calendar.DAY_OF_MONTH) + cal.get(Calendar.DAY_OF_MONTH)
                        + cal.get(Calendar.HOUR) + cal.get(Calendar.MINUTE) + cal.get(Calendar.SECOND);
                
                logFilePath = logPath.getPath() + File.separatorChar + "ibrdtn_" + time + ".log";
            }
        }

        if (logFilePath != null) {
            // enable file logging
            mDaemon.setLogFile(logFilePath, logLevel);
        } else {
            // disable file logging
            mDaemon.setLogFile("", 0);
        }

        // set debug verbosity
        mDaemon.setDebug(debugVerbosity);
        
        // initialize daemon configuration
        onConfigurationChanged();
        
        try {
            mDaemon.init(DaemonRunLevel.RUNLEVEL_API);
        } catch (NativeDaemonException e) {
            Log.e(TAG, "error while initializing the daemon process", e);
        }
    }
	
	public synchronized void start() {
	    WifiManager wifi_manager = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);

        // listen to multicast packets
        _mcast_lock = wifi_manager.createMulticastLock(TAG);
        _mcast_lock.acquire();

        // reload daemon configuration
        onConfigurationChanged();
        
	    try {
            mDaemon.init(DaemonRunLevel.RUNLEVEL_ROUTING_EXTENSIONS);
        } catch (NativeDaemonException e) {
            Log.e(TAG, "error while starting the daemon process", e);
        }
	}
	
	public synchronized void stop() {
	    // stop the running daemon
	    try {
            mDaemon.init(DaemonRunLevel.RUNLEVEL_API);
        } catch (NativeDaemonException e) {
            Log.e(TAG, "error while stopping the daemon process", e);
        }

	    // release multicast lock
        if (_mcast_lock != null) {
            _mcast_lock.release();
            _mcast_lock = null;
        }
	}
	
	public synchronized void restart(Integer runlevel, OnRestartListener listener) {
	    // restart the daemon
        DaemonRunLevel restore = mDaemon.getRunLevel();
        DaemonRunLevel rl = DaemonRunLevel.swigToEnum(runlevel);
        
        // do not restart if the current runlevel is below or equal
        if (restore.swigValue() <= rl.swigValue()) {
            // reload configuration
            onConfigurationChanged();
            if (listener != null) listener.OnReloadConfiguration();
        }
        
	    try {
	        // bring the daemon down
	        mDaemon.init(rl);
	        if (listener != null) listener.OnStop();
	        
	        // reload configuration
	        onConfigurationChanged();
	        if (listener != null) listener.OnReloadConfiguration();
	        
	        // restore the old runlevel
	        mDaemon.init(restore);
	        if (listener != null) listener.OnStart();
	    } catch (NativeDaemonException e) {
            Log.e(TAG, "error while restarting the daemon process", e);
        }
	}
	
    public synchronized void destroy() {
        // get daemon preferences
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this.mContext);
        
        // unlisten to preference changes
        preferences.unregisterOnSharedPreferenceChangeListener(_pref_listener);
        
        // stop the running daemon
        try {
            mDaemon.init(DaemonRunLevel.RUNLEVEL_ZERO);
        } catch (NativeDaemonException e) {
            Log.e(TAG, "error while destroying the daemon process", e);
        }
    }
    
    final HashMap<String, DaemonRunLevel> mRestartMap = initializeRestartMap();
    
    private HashMap<String, DaemonRunLevel> initializeRestartMap() {
        HashMap<String, DaemonRunLevel> ret = new HashMap<String, DaemonRunLevel>();
        
        ret.put("endpoint_id", DaemonRunLevel.RUNLEVEL_CORE);
        ret.put("routing", DaemonRunLevel.RUNLEVEL_ROUTING_EXTENSIONS);
        ret.put("interface_", DaemonRunLevel.RUNLEVEL_NETWORK);
        ret.put("discovery_announce", DaemonRunLevel.RUNLEVEL_NETWORK);
        ret.put("checkIdleTimeout", DaemonRunLevel.RUNLEVEL_NETWORK);
        ret.put("checkFragmentation", DaemonRunLevel.RUNLEVEL_NETWORK);
        ret.put("timesync_mode", DaemonRunLevel.RUNLEVEL_API);
        ret.put("storage_mode", DaemonRunLevel.RUNLEVEL_CORE);
        
        return ret;
    }
    
    final HashSet<String> mConfigurationSet = initializeConfigurationSet();
    
    private HashSet<String> initializeConfigurationSet() {
        HashSet<String> ret = new HashSet<String>();
              
        ret.add("constrains_lifetime");
        ret.add("constrains_timestamp");
        ret.add("security_mode");
        ret.add("security_bab_key");
        ret.add("log_options");
        ret.add("log_debug_verbosity");
        ret.add("log_enable_file");
        ret.add("storage_mode");
        
        return ret;
    }
    
    private SharedPreferences.OnSharedPreferenceChangeListener _pref_listener = new SharedPreferences.OnSharedPreferenceChangeListener() {
        @Override
        public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
            Log.d(TAG, "Preferences has changed " + key);

            if (mRestartMap.containsKey(key)) {
                // check runlevel and restart some runlevels if necessary
                final Intent intent = new Intent(DaemonProcess.this.mContext, DaemonService.class);
                intent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_RESTART);
                intent.putExtra("runlevel", mRestartMap.get(key).swigValue() - 1);
                DaemonProcess.this.mContext.startService(intent);
            }
            else if (key.equals("enabledSwitch"))
            {
                if (prefs.getBoolean(key, false)) {
                    // startup the daemon process
                    final Intent intent = new Intent(DaemonProcess.this.mContext, DaemonService.class);
                    intent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_STARTUP);
                    DaemonProcess.this.mContext.startService(intent);
                } else {
                    // shutdown the daemon
                    final Intent intent = new Intent(DaemonProcess.this.mContext, DaemonService.class);
                    intent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_SHUTDOWN);
                    DaemonProcess.this.mContext.startService(intent);
                }
            }
            else if (key.startsWith("interface_"))
            {
                // a interface has been removed or added
                // check runlevel and restart some runlevels if necessary
                final Intent intent = new Intent(DaemonProcess.this.mContext, DaemonService.class);
                intent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_RESTART);
                intent.putExtra("runlevel", mRestartMap.get("interface_").swigValue() - 1);
                DaemonProcess.this.mContext.startService(intent);
            }
            else if (key.equals("cloud_uplink"))
            {
                Log.d(TAG, key + " == " + String.valueOf( prefs.getBoolean(key, false) ));
                synchronized(DaemonProcess.this) {
                    if (prefs.getBoolean(key, false)) {
                        mDaemon.addConnection(__CLOUD_EID__.toString(),
                                __CLOUD_PROTOCOL__, __CLOUD_ADDRESS__, __CLOUD_PORT__);
                    } else {
                        mDaemon.removeConnection(__CLOUD_EID__.toString(),
                                __CLOUD_PROTOCOL__, __CLOUD_ADDRESS__, __CLOUD_PORT__);
                    }
                }
            }
            else if (key.startsWith("log_options"))
            {
                Log.d(TAG, key + " == " + prefs.getString(key, "<not set>"));
                
                int logLevel = Integer.valueOf(prefs.getString("log_options", "0"));
                int debugVerbosity = Integer.valueOf(prefs.getString("log_debug_verbosity", "0"));

                // disable debugging if the log level is lower than 3
                if (logLevel < 3) debugVerbosity = 0;

                synchronized(DaemonProcess.this) {
                    // set logging options
                    mDaemon.setLogging("Core", logLevel);

                    // set debug verbosity
                    mDaemon.setDebug( debugVerbosity );
                }
            }
            else if (key.startsWith("log_debug_verbosity"))
            {
                Log.d(TAG, key + " == " + prefs.getString(key, "<not set>"));
                
                int logLevel = Integer.valueOf(prefs.getString("log_options", "0"));
                int debugVerbosity = Integer.valueOf(prefs.getString("log_debug_verbosity", "0"));
                
                // disable debugging if the log level is lower than 3
                if (logLevel < 3) debugVerbosity = 0;
                
                synchronized(DaemonProcess.this) {
                    // set debug verbosity
                    mDaemon.setDebug( debugVerbosity );
                }
            }
            else if (key.startsWith("log_enable_file"))
            {
                Log.d(TAG, key + " == " + prefs.getBoolean(key, false));
                
                // set logfile options
                String logFilePath = null;
                
                if (prefs.getBoolean("log_enable_file", false)) {
                    File logPath = DaemonStorageUtils.getStoragePath("logs");
                    if (logPath != null) {
                        logPath.mkdirs();
                        Calendar cal = Calendar.getInstance();
                        String time = "" + cal.get(Calendar.YEAR) + cal.get(Calendar.MONTH) + cal.get(Calendar.DAY_OF_MONTH) + cal.get(Calendar.DAY_OF_MONTH)
                                + cal.get(Calendar.HOUR) + cal.get(Calendar.MINUTE) + cal.get(Calendar.SECOND);
                        
                        logFilePath = logPath.getPath() + File.separatorChar + "ibrdtn_" + time + ".log";
                    }
                }

                synchronized(DaemonProcess.this) {
                    if (logFilePath != null) {
                        int logLevel = Integer.valueOf(prefs.getString("log_options", "0"));
                        
                        // enable file logging
                        mDaemon.setLogFile(logFilePath, logLevel);
                    } else {
                        // disable file logging
                        mDaemon.setLogFile("", 0);
                    }
                }
            } else if (mConfigurationSet.contains(key)) {
                // default action
                onConfigurationChanged();
            }
        }
    };
	
	private void onConfigurationChanged() {
        String configPath = mContext.getFilesDir().getPath() + "/" + "config";

        // create configuration file
        createConfig(mContext, configPath);
        
        // set configuration file
        mDaemon.setConfigFile(configPath);
	}
	
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
                	neighborIntent.putExtra("action", action);
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
            mHandler.onEvent(event);

            if (neighborIntent != null) {
                mHandler.onEvent(neighborIntent);
                mHandler.onNeighborhoodChanged();
            }

            if (Log.isLoggable(TAG, Log.DEBUG)) {
                Log.d(TAG, "EVENT intent broadcasted: " + eventName + "; Action: " + action);
            }
        }
	};
	
	private NativeDaemonCallback mDaemonCallback = new NativeDaemonCallback() {
		@Override
		public void levelChanged(DaemonRunLevel level) {
			if (DaemonRunLevel.RUNLEVEL_ROUTING_EXTENSIONS.equals(level)) {
			    // enable cloud-uplink
			    SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(DaemonProcess.this.mContext);
                if (prefs.getBoolean("cloud_uplink", false)) {
                    mDaemon.addConnection(__CLOUD_EID__.toString(),
                            __CLOUD_PROTOCOL__, __CLOUD_ADDRESS__, __CLOUD_PORT__);
                }
			    
			    setState(DaemonState.ONLINE);
			}
			else if (DaemonRunLevel.RUNLEVEL_API.equals(level)) {
			    setState(DaemonState.OFFLINE);
			}
		}
	};
	
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
			
			// enable traffic stats
			p.println("stats_traffic = yes");

			if (preferences.getBoolean("constrains_lifetime", false)) {
				p.println("limit_lifetime = 1209600");
			}

			if (preferences.getBoolean("constrains_timestamp", false)) {
				p.println("limit_predated_timestamp = 1209600");
			}

			// limit block size to 50 MB
			p.println("limit_blocksize = 250M");
			p.println("limit_foreign_blocksize = 50M");

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
			
			String timesyncmode = preferences.getString("timesync_mode", "disabled");
			
			if (timesyncmode.equals("master")) {
                p.println("time_reference = yes");
                p.println("time_discovery_announcements = yes");
                p.println("time_synchronize = no");
                p.println("time_set_clock = no");
			} else if (timesyncmode.equals("slave")) {
			    p.println("time_reference = no");
                p.println("time_discovery_announcements = yes");
                p.println("time_synchronize = yes");
			    p.println("time_set_clock = no");
			    p.println("#time_sigma = 1.001");
			    p.println("#time_psi = 0.9");
			    p.println("#time_sync_level = 0.15");
			}

			if (preferences.getBoolean("checkIdleTimeout", false)) {
				p.println("tcp_idle_timeout = 30");
			}
			
			if (preferences.getBoolean("checkFragmentation", true)) {
			    p.println("fragmentation = yes");
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

			String storage_mode = preferences.getString( "storage_mode", "disk-persistent" );
			if ("disk".equals( storage_mode ) || "disk-persistent".equals( storage_mode )) {
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
			}

			if ("disk-persistent".equals( storage_mode )) {
    			File bundlePath = DaemonStorageUtils.getStoragePath("bundles");
    			if (bundlePath != null) {
    				p.println("storage_path = " + bundlePath.getPath());
    			}
			}

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
