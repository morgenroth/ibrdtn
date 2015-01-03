package de.tubs.ibr.dtn.api;

import java.util.LinkedList;

import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import de.tubs.ibr.dtn.api.DTNClient.Session;

public abstract class DTNIntentService extends Service {
	
	private HandlerThread mServiceThread;
	private ServiceHandler mServiceHandler;
	
	protected abstract void onHandleIntent(Intent intent);
	protected abstract void onSessionConnected(Session session);
	protected abstract void onSessionDisconnected();
	
	private String TAG = null;
	private DTNClient mClient = null;
	
	private static final int WHAT_CONNECTED = 1;
	private static final int WHAT_RECONNECT = 2;
	private static final int WHAT_INTENT = 3;
	
	public DTNIntentService(String tag) {
		TAG = tag;
	}
	
	protected DTNClient getClient() {
		return mClient;
	}
	
	@Override
	public IBinder onBind(Intent intent) {
		throw new UnsupportedOperationException("This service is not available for binding.");
	}

	private final static class ServiceHandler extends Handler implements SessionConnection {
		
		private DTNIntentService mService = null;
		private boolean mOngoing = false;
		private LinkedList<PendingOperation> mQueue = new LinkedList<PendingOperation>();
		private DTNClient.Session mSession = null;
		
		private class PendingOperation {
			public Intent intent;
			public int startId;
		};
		
		public ServiceHandler(Looper looper, DTNIntentService service) {
			super(looper);
			mService = service;
		}
		
		public void queue(Intent intent, int startId) {
			Message msg = obtainMessage(WHAT_INTENT);
			msg.arg1 = startId;
			msg.obj = intent;
			sendMessage(msg);
		}
		
		public void setOngoing(boolean ongoing) {
			mOngoing = ongoing;
		}
		
		@Override
		public void onSessionDisconnected() {
			mSession = null;
			mService.onSessionDisconnected();
			
			// schedule reconnect
			sendMessageDelayed(obtainMessage(WHAT_RECONNECT), 5000);
		}
		
		@Override
		public void onSessionConnected(Session session) {
			mSession = session;
			mService.onSessionConnected(mSession);
			
			// start processing
			sendMessage(obtainMessage(WHAT_CONNECTED));
		}

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
				case WHAT_INTENT:
				{
					Intent intent = (Intent) msg.obj;
					
					if (mSession == null) {
						// put intent into the pending queue
						PendingOperation po = new PendingOperation();
						po.intent = intent;
						po.startId = msg.arg1;
						mQueue.add(po);
					}
					else {
						if (intent.getAction() != null) mService.onHandleIntent(intent);
						if (!mOngoing) mService.stopSelf(msg.arg1);
					}
					break;
				}
				
				case WHAT_CONNECTED:
				{
					// enqueue all pending actions
					for (PendingOperation po : mQueue) {
						queue(po.intent, po.startId);
					}
					mQueue.clear();
					break;
				}
				
				case WHAT_RECONNECT:
				{
					try {
						mService.mClient.reconnect();
					} catch (ServiceNotAvailableException e) {
						Log.e(mService.TAG, "DTN service no longer available.");
						mService.stopSelf();
					}
					break;
				}
			}
		}
	}
	
	@Override
	public void onStart(Intent intent, int startId) {
		/*
		 * If no explicit intent do not queue any intent to the handler
		 * queue
		 */
		if (intent == null || intent.getAction() == null) {
			return;
		}

		// queue the intent to the handler
		mServiceHandler.queue(intent, startId);
	}
	
	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		onStart(intent, startId);
		return Service.START_STICKY;
	}
	
	@Override
	public void onCreate() {
		super.onCreate();
		
		/*
		 * incoming Intents will be processed by ServiceHandler and queued in
		 * HandlerThread
		 */
		mServiceThread = new HandlerThread(TAG, android.os.Process.THREAD_PRIORITY_BACKGROUND);
		mServiceThread.start();
		Looper looper = mServiceThread.getLooper();
		mServiceHandler = new ServiceHandler(looper, this);
		
		// create a client object
		mClient = new DTNClient(mServiceHandler);
	}
	
	protected void initialize(Registration reg) throws ServiceNotAvailableException {
		mClient.initialize(this, reg);
	}

	@Override
	public void onDestroy() {
		// terminate the client connection
		mClient.terminate();
		mClient = null;
		
		try {
			// stop looper that handles incoming intents
			mServiceThread.quit();
			mServiceThread.join();
		} catch (InterruptedException e) {
			Log.e(TAG, "Wait for looper thread was interrupted.", e);
		}
		
		super.onDestroy();
	}
	
	protected void setOngoing(boolean ongoing) {
		mServiceHandler.setOngoing(ongoing);
	}
}
