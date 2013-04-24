package de.tubs.ibr.dtn.sendfile;

import java.io.File;
import java.io.FileNotFoundException;

import android.app.IntentService;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import android.util.Log;
import de.tubs.ibr.dtn.DTNService;
import de.tubs.ibr.dtn.api.Block;
import de.tubs.ibr.dtn.api.Bundle;
import de.tubs.ibr.dtn.api.DTNSession;
import de.tubs.ibr.dtn.api.DTNSessionCallback;
import de.tubs.ibr.dtn.api.EID;
import de.tubs.ibr.dtn.api.GroupEndpoint;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.api.TransferMode;

public class SendService extends IntentService {
	
	private static final String TAG = "SendService";
	private DTNService service = null;
	private DTNSession session = null;

	public SendService(String name) {
		super(name);
	}
	
	private BroadcastReceiver _receiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			if (intent.getAction().equals(de.tubs.ibr.dtn.Intent.REGISTRATION))
			{
				try {
					session = service.getSession(SendService.class.getName());
					
					Log.i(TAG, "registration successful");
					
					synchronized(SendService.this) {
						// notify the waiting onIntent call
						SendService.this.notifyAll();
					}
				} catch (RemoteException e) {
					Log.e(TAG, "registration failed", e);
				}
			}
		}
	};
	
	private ServiceConnection mConnection = new ServiceConnection() {
		public void onServiceConnected(ComponentName name, IBinder service) {
			// register to daemon intents
			IntentFilter rfilter = new IntentFilter(de.tubs.ibr.dtn.Intent.REGISTRATION);
			rfilter.addCategory(SendService.class.getName());
			registerReceiver(_receiver, rfilter );
			
			SendService.this.service = DTNService.Stub.asInterface(service);
			
			synchronized(SendService.this) {
				// notify the waiting onIntent call
				SendService.this.notifyAll();
			}
		}

		public void onServiceDisconnected(ComponentName name) {
			SendService.this.service = null;
		}
	};

	@Override
	public void onCreate() {
		super.onCreate();
		
		// set service to null
		service = null;
		
		Intent bindIntent = new Intent(DTNService.class.getName());
		bindService(bindIntent, mConnection, Context.BIND_AUTO_CREATE);
	}

	@Override
	public void onDestroy() {
        // Detach our existing connection.
		unbindService(mConnection);
		
		super.onDestroy();
	}
	
	private DTNSessionCallback _session_cb = new DTNSessionCallback() {

		@Override
		public IBinder asBinder() {
			return null;
		}

		@Override
		public void endBlock() throws RemoteException {
		}

		@Override
		public void endBundle() throws RemoteException {
		}

		@Override
		public ParcelFileDescriptor fd() throws RemoteException {
			return null;
		}

		@Override
		public void payload(byte[] arg0) throws RemoteException {
		}

		@Override
		public void progress(long pos, long length) throws RemoteException {
			// TODO: update notification
			Log.d(TAG, "progress " + pos + "/" + length);
		}

		@Override
		public TransferMode startBlock(Block arg0) throws RemoteException {
			return null;
		}

		@Override
		public void startBundle(Bundle arg0) throws RemoteException {
		}
	};

	@Override
	protected void onHandleIntent(Intent intent) {
		// return if there is no action
		if (intent.getAction() == null) return;
		
		try {
			synchronized(this) {
				// wait until the service is connected
				while (service == null) wait();
				
				Log.d(TAG, "service connected");
				
				// wait until the session is available
				while (session == null) wait();
				
				Log.d(TAG, "session available");
			}
			
			// if file transfer is requested
			if (de.tubs.ibr.dtn.Intent.SENDFILE.equals(intent.getAction())) {
				File file = new File(intent.getStringExtra("filename"));
				long filelen = file.length();
				
				// the the destination address
				EID eid = intent.getParcelableExtra("destination");
				
				// open file as parcelable object
				ParcelFileDescriptor fd = ParcelFileDescriptor.open(file, ParcelFileDescriptor.MODE_READ_ONLY);
				
				Bundle b = new Bundle();
				b.setLifetime(3600L);
				b.setDestination(eid);
				
				// send the file to singleton endpoint
				session.sendFileDescriptor(_session_cb, b, fd, filelen);
			}
		} catch (InterruptedException e) {
			Log.e(TAG, "Error in onHandleIntent", e);
		} catch (RemoteException e) {
			Log.e(TAG, "Error in onHandleIntent", e);
		} catch (FileNotFoundException e) {
			Log.e(TAG, "Error in onHandleIntent", e);
		}
	}
}
