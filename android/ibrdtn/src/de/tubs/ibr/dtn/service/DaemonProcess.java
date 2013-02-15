/*
 * DaemonProcess.java
 * 
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
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

import ibrdtn.api.APIConnection;
import ibrdtn.api.SocketAPIConnection;
import ibrdtn.api.object.SingletonEndpoint;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Map;

import de.tubs.ibr.dtn.service.StreamLogger.StreamLoggerListener;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.PackageInfo;
import android.content.res.AssetManager;
import android.preference.PreferenceManager;
import android.provider.Settings.Secure;
import android.util.Log;

public class DaemonProcess extends Thread {

//	private final static String TAG = "DaemonProcess";
//	private ProcessListener _listener = null;
//	
//	private ProcessBuilder _builder = null;
//	private Process _proc = null;
//	private Context _context = null;
//	
//	public interface ProcessListener {
//		public void onProcessStart();
//		public void onProcessStop();
//		public void onProcessError();
//		public void onProcessLog(String log);
//	}
//	
//	private static String toHex(byte[] data) {
//		// Create Hex String
//		StringBuffer hexString = new StringBuffer();
//		for (int i=0; i < data.length; i++)
//		    hexString.append(Integer.toHexString(0xFF & data[i]));
//		return hexString.toString();
//	}
//	
//	public static SingletonEndpoint getUniqueEndpointID(Context context) {
//		final String androidId = Secure.getString(context.getContentResolver(), Secure.ANDROID_ID);
//		MessageDigest md;
//		try {
//			md = MessageDigest.getInstance("MD5");
//			byte[] digest = md.digest(androidId.getBytes());
//			return new SingletonEndpoint("dtn://android-" + toHex(digest).substring(4, 12) + ".dtn");
//		} catch (NoSuchAlgorithmException e) {
//			// md5 not available
//		}
//		return new SingletonEndpoint("dtn://android-" + androidId.substring(4, 12) + ".dtn");
//	}
//	
//	// if set to true, use unix domain sockets for API connections.
//	private final Boolean _use_unix_socket = false;
//	
//	public DaemonProcess(Context context, ProcessListener listener)
//	{
//		this._context = context;
//		this._listener = listener;
//	}
//	
//	public APIConnection getAPIConnection()
//	{
//		if (_use_unix_socket) {
//			return new UnixAPIConnection( _context.getFilesDir().getPath() + "/ibrdtn.sock" );
//		} else {
//			return new SocketAPIConnection();
//		}
//	}
//	
//	public void kill()
//	{
//		if (_proc != null)
//		{
//			_proc.destroy();
//			_proc = null;
//		}
//	}
//	
//	public void start(Context context)
//	{
//		// return if no builder was created
//		if (_builder != null) return;
//		
//		// check for mandatory dtnd binaries
//		check_executables(context);
//		
//		// create configuration file
//		create_config(context);
//		
//		// instantiate a new process builder for the daemon
//		_builder = new ProcessBuilder();
//		
//		// set executable and command line args
//		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
//		if (preferences.getBoolean("debug", false))
//		{
//			_builder.command(context.getFilesDir().getPath() + "/dtnd", "-c", context.getFilesDir().getPath() + "/config", "-d", "99");
//		}
//		else
//		{
//			_builder.command(context.getFilesDir().getPath() + "/dtnd", "-c", context.getFilesDir().getPath() + "/config");
//		}
//		
//		// redirect error messages to the standard output
//		_builder.redirectErrorStream(true);
//
//		// start this thread
//		start();
//	}
//		
//	public void run()
//	{
//		// return if no builder was created
//		if (_builder == null) return;
//		
//		try {
//			// lower the thread priority
//			setPriority(MIN_PRIORITY);
//			
//			// kill previous process
//			kill();
//			
//			// run the daemon
//			_proc = _builder.start();
//
//		    StreamLoggerListener log_listener = new StreamLoggerListener() {
//				public void onLog(String TAG, String log) {
//					if (_listener != null) _listener.onProcessLog(log);
//				}
//		    };
//		    
//		    StreamLogger std_logger = new StreamLogger("dtnd", _proc.getInputStream(), log_listener);
//		    StreamLogger err_logger = new StreamLogger("dtnd_error", _proc.getErrorStream(), log_listener);
//		    
//		    if (_listener != null) _listener.onProcessStart();
//		    
//		    Thread std_log_thread = new Thread(std_logger);
//		    Thread err_log_thread = new Thread(err_logger);
//		    
//		    // start logging processes
//		    std_log_thread.start();
//		    err_log_thread.start();
//		    
//		    // wait until the logging processes are finished
//		    std_log_thread.join();
//		    err_log_thread.join();
//		    
//		    if (std_logger.hasErrorOnExit() || err_logger.hasErrorOnExit()) {
//		    	if (_listener != null) _listener.onProcessError();
//		    } else {
//		    	if (_listener != null) _listener.onProcessStop();		    	
//		    }
//		} catch (IOException e) {
//			Log.e(TAG, "Unable to run daemon: " + e.toString());
//			if (_listener != null) _listener.onProcessError();
//		} catch (InterruptedException e) {
//			// join has been interrupted
//		}
//	}
//	
//	private Boolean check_binary_version(File binary, String version)
//	{
//		if (!binary.exists()) return false;
//		
//		try {
//			Process dtnd_version = Runtime.getRuntime().exec(binary.getAbsolutePath() + " --version");
//			BufferedReader input = new BufferedReader( new InputStreamReader( dtnd_version.getInputStream() ) );
//			String data = input.readLine();
//			
//			// could not get dtnd version
//			if (data == null) return false;
//			
//			// check if the version is equal
//			return (data.contains(version));
//		} catch (IOException e) { };
//		
//		return false;
//	}
//	
//	private void check_executables(Context context)
//	{
//		File binary = new File(context.getFilesDir().getPath() + "/" + "dtnd");
//		
//		// get shared preferences with binary version
//		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
//		int version_code = 0;
//		
//		try {
//			// check for the dtnd binary
//			PackageInfo pinfo = context.getPackageManager().getPackageInfo(context.getPackageName(), 0);
//			version_code = pinfo.versionCode;
//			
//			if (prefs.getInt("binary_version", 0) == pinfo.versionCode)
//			{
//				// strip versionCode off the versionName
//				String[] daemonVersion = pinfo.versionName.split("-");
//				
//				if (check_binary_version(binary, daemonVersion[0]))
//				{
//					Log.i("IBR-DTN", "package version " + pinfo.versionName + " is up to date");
//					return;
//				}
//			}
//			
//			Log.i("IBR-DTN", "package version " + pinfo.versionName + " is outdated");
//			
//		} catch (android.content.pm.PackageManager.NameNotFoundException e) {
//			Log.e("IBR-DTN", "could not get package version");
//		}
//		
//		try {
//			// delete the binary if exists
//			if (binary.exists()) binary.delete();
//			
//			Log.i("IBR-DTN", "Binary 'dtnd' invalid. Try to install it to " + binary.getAbsolutePath());
//			
//			AssetManager assetManager = context.getAssets();
//			
//			InputStream stream;
//			Log.i("DaemonProcess", "Detect system architecture: " + System.getProperty("os.arch"));
//			
//			if (System.getProperty("os.arch").contains("i686"))
//			{
//				Log.i("DaemonProcess", "Install x86 binary");
//				stream = assetManager.open("dtnd.x86");
//			}
//			else
//			{
//				Log.i("DaemonProcess", "Install ARM binary");
//				stream = assetManager.open("dtnd");
//			}
//			
//			FileOutputStream writer = context.openFileOutput("dtnd", Context.MODE_PRIVATE);
//			
//			// copy the binary to the local file
//			copy(stream, writer, 4096);
//			
//			// close the filehandle
//			writer.close();
//			
//			// set executable
//			binary.setExecutable(true);
//			
//			// mark this version as installed
//			Editor e = prefs.edit();
//			e.putInt("binary_version", version_code);
//			e.commit();
//			
//		} catch (IOException e) { }
//	}
//	
//	private void create_config(Context context)
//	{
//		// load preferences
//		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
//		File config = new File(context.getFilesDir().getPath() + "/" + "config");
//		
//		// remove old config file
//		if (config.exists())
//		{
//			config.delete();
//		}
//		
//		try {
//			FileOutputStream writer = context.openFileOutput("config", Context.MODE_PRIVATE);
//			
//			// initialize default values if configured set already
//			de.tubs.ibr.dtn.daemon.Preferences.initializeDefaultPreferences(context);
//
//			// set EID
//			PrintStream p = new PrintStream(writer);
//			p.println("local_uri = " + preferences.getString("endpoint_id", DaemonProcess.getUniqueEndpointID(context).toString()));
//			p.println("routing = " + preferences.getString("routing", "default"));
//			
//			if (_use_unix_socket) {
//				// configure local API interface
//				p.println("api_socket = " + _context.getFilesDir().getPath() + "/ibrdtn.sock");
//			}
//			
//			if (preferences.getBoolean("constrains_lifetime", false))
//			{
//				p.println("limit_lifetime = 1209600");
//			}
//			
//			if (preferences.getBoolean("constrains_timestamp", false))
//			{
//				p.println("limit_predated_timestamp = 1209600");
//			}
//			
//			// limit block size to 50 MB
//			p.println("limit_blocksize = 50M");
//			
//			String secmode = preferences.getString("security_mode", "disabled");
//			
//			if (!secmode.equals("disabled")) {
//				File sec_folder = new File(context.getFilesDir().getPath() + "/bpsec");
//				if (!sec_folder.exists() || sec_folder.isDirectory()) {
//					p.println("security_path = " + sec_folder.getPath());
//				}
//			}
//			
//			if (secmode.equals("bab"))
//			{
//				// write default BAB key to file
//				String bab_key = preferences.getString("security_bab_key", "");
//				File bab_file = new File(context.getFilesDir().getPath() + "/default-bab-key.mac");
//
//				// remove old key file
//				if (bab_file.exists()) bab_file.delete();
//				
//				FileOutputStream bab_output = context.openFileOutput("default-bab-key.mac", Context.MODE_PRIVATE);
//				PrintStream bab_writer = new PrintStream(bab_output);
//				bab_writer.print(bab_key);
//				bab_writer.flush();
//				bab_writer.close();
//				
//				if (bab_key.length() > 0) {
//					// enable security extension: BAB
//					p.println("security_level = 1");
//					
//					// add BAB key to the configuration
//					p.println("security_bab_default_key = " + bab_file.getPath());
//				}
//			}
//			
//			if  (preferences.getBoolean("checkIdleTimeout", false))
//			{
//				p.println("tcp_idle_timeout = 30");
//			}
//
//			// set multicast address for discovery
//			p.println("discovery_address = ff02::142 224.0.0.142");
//			
//			if (preferences.getBoolean("discovery_announce", true))
//			{
//				p.println("discovery_announce = 1");
//			}
//			else
//			{
//				p.println("discovery_announce = 0");
//			}
//			
//			String internet_ifaces = "";
//			String ifaces = "";
//			
//			Map<String, ?> prefs = preferences.getAll();
//			for (Map.Entry<String, ?> entry : prefs.entrySet() )
//		    {
//				String key = entry.getKey();
//	            if (key.startsWith("interface_"))
//	            {
//	            	if (entry.getValue() instanceof Boolean)
//	            	{
//	            		if ((Boolean)entry.getValue())
//	            		{
//	    	            	String iface = key.substring(10, key.length());
//	    	            	ifaces = ifaces + " " + iface;
//	    	
//	    		    		p.println("net_" + iface + "_type = tcp");
//	    		            p.println("net_" + iface + "_interface = " + iface);
//	    		            p.println("net_" + iface + "_port = 4556");
//	    		            internet_ifaces += iface + " ";
//	            		}
//	            	}
//	            }
//		    }
//			
//			p.println("net_interfaces = " + ifaces);
//			p.println("net_internet = " + internet_ifaces);
//			
//			// storage path
//			File blobPath = DaemonManager.getStoragePath("blob");
//			if (blobPath != null)
//			{
//				p.println("blob_path = " + blobPath.getPath());
//				
//				// flush storage path
//				File[] files = blobPath.listFiles();
//				if (files != null)
//				{
//					for (File f : files)
//					{
//						f.delete();
//					}
//				}
//			}
//			
//			File bundlePath = DaemonManager.getStoragePath("bundles");
//			if (bundlePath != null)
//			{
//				p.println("storage_path = " + bundlePath.getPath());
//			}
//			
///*
//			if (preferences.getBoolean("connect_static", false))
//			{
//				// add static connection
//				p.println("static1_uri = " + preferences.getString("host_name", "dtn:none"));
//				p.println("static1_address = " + preferences.getString("host_address", "0.0.0.0"));
//				p.println("static1_proto = tcp");
//				p.println("static1_port = " + preferences.getString("host_port", "4556"));
//		
//				// p.println("net_autoconnect = 120");
//			}
//*/
//
//			// enable interface rebind
//			p.println("net_rebind = yes");
//			
//			// flush the write buffer
//			p.flush();
//			
//			// close the filehandle
//			writer.close();
//		} catch (IOException e) { }
//	}
//	
//	private static void copy(InputStream in, OutputStream out, int bufferSize) throws IOException
//	{
//	   // Read bytes and write to destination until eof
//
//	   byte[] buf = new byte[bufferSize];
//	   int len = 0;
//	   while ((len = in.read(buf)) >= 0)
//	   {
//	      out.write(buf, 0, len);
//	   }
//	}
}
