package de.tubs.ibr.dtn.stats;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.TimeZone;
import java.util.zip.GZIPOutputStream;

import android.app.IntentService;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.preference.PreferenceManager;
import android.provider.Settings.Secure;
import android.util.Log;
import de.tubs.ibr.dtn.api.DTNClient;
import de.tubs.ibr.dtn.api.DTNClient.Session;
import de.tubs.ibr.dtn.api.Registration;
import de.tubs.ibr.dtn.api.ServiceNotAvailableException;
import de.tubs.ibr.dtn.api.SessionConnection;
import de.tubs.ibr.dtn.api.SessionDestroyedException;
import de.tubs.ibr.dtn.api.SingletonEndpoint;

public class CollectorService extends IntentService {
	
	public final static String DELIVER_DATA = "de.tubs.ibr.dtn.stats.DELIVER_DATA";
	public final static String REC_DATA = "de.tubs.ibr.dtn.stats.REC_DATA";
	private final static String TAG = "CollectorService";
	
	private final long LIMIT_FILESIZE = 500000;
	public static Object datalock = new Object(); 
	
	private final SingletonEndpoint deliver_endpoint = new SingletonEndpoint("dtn://quorra.ibr.cs.tu-bs.de/datacollector");
	
	// DTN client to talk with the DTN service
	private DTNClient _client = null;
	private DTNClient.Session _session = null;
	
	public CollectorService() {
		super(TAG);
	}
	
	@Override
	public void onDestroy() {
		// destroy DTN client
		if (_client != null) {
			// unregister at the daemon
			if (_session != null) {
				_session.destroy();
			}
	
			_client.terminate();
			
			// clear all variables
			_client = null;
		}
		
		super.onDestroy();
	}
	
	private void performDataRecording(String data) {
		synchronized(datalock) {
			try {
				FileOutputStream output = openFileOutput("events.dat", Context.MODE_PRIVATE | Context.MODE_APPEND);
				output.write(data.getBytes());
				output.close();
				
				// check if the last statistic bundle was send at least two hours ago
				SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(CollectorService.this);
				Calendar calendar = Calendar.getInstance();
				calendar.roll(Calendar.HOUR, false);
				
				if (calendar.getTimeInMillis() < prefs.getLong("stats_timestamp", 0)) return;
							
				File ef = new File(getCacheDir().getPath() + File.separatorChar + "events.dat");
				//if (ef.exists()) Log.d(TAG, "File size: " + Long.toString( ef.length() ));
				
				// compress and send the log file if the size is too large
				if (ef.length() > LIMIT_FILESIZE) {
					Calendar now = Calendar.getInstance();
					prefs.edit().putLong("stats_timestamp", now.getTimeInMillis()).commit(); 

					// open activity
					Intent i = new Intent(CollectorService.this, CollectorService.class);
					i.setAction(CollectorService.DELIVER_DATA);
					i.setData(Uri.fromFile(ef));
					startService(i);
				}
			} catch (FileNotFoundException e) {
				Log.e(TAG, "Failed to open data file for statistic logging", e);
			} catch (IOException e) {
				Log.e(TAG, "Failed to open data file for statistic logging", e);
			}
		}
	}
	
	private synchronized void performDataTransmission() {
		if (_client == null) {
			// create a new registration
			Registration reg = new Registration("datacollector");
			
			try {
				// create a new DTN client
				_client = new DTNClient(_session_listener);
				_client.initialize(this, reg);
			} catch (ServiceNotAvailableException e) {
				// error
			}
		}
		
		try {
			// wait until the client is connected
			while (_session == null) {
				if (_client.isDisconnected()) return;
				wait();
			}
		} catch (InterruptedException e) {
			return;
		}
		
		// send data
		performDataDelivery();
	}
	
	private SessionConnection _session_listener = new SessionConnection() {
		public void onSessionConnected(Session session) {
			synchronized(CollectorService.this) {
				_session = session;
				notifyAll();
			}
		}

		public void onSessionDisconnected() {
			synchronized(CollectorService.this) {
				_session = null;
				notifyAll();
			}
		}
	};
	
	private void performDataDelivery() {
		// wait until the DTN service is connected
		try {
			final String androidId = Secure.getString(getContentResolver(), Secure.ANDROID_ID);

			Calendar gmt = new GregorianCalendar(TimeZone.getTimeZone("GMT"));
			String timestamp = Long.toString( gmt.getTimeInMillis() );
			
			String xmlHeader = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><events device=\"" + androidId + "\" timestamp=\"" + timestamp + "\">";
			String xmlFoooter = "</events>";

			synchronized(datalock) {				
				FileInputStream input = openFileInput("events.dat");
				FileOutputStream output = openFileOutput("events.dat.gz", Context.MODE_PRIVATE);
				GZIPOutputStream zos = new GZIPOutputStream(new BufferedOutputStream(output));
				
				try {
					zos.write(xmlHeader.getBytes());
					
					byte[] b = new byte[4 * 1024];
					int read;
					while ((read = input.read(b)) != -1) {
						zos.write(b, 0, read);
					}
					
					zos.write(xmlFoooter.getBytes());
				} finally {
					zos.close();
				}
				
				File ef = new File(getCacheDir().getPath() + File.separatorChar + "events.dat");
				File efz = new File(getCacheDir().getPath() + File.separatorChar + "events.dat.gz");
				
				// send and delete zipped file
				Log.d(TAG, "Compressed log file: " + Long.toString( efz.length() ));
				
				ParcelFileDescriptor fd = ParcelFileDescriptor.open(efz, ParcelFileDescriptor.MODE_READ_ONLY);
				try {
					if (_session != null) {
						// send compressed data as bundle
						_session.send(deliver_endpoint, 172800, fd);
						
						Log.d(TAG, "data file sent");
					}
					
					// delete data file
					ef.delete();
				} catch (SessionDestroyedException e) {
					
				}
				
				// delete compressed data file
				efz.delete();
			};
		} catch (FileNotFoundException e) {
			// no data to send here
		} catch (IOException e) {
			Log.e(TAG, "error", e);
		}
	}

	@Override
	protected void onHandleIntent(Intent intent) {
		if (intent == null) return;;

		if ( DELIVER_DATA.equals( intent.getAction() ) ) {
			if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "Compress and deliver");
			performDataTransmission();
		} else if ( REC_DATA.equals( intent.getAction() ) ) {
			if ( intent.hasExtra("xmldata") ) {
				final String data = intent.getStringExtra("xmldata");
				if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "Data recording: " + data);
				performDataRecording(data);
			}
		}
	}
}
