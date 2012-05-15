package de.tubs.ibr.dtn.chat.service;

import java.util.Date;
import java.util.StringTokenizer;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Binder;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.os.PowerManager;
import android.preference.PreferenceManager;
import android.util.Log;
import de.tubs.ibr.dtn.api.Block;
import de.tubs.ibr.dtn.api.Bundle;
import de.tubs.ibr.dtn.api.CallbackMode;
import de.tubs.ibr.dtn.api.DTNClient;
import de.tubs.ibr.dtn.api.DTNClient.Session;
import de.tubs.ibr.dtn.api.DataHandler;
import de.tubs.ibr.dtn.api.GroupEndpoint;
import de.tubs.ibr.dtn.api.Registration;
import de.tubs.ibr.dtn.api.ServiceNotAvailableException;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.chat.MessageActivity;
import de.tubs.ibr.dtn.chat.R;
import de.tubs.ibr.dtn.chat.core.Buddy;
import de.tubs.ibr.dtn.chat.core.Message;
import de.tubs.ibr.dtn.chat.core.Roster;

public class ChatService extends Service {
	
	private static final String TAG = "ChatService";
	
	private static final int MESSAGE_NOTIFICATION = 1;
	public static final String ACTION_OPENCHAT = "de.tubs.ibr.dtn.chat.OPENCHAT";
	public static final GroupEndpoint PRESENCE_GROUP_EID = new GroupEndpoint("dtn://chat.dtn/presence");
	private Registration _registration = null;
	private Boolean _service_available = false;
	
	// executor to process local job queue
	private ExecutorService _executor = null;
	
    // This is the object that receives interactions from clients.  See
    // RemoteService for a more complete example.
    private final IBinder mBinder = new LocalBinder();
    
    // local roster with the connection to the database
    private Roster roster = null;
    
	// DTN client to talk with the DTN service
	private LocalDTNClient _client = null;
	
	private class LocalDTNClient extends DTNClient {
		
		public LocalDTNClient() {
			super(getApplicationInfo().packageName);
		}

		@Override
		protected void sessionConnected(Session session) {
			Log.d(TAG, "DTN session connected");
		}

		@Override
		protected CallbackMode sessionMode() {
			return CallbackMode.SIMPLE;
		}

		@Override
		protected void online() {
		}

		@Override
		protected void offline() {
		}
	};
	
    private DataHandler _data_handler = new DataHandler()
    {
    	Bundle current;

		@Override
		public void startBundle(Bundle bundle) {
			this.current = bundle;
		}

		@Override
		public void endBundle() {
			
			final de.tubs.ibr.dtn.api.BundleID received = new de.tubs.ibr.dtn.api.BundleID(this.current);

			// run the queue and delivered process asynchronously
			_executor.execute(new Runnable() {
		        public void run() {
					try {
						_client.getSession().delivered(received);
					} catch (Exception e) {
						Log.e(TAG, "Can not mark bundle as delivered.", e);
					}
		        }
			});
			
			this.current = null;
		}

		@Override
		public void startBlock(Block block) {
		}

		@Override
		public void endBlock() {
		}

		@Override
		public void characters(String data) {
		}

		@Override
		public void payload(byte[] data) {
			String msg = new String(data);
			
			if (current.destination.equalsIgnoreCase(PRESENCE_GROUP_EID.toString()))
			{
				eventNewPresence(current.source, current.timestamp, msg);
			}
			else
			{
				eventNewMessage(current.source, current.timestamp, msg);
			}
		}

		@Override
		public ParcelFileDescriptor fd() {
			return null;
		}

		@Override
		public void progress(long current, long length) {
		}

		@Override
		public void finished(int startId) {
		}
    
		private void eventNewPresence(String source, Date created, String payload)
		{
			Log.i(TAG, "Presence received from " + source);
			
			// buddy info
			String nickname = null;
			String presence = null;
			String status = null;
			
			StringTokenizer tokenizer = new StringTokenizer(payload, "\n");
			while (tokenizer.hasMoreTokens())
			{
				String data = tokenizer.nextToken();
				
				// search for the delimiter
				int delimiter = data.indexOf(':');
				
				// if the is no delimiter, ignore the line
				if (delimiter == -1) return;
				
				// split the keyword and data pair
				String keyword = data.substring(0, delimiter);
				String value = data.substring(delimiter + 1, data.length()).trim();
				
				if (keyword.equalsIgnoreCase("Presence"))
				{
					presence = value;
				}
				else if (keyword.equalsIgnoreCase("Nickname"))
				{
					nickname = value;
				}
				else if (keyword.equalsIgnoreCase("Status"))
				{
					status = value;
				}
			}
			
			if (nickname != null)
			{
				eventBuddyInfo(created, source, presence, nickname, status);
			}
		}
		
		private void eventNewMessage(String source, Date created, String payload)
		{
			if (source == null)
			{
				Log.e(TAG, "message source is null!");
			}
			
			Buddy b = getRoster().get(source);
			Message msg = new Message(true, created, new Date(), payload);
			msg.setBuddy(b);
			
			getRoster().storeMessage(msg);
			
			// create a notification
			createNotification(b, msg);
			
			// create a status bar notification
			Log.i(TAG, "New message received!");
		}
    };
    
    /**
     * Class for clients to access.  Because we know this service always
     * runs in the same process as its clients, we don't need to deal with
     * IPC.
     */
    public class LocalBinder extends Binder {
    	public ChatService getService() {
            return ChatService.this;
        }
    }

	@Override
	public IBinder onBind(Intent intent) {
		return mBinder;
	}
	
	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i(TAG, "Received start id " + startId + ": " + intent);
        
        if (intent == null) return START_NOT_STICKY;
        if (intent.getAction() == null) return START_NOT_STICKY;

        // create a task to process concurrently
        if (intent.getAction().equals(AlarmReceiver.ACTION))
        {
        	_executor.execute(new BroadcastPresence(startId));
			return START_STICKY;
        }
        // create a task to check for messages
        else if (intent.getAction().equals(de.tubs.ibr.dtn.Intent.RECEIVE))
        {
        	final int stopId = startId;
        	
			// schedule next bundle query
        	_executor.execute(new Runnable() {
		        public void run() {
			        try {
		        		while (_client.query());
		        	} catch (Exception e) { };
		        	
		        	stopSelfResult(stopId);
		        }
			});
			
        	return START_STICKY;
        }
        
        return START_NOT_STICKY;
	}

	@Override
	public void onCreate()
	{
		Log.i(TAG, "service created.");
		super.onCreate();
		
		// create a new executor for tasks
		_executor = Executors.newSingleThreadExecutor();
		
		// create a new client object
		_client = new LocalDTNClient();

		// create a roster object
		this.roster = new Roster();
		this.roster.open(this);
		
		// create registration
    	_registration = new Registration("chat");
    	_registration.add(PRESENCE_GROUP_EID);
		
		// register own data handler for incoming bundles
		_client.setDataHandler(_data_handler);
		
		try {
			_client.initialize(this, _registration);
			_service_available = true;
		} catch (ServiceNotAvailableException e) {
			_service_available = false;
		}
	}
	
	public Boolean isServiceAvailable() {
		return _service_available;
	}
	
	@Override
	public void onDestroy()
	{
		try {
			// stop executor
			_executor.shutdown();
			
			// ... and wait until all jobs are done
			if (!_executor.awaitTermination(10, TimeUnit.SECONDS)) {
				_executor.shutdownNow();
			}
			
			// destroy DTN client
			_client.terminate();
		} catch (InterruptedException e) {
			Log.e(TAG, "Interrupted on service destruction.", e);
		}
		
		// close the roster (plus db connection)
		this.roster.close();
		
		// clear all variables
		this.roster = null;
		_client = null;
		_executor = null;
		
		super.onDestroy();
		
		Log.i(TAG, "service destroyed.");
	}
	
	public synchronized Roster getRoster()
	{
		return this.roster;
	}
	
	public void sendMessage(Message msg) throws Exception
	{
		Session s = _client.getSession();
		
		SingletonEndpoint destination = new SingletonEndpoint(msg.getBuddy().getEndpoint());
		
		String lifetime = PreferenceManager.getDefaultSharedPreferences(this).getString("messageduration", "259200");
		if (!s.send(destination, Integer.parseInt(lifetime), msg.getPayload()))
		{
			throw new Exception("could not send the message");
		}
	}
	
	public void sendPresence(String presence, String nickname, String status) throws Exception
	{
		Session s = _client.getSession();
		
		String presence_message = "Presence: " + presence + "\n" +
				"Nickname: " + nickname + "\n" +
				"Status: " + status;
		
		if (!s.send(ChatService.PRESENCE_GROUP_EID, 3600, presence_message))
		{
			throw new Exception("could not send the message");
		}
	}
	
	public void eventBuddyInfo(Date timestamp, String source, String presence, String nickname, String status)
	{
		// get the buddy object
		Buddy b = getRoster().get(source);
		
		// discard the buddy info, if it is too old
		if (b.getLastSeen() != null)
		{
			if (timestamp.before( b.getLastSeen() ))
			{
				Log.d(TAG, "eventBuddyInfo discarded: " + source + "; " + presence + "; " + nickname + "; " + status);
				return;
			}
		}
		
		Log.d(TAG, "eventBuddyInfo: " + source + "; " + presence + "; " + nickname + "; " + status);
		
		b.setNickname(nickname);
		b.setLastSeen(timestamp);
		b.setStatus(status);
		b.setPresence(presence);
		getRoster().store(b);
	}
	
	private class BroadcastPresence implements Runnable {
		
		private int startId = 0;
		
		public BroadcastPresence(int startId)
		{
			this.startId = startId;
		}

		@Override
		public void run() {
			SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(ChatService.this);
			
			// check if the screen is active
			PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
		    Boolean screenOn = pm.isScreenOn();
			
		    String presence_tag = preferences.getString("presencetag", "auto");
		    String presence_nick = preferences.getString("editNickname", "Nobody");
		    String presence_text = preferences.getString("statustext", "");
		    
		    if (presence_tag.equals("auto"))
		    {
			    if (screenOn)
			    {
			    	presence_tag = "chat";
			    }
			    else
			    {
			    	presence_tag = "away";
			    }
		    }
			
			Log.i(TAG, "push out presence; " + presence_tag);
			try {
				sendPresence(presence_tag, presence_nick, presence_text);
			} catch (Exception e) { }
			
			Editor edit = preferences.edit();
			edit.putLong("lastpresenceupdate", (new Date().getTime()));
			edit.commit();
			
			stopSelfResult(startId);
		}
	}
	
	private void createNotification(Buddy b, Message msg)
	{
		if (MessageActivity.getVisibleBuddy() != null)
			Log.i(TAG, "active buddy: " + MessageActivity.getVisibleBuddy().getNickname());
		
		if (MessageActivity.getVisibleBuddy() == b)
		{
			return;
		}
		
		String ns = Context.NOTIFICATION_SERVICE;
		NotificationManager mNotificationManager = (NotificationManager) getSystemService(ns);
		
		int icon = R.drawable.ic_message;
		CharSequence tickerText = getString(R.string.new_message_from) + " " + b.getNickname();
		long when = System.currentTimeMillis();

		Notification notification = new Notification(icon, tickerText, when);
		notification.defaults |= Notification.DEFAULT_SOUND;
		notification.defaults |= Notification.DEFAULT_VIBRATE;
		notification.flags |= Notification.FLAG_AUTO_CANCEL;
		
		CharSequence contentTitle = getString(R.string.new_message);
		CharSequence contentText = b.getNickname() + ":\n" + msg.getPayload();

		Intent notificationIntent = new Intent(this, MessageActivity.class);
		notificationIntent.setAction(ACTION_OPENCHAT);
		notificationIntent.addCategory("android.intent.category.DEFAULT");
		notificationIntent.putExtra("buddy", b.getEndpoint());
		
		PendingIntent contentIntent = PendingIntent.getActivity(this, 0, notificationIntent, PendingIntent.FLAG_UPDATE_CURRENT);

		notification.setLatestEventInfo(this, contentTitle, contentText, contentIntent);

		mNotificationManager.notify(MESSAGE_NOTIFICATION, notification);
	}
}
