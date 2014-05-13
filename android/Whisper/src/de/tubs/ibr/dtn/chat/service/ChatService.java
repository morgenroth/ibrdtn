/*
 * ChatService.java
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
package de.tubs.ibr.dtn.chat.service;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Date;
import java.util.Locale;
import java.util.StringTokenizer;

import android.app.IntentService;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Binder;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.os.PowerManager;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.support.v4.app.NotificationCompat;
import android.support.v4.app.TaskStackBuilder;
import android.util.Log;
import de.tubs.ibr.dtn.DTNService;
import de.tubs.ibr.dtn.api.Block;
import de.tubs.ibr.dtn.api.Bundle;
import de.tubs.ibr.dtn.api.Bundle.ProcFlags;
import de.tubs.ibr.dtn.api.BundleID;
import de.tubs.ibr.dtn.api.DTNClient;
import de.tubs.ibr.dtn.api.DTNClient.Session;
import de.tubs.ibr.dtn.api.DataHandler;
import de.tubs.ibr.dtn.api.GroupEndpoint;
import de.tubs.ibr.dtn.api.Registration;
import de.tubs.ibr.dtn.api.ServiceNotAvailableException;
import de.tubs.ibr.dtn.api.SessionConnection;
import de.tubs.ibr.dtn.api.SessionDestroyedException;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.api.TransferMode;
import de.tubs.ibr.dtn.chat.MainActivity;
import de.tubs.ibr.dtn.chat.R;
import de.tubs.ibr.dtn.chat.core.Buddy;
import de.tubs.ibr.dtn.chat.core.Message;
import de.tubs.ibr.dtn.chat.core.Roster;

public class ChatService extends IntentService {
	
	public enum Debug {
		NOTIFICATION,
		BUDDY_ADD,
		SEND_PRESENCE
	}
	
	private static final String TAG = "ChatService";
	
	public static final String EXTRA_BUDDY_ID = "de.tubs.ibr.dtn.chat.BUDDY_ID";
	public static final String EXTRA_TEXT_BODY = "de.tubs.ibr.dtn.chat.TEXT_BODY";
	public static final String EXTRA_DISPLAY_NAME = "de.tubs.ibr.dtn.chat.DISPLAY_NAME";
	public static final String EXTRA_PRESENCE = "de.tubs.ibr.dtn.chat.EXTRA_PRESENCE";
	public static final String EXTRA_STATUS = "de.tubs.ibr.dtn.chat.EXTRA_STATUS";
	
	// mark a specific bundle as delivered
	public static final String MARK_DELIVERED_INTENT = "de.tubs.ibr.dtn.chat.MARK_DELIVERED";
	public static final String REPORT_DELIVERED_INTENT = "de.tubs.ibr.dtn.chat.REPORT_DELIVERED";
	
	public static final String ACTION_NEW_MESSAGE = "de.tubs.ibr.dtn.chat.ACTION_NEW_MESSAGE";
	public static final String ACTION_PRESENCE_ALARM = "de.tubs.ibr.dtn.chat.PRESENCE_ALARM";
	public static final String ACTION_SEND_MESSAGE = "de.tubs.ibr.dtn.chat.SEND_MESSAGE";
	public static final String ACTION_REFRESH_PRESENCE = "de.tubs.ibr.dtn.chat.REFRESH_PRESENCE";
	
	private static final int MESSAGE_NOTIFICATION = 1;
	public static final String ACTION_OPENCHAT = "de.tubs.ibr.dtn.chat.OPENCHAT";
	public static final GroupEndpoint PRESENCE_GROUP_EID = new GroupEndpoint("dtn://chat.dtn/presence");
	private Registration _registration = null;
	private ServiceError _service_error = ServiceError.NO_ERROR;

	// This is the object that receives interactions from clients.  See
	// RemoteService for a more complete example.
	private final IBinder mBinder = new LocalBinder();
	
	// local roster with the connection to the database
	private Roster roster = null;
	
	// DTN client to talk with the DTN service
	private DTNClient _client = null;
	
	public ChatService() {
		super(TAG);
	}
	
	private DataHandler _data_handler = new DataHandler()
	{
		ByteArrayOutputStream stream = null;
		Bundle current;
		Long flags = 0L;

		public void startBundle(Bundle bundle) {
			this.current = bundle;
			this.flags = 0L;
			
			if (bundle.get(Bundle.ProcFlags.DTNSEC_STATUS_CONFIDENTIAL)) {
				this.flags |= Message.FLAG_ENCRYPTED;
			}
			
			if (bundle.get(Bundle.ProcFlags.DTNSEC_STATUS_VERIFIED)) {
				this.flags |= Message.FLAG_SIGNED;
			}
		}

		public void endBundle() {
			de.tubs.ibr.dtn.api.BundleID received = new de.tubs.ibr.dtn.api.BundleID(this.current);
			
			// run the queue and delivered process asynchronously
			Intent i = new Intent(ChatService.this, ChatService.class);
			i.setAction(MARK_DELIVERED_INTENT);
			i.putExtra("bundleid", received);
			startService(i);

			this.current = null;
		}

		public TransferMode startBlock(Block block) {
			// ignore messages with a size larger than 8k
			if ((block.length > 8196) || (block.type != 1)) return TransferMode.NULL;
			
			// create a new bytearray output stream
			stream = new ByteArrayOutputStream();
			
			return TransferMode.SIMPLE;
		}

		public void endBlock() {
			if (stream != null) {
				String msg = new String(stream.toByteArray());
				stream = null;
				
				if (current.getDestination().equals(PRESENCE_GROUP_EID))
				{
					eventNewPresence(current.getSource(), current.getTimestamp().getDate(), msg, flags);
				}
				else
				{
					eventNewMessage(current.getSource(), current.getTimestamp().getDate(), msg, flags);
				}
			}
		}

		public void payload(byte[] data) {
			if (stream == null) return;
			// write data to the stream
			try {
				stream.write(data);
			} catch (IOException e) {
				Log.e(TAG, "error on writing payload", e);
			}
		}

		public ParcelFileDescriptor fd() {
			return null;
		}

		public void progress(long current, long length) {
		}
	
		private void eventNewPresence(SingletonEndpoint source, Date created, String payload, Long flags)
		{
			Log.i(TAG, "Presence received from " + source);
			
			// buddy info
			String nickname = null;
			String presence = null;
			String status = null;
			String voiceeid = null;
			String language = null;
			String country = null;
			
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
				else if (keyword.equalsIgnoreCase("Voice"))
				{
					voiceeid = value;
				}
				else if (keyword.equalsIgnoreCase("Language"))
				{
					language = value;
				}
				else if (keyword.equalsIgnoreCase("Country"))
				{
					country = value;
				}
			}
			
			if (nickname != null)
			{
				getRoster().updatePresence(source.toString(), created, presence, nickname, status, voiceeid, language, country, flags);
			}
		}
		
		private void eventNewMessage(SingletonEndpoint source, Date created, String payload, Long flags)
		{
			if (source == null)
			{
				Log.e(TAG, "message source is null!");
			}
			
			// create a new message
			Long msgId = getRoster().createMessage(source.toString(), created, new Date(), true, payload, flags);
			
			// retrieve message object
			Message msg = getRoster().getMessage(msgId);
			
			// get buddy object
			Buddy b = getRoster().getBuddy(msg.getBuddyId());
			
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
	public void onCreate()
	{
		// call onCreate of the super-class
		super.onCreate();
		
		// create a new client object
		_client = new DTNClient(new SessionConnection() {
			@Override
			public void onSessionConnected(Session session) {
				// respect user settings
				if (PreferenceManager.getDefaultSharedPreferences(ChatService.this).getBoolean("checkBroadcastPresence", false))
				{
					// register scheduled presence update
					PresenceGenerator.activate(ChatService.this);
				}
				
				// register own data handler for incoming bundles
				session.setDataHandler(_data_handler);
			}

			@Override
			public void onSessionDisconnected() {
			}
		});

		// create a roster object
		this.roster = new Roster();
		this.roster.open(this);
		
		// create registration
		_registration = new Registration("chat");
		_registration.add(PRESENCE_GROUP_EID);
		
		try {
			_client.initialize(this, _registration);
			_service_error = ServiceError.NO_ERROR;
		} catch (ServiceNotAvailableException e) {
			_service_error = ServiceError.SERVICE_NOT_FOUND;
		} catch (SecurityException ex) {
			_service_error = ServiceError.PERMISSION_NOT_GRANTED;
		}
		
		Log.i(TAG, "service created.");
	}
	
	public ServiceError getServiceError() {
		return _service_error;
	}
	
	@Override
	public void onDestroy()
	{
		// close the roster (plus db connection)
		this.roster.close();
		
		// destroy DTN client
		_client.terminate();
		
		// clear all variables
		this.roster = null;
		_client = null;

		super.onDestroy();
		
		Log.i(TAG, "service destroyed.");
	}
	
	public synchronized Roster getRoster()
	{
		return this.roster;
	}

	@SuppressWarnings("deprecation")
	private void showNotification(Intent intent)
	{
		int defaults = 0;

		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
		if (prefs.getBoolean("vibrateOnMessage", true)) {
			defaults |= Notification.DEFAULT_VIBRATE;
		}
		
		Long buddyId = intent.getLongExtra(EXTRA_BUDDY_ID, -1L);
		String displayName = intent.getStringExtra(EXTRA_DISPLAY_NAME);
		String textBody = intent.getStringExtra(EXTRA_TEXT_BODY);
		
		CharSequence tickerText = getString(R.string.new_message_from) + " " + displayName;
		CharSequence contentTitle = getString(R.string.new_message);
		CharSequence contentText = displayName + ":\n" + textBody;
		
		TaskStackBuilder stackBuilder = TaskStackBuilder.create(this);
		
		// forward intent to the activity
		intent.setClass(this, MainActivity.class);
		
		// Adds the intent to the main view
		stackBuilder.addNextIntent(intent);
		// Gets a PendingIntent containing the entire back stack
		PendingIntent contentIntent = stackBuilder.getPendingIntent(buddyId.intValue(), PendingIntent.FLAG_UPDATE_CURRENT);
		
		NotificationCompat.Builder builder = new NotificationCompat.Builder(this);
		builder.setContentTitle(contentTitle);
		builder.setContentText(contentText);
		builder.setSmallIcon(R.drawable.ic_message);
		builder.setTicker(tickerText);
		builder.setDefaults(defaults);
		builder.setWhen( System.currentTimeMillis() );
		builder.setContentIntent(contentIntent);
		builder.setLights(0xffff0000, 300, 1000);
		builder.setSound( Uri.parse( prefs.getString("ringtoneOnMessage", "content://settings/system/notification_sound") ) );
		builder.setAutoCancel(true);
		
		Notification notification = builder.getNotification();
		
		NotificationManager mNotificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
		mNotificationManager.notify(buddyId.toString(), MESSAGE_NOTIFICATION, notification);
		
		if (prefs.getBoolean("ttsWhenOnHeadset", false)) {
			AudioManager am = (AudioManager)getSystemService(Context.AUDIO_SERVICE);
			
			if (am.isBluetoothA2dpOn() || am.isWiredHeadsetOn()) {
				// speak the notification
				Intent tts_intent = new Intent(this, TTSService.class);
				tts_intent.setAction(TTSService.INTENT_SPEAK);
				tts_intent.putExtra("speechText", tickerText + ": " + textBody);
				startService(tts_intent);
			}
		}
	}
	
	private void createNotification(Buddy b, Message msg)
	{
		Intent intent = new Intent(ACTION_NEW_MESSAGE);
		intent.putExtra(EXTRA_BUDDY_ID, b.getId());
		intent.putExtra(EXTRA_DISPLAY_NAME, b.getNickname());
		intent.putExtra(EXTRA_TEXT_BODY, msg.getPayload());
		
		// send intent to activity or broadcast receiver for notification
		sendOrderedBroadcast(intent, null);
	}
	
	public void clearNotification(Long buddyId) {
		if (buddyId == null) return;
		
		// clear notification
		NotificationManager mNotificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
		mNotificationManager.cancel(buddyId.toString(), MESSAGE_NOTIFICATION);
	}

	@Override
	protected void onHandleIntent(Intent intent) {
		String action = intent.getAction();
		
		// create a task to process concurrently
		if (ACTION_PRESENCE_ALARM.equals(action))
		{
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
			actionRefreshPresence(presence_tag, presence_nick, presence_text);
			
			Editor edit = preferences.edit();
			edit.putLong("lastpresenceupdate", (new Date().getTime()));
			edit.commit();
		}
		// create a task to check for messages
		else if (de.tubs.ibr.dtn.Intent.RECEIVE.equals(action))
		{
			try {
				while (_client.getSession().queryNext());
			} catch (SessionDestroyedException e) {
				Log.e(TAG, "Can not query for bundle", e);
			} catch (InterruptedException e) {
				Log.e(TAG, "Can not query for bundle", e);
			}
		}
		else if (MARK_DELIVERED_INTENT.equals(action))
		{
			actionMarkDelivered(intent);
		}
		else if (REPORT_DELIVERED_INTENT.equals(action))
		{
			actionReportDelivered(intent);
		}
		else if (ACTION_SEND_MESSAGE.equals(action))
		{
			Long buddyId = intent.getLongExtra(ChatService.EXTRA_BUDDY_ID, -1);
			String text = intent.getStringExtra(ChatService.EXTRA_TEXT_BODY);
			
			// abort if there is no buddyId
			if (buddyId < 0) return;
			
			actionSendMessage(buddyId, text);
		}
		else if (ACTION_REFRESH_PRESENCE.equals(action))
		{
			String presence = intent.getStringExtra(ChatService.EXTRA_PRESENCE);
			String nickname = intent.getStringExtra(ChatService.EXTRA_DISPLAY_NAME);
			String status = intent.getStringExtra(ChatService.EXTRA_STATUS);
			
			actionRefreshPresence(presence, nickname, status);
		}
		else if (ACTION_NEW_MESSAGE.equals(action)) {
			showNotification(intent);
		}
	}
	
	private void actionMarkDelivered(Intent intent) {
		BundleID bundleid = intent.getParcelableExtra("bundleid");
		if (bundleid == null) {
			Log.e(TAG, "Intent to mark a bundle as delivered, but no bundle ID given");
			return;
		}
		
		try {
			_client.getSession().delivered(bundleid);
		} catch (Exception e) {
			Log.e(TAG, "Can not mark bundle as delivered.", e);
		}	
	}
	
	private void actionReportDelivered(Intent intent) {
		SingletonEndpoint source = intent.getParcelableExtra("source");
		BundleID bundleid = intent.getParcelableExtra("bundleid");
		
		if (bundleid == null) {
			Log.e(TAG, "Intent to mark a bundle as delivered, but no bundle ID given");
			return;
		}
		
		synchronized(this.roster) {
			// report delivery to the roster
			getRoster().reportDelivery(source, bundleid);
		}
	}
	
	private void actionSendMessage(Long buddyId, String text) {
		try {
			Session s = _client.getSession();
			
			Long msgId = getRoster().createMessage(buddyId, new Date(), new Date(), false, text, 0L);
			
			// load buddy from roster
			Buddy buddy = getRoster().getBuddy( buddyId );
			
			// create a new bundle
			Bundle b = new Bundle();
			
			b.setDestination(new SingletonEndpoint(buddy.getEndpoint()));
			
			String lifetime = PreferenceManager.getDefaultSharedPreferences(this).getString("messageduration", "259200");
			b.setLifetime(Long.parseLong(lifetime));
			
			// set status report requests
			b.set(ProcFlags.REQUEST_REPORT_OF_BUNDLE_DELIVERY, true);
			b.setReportto(SingletonEndpoint.ME);
			
			// request encryption for this message
			b.set(ProcFlags.DTNSEC_REQUEST_ENCRYPT, true);
			
			// request signing of the message
			b.set(ProcFlags.DTNSEC_REQUEST_SIGN, true);
			
			synchronized(this.roster) {
				// send out the message
				BundleID ret = s.send(b, text.getBytes());
				
				if (ret == null)
				{
					Log.e(TAG, "could not send the message");
				}
				else
				{
					Log.d(TAG, "Bundle sent, BundleID: " + ret.toString());
				}
				
				// update message into the database
				getRoster().reportSent(msgId, ret.toString());
			}
		} catch (InterruptedException e) {
			e.printStackTrace();
		} catch (SessionDestroyedException e) {
			e.printStackTrace();
		}
	}
	
	public void actionRefreshPresence(String presence, String nickname, String status) {	
		try {
			Session s = _client.getSession();
			
			String presence_message = "Presence: " + presence + "\n" +
					"Nickname: " + nickname + "\n" +
					"Status: " + status + "\n" +
					"Language: " + Locale.getDefault().getLanguage() + "\n" +
					"Country: " + Locale.getDefault().getCountry();
			
			try {
				if (Utils.isVoiceRecordingSupported(this)) {
					DTNService dtns = _client.getDTNService();
					if (dtns != null) {
						presence_message += "\n" + "Voice: " + dtns.getEndpoint() + "/dtalkie";
					}
				}
			} catch (RemoteException e) { }
			
			// create a new bundle
			Bundle b = new Bundle();
			
			// set destination to group endpoint
			b.setDestination(ChatService.PRESENCE_GROUP_EID);
			
			// set lifetime to one hour
			b.setLifetime(3600L);
			
			// request signing of the message
			b.set(ProcFlags.DTNSEC_REQUEST_SIGN, true);
			
			BundleID ret = s.send(b, presence_message.getBytes());
			
			if (ret == null)
			{
				Log.e(TAG, "could not send the message");
			}
			else
			{
				Log.d(TAG, "Presence sent, BundleID: " + ret.toString());
			}
		} catch (InterruptedException e) {
			e.printStackTrace();
		} catch (SessionDestroyedException e) {
			e.printStackTrace();
		}
	}
	
	public void startDebug(Debug d) {
		String debug_source = "dtn://debug/chat";
		
		switch (d) {
		case NOTIFICATION:
			// create a new message
			Long msgId = getRoster().createMessage(debug_source, new Date(), new Date(), true, "Hello World", 0L);
			
			// retrieve message object
			Message msg = getRoster().getMessage(msgId);
			
			// get buddy object
			Buddy b = getRoster().getBuddy(msg.getBuddyId());
			
			// create a notification
			createNotification(b, msg);
			break;
		case BUDDY_ADD:
			getRoster().updatePresence(debug_source + "/" + String.valueOf((new Date()).getTime()), new Date(), "online", "Debug Buddy", "Hello World", "dtn://test/dtalkie", "en", "gb", 0L);
			break;
			
		case SEND_PRESENCE:
			// wake-up the chat service and queue a send presence task
			Intent i = new Intent(this, ChatService.class);
			i.setAction(ACTION_PRESENCE_ALARM);
			startService(i);
			break;
		}
	}
}
