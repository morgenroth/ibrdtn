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
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.zip.GZIPOutputStream;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.provider.Settings.Secure;
import android.util.Log;
import de.tubs.ibr.dtn.api.DTNClient;
import de.tubs.ibr.dtn.api.Registration;
import de.tubs.ibr.dtn.api.ServiceNotAvailableException;
import de.tubs.ibr.dtn.api.SessionDestroyedException;
import de.tubs.ibr.dtn.api.SingletonEndpoint;

public class CollectorService extends Service {
	
	public final static String DELIVER_DATA = "de.tubs.ibr.dtn.stats.DELIVER_DATA";
	private final static String TAG = "CollectorService";
	private ExecutorService executor = null;
	
	private final SingletonEndpoint deliver_endpoint = new SingletonEndpoint("dtn://quorra.ibr.cs.tu-bs.de/datacollector");
	
	// DTN client to talk with the DTN service
	private DTNClient _client = null;

	@Override
	public IBinder onBind(Intent arg0) {
		return null;
	}

	@Override
	public void onCreate() {
		executor = Executors.newSingleThreadExecutor();
		
        // create a new registration
        Registration reg = new Registration("datacollector");
        
		try {
	        // create a new DTN client
	        _client = new DTNClient();
			_client.initialize(this, reg);
		} catch (ServiceNotAvailableException e) {
			// error
		}
		
		super.onCreate();
	}

	@Override
	public void onDestroy() {
		// unregister at the daemon
		_client.unregister();
		
		try {
			// stop executor
			executor.shutdown();
			
			// ... and wait until all jobs are done
			if (!executor.awaitTermination(10, TimeUnit.SECONDS)) {
				executor.shutdownNow();
			}
			
			// destroy DTN client
			_client.terminate();
		} catch (InterruptedException e) {
			Log.e(TAG, "Interrupted on service destruction.", e);
		}
		
		// clear all variables
		executor = null;
		_client = null;
		
		super.onDestroy();
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		if (intent == null) return super.onStartCommand(intent, flags, startId);
		if (!DELIVER_DATA.equals(intent.getAction())) return super.onStartCommand(intent, flags, startId);
		
		final int stopId = startId;

		Log.d(TAG, "Compress and deliver initiated");
		executor.execute(new Runnable() {

			@Override
			public void run() {
				// wait until the DTN service is connected
				
				final String androidId = Secure.getString(getContentResolver(), Secure.ANDROID_ID);

				Calendar gmt = new GregorianCalendar(TimeZone.getTimeZone("GMT"));
				String timestamp = Long.toString( gmt.getTimeInMillis() );
				
				String xmlHeader = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><events device=\"" + androidId + "\" timestamp=\"" + timestamp + "\">";
				String xmlFoooter = "</events>";

				synchronized(DataReceiver.datalock) {				
					try {
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
						
						File ef = new File(getFilesDir().getPath() + File.separatorChar + "events.dat");
						File efz = new File(getFilesDir().getPath() + File.separatorChar + "events.dat.gz");
						
						// send and delete zipped file
						Log.d(TAG, "Compressed log file: " + Long.toString( efz.length() ));
						
						ParcelFileDescriptor fd = ParcelFileDescriptor.open(efz, ParcelFileDescriptor.MODE_READ_ONLY);
						try {
							// send compressed data as bundle
							_client.getSession().send(deliver_endpoint, 172800, fd, efz.length());
							
							Log.d(TAG, "data file sent");
							
							// delete data file
							ef.delete();
						} catch (SessionDestroyedException e) {
							
						} catch (InterruptedException e) {
							
						}
						
						// delete compressed data file
						efz.delete();
						
					} catch (FileNotFoundException e) {
						// no data to send here
					} catch (IOException e) {
						e.printStackTrace();
					};
				};
				
				stopSelfResult(stopId);
			}
			
		});
		return Service.START_REDELIVER_INTENT;
	}
}
