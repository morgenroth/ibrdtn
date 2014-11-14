package de.tubs.ibr.dtn.api;

import java.util.LinkedList;

import android.annotation.SuppressLint;
import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import de.tubs.ibr.dtn.api.DTNClient.Session;

public abstract class DTNIntentService extends Service {
	
	private volatile Looper mServiceLooper;
	private volatile ServiceHandler mServiceHandler;
	
	protected abstract void onHandleIntent(Intent intent);
	protected abstract void onSessionConnected(Session session);
	protected abstract void onSessionDisconnected();
	
	private DTNClient mClient = null;
	private DTNClient.Session mSession = null;

	private LinkedList<PendingOperation> mQueue = new LinkedList<PendingOperation>();
	
	private static final int STATE_CONNECTED = -1;
	
	class PendingOperation {
		public Intent intent;
		public int startId;
	};
	
	public DTNIntentService(String tag) {
	}
	
	protected DTNClient getClient() {
		return mClient;
	}
	
	@Override
	public IBinder onBind(Intent intent) {
		throw new UnsupportedOperationException("This service is not available for binding.");
	}

	@SuppressLint("HandlerLeak")
	private final class ServiceHandler extends Handler {
		public ServiceHandler(Looper looper) {
			super(looper);
		}

		@Override
		public void handleMessage(Message msg) {
			Intent intent = (Intent) msg.obj;
			
			if (msg.arg1 == STATE_CONNECTED) {
				// enqueue all pending actions
				for (PendingOperation po : mQueue) {
					queue(po.intent, po.startId);
				}
				mQueue.clear();
			}
			else if (mSession == null) {
				// put intent into the pending queue
				PendingOperation po = new PendingOperation();
				po.intent = intent;
				po.startId = msg.arg1;
				mQueue.add(po);
			}
			else if (intent.getAction() == null) {
				stopSelf(msg.arg1);
			}
			else {
				onHandleIntent(intent);
				stopSelf(msg.arg1);
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
		queue(intent, startId);
	}
	
	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		onStart(intent, startId);
		return Service.START_STICKY;
	}
	
	private void queue(Intent intent, int startId) {
		Message msg = mServiceHandler.obtainMessage();
		msg.arg1 = startId;
		msg.obj = intent;
		mServiceHandler.sendMessage(msg);
	}
	
	@Override
	public void onCreate() {
		super.onCreate();
		
		/*
		 * incoming Intents will be processed by ServiceHandler and queued in
		 * HandlerThread
		 */
		HandlerThread thread = new HandlerThread("DaemonService_IntentThread");
		thread.start();
		mServiceLooper = thread.getLooper();
		mServiceHandler = new ServiceHandler(mServiceLooper);
		
		// create a client object
		mClient = new DTNClient(mConnectionCallback);
	}
	
	protected void initialize(Registration reg) throws ServiceNotAvailableException {
		mClient.initialize(this, reg);
	}

	@Override
	public void onDestroy() {
		// terminate the client connection
		mClient.terminate();
		mClient = null;
		
		// stop looper that handles incoming intents
		mServiceLooper.quit();
		
		super.onDestroy();
	}
	
	SessionConnection mConnectionCallback = new SessionConnection() {
		@Override
		public void onSessionDisconnected() {
			mSession = null;
			DTNIntentService.this.onSessionDisconnected();
		}
		
		@Override
		public void onSessionConnected(Session session) {
			mSession = session;
			DTNIntentService.this.onSessionConnected(mSession);
			
			// start processing
			queue(null, STATE_CONNECTED);
		}
	};
}
