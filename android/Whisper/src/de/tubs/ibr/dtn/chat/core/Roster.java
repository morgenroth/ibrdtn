/*
 * Roster.java
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
package de.tubs.ibr.dtn.chat.core;

import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;
import de.tubs.ibr.dtn.api.BundleID;
import de.tubs.ibr.dtn.api.SingletonEndpoint;

public class Roster extends LinkedList<Buddy> {
	
	public final static String REFRESH = "de.tubs.ibr.dtn.chat.ROSTER_REFRESH";
	
	private final String TAG = "Roster";
	
	private DBOpenHelper _helper = null;
	private SQLiteDatabase database = null;
	private Context context = null;
	
	private Timer refresh_timer = null;

	/**
	 * unique serial id
	 */
	private static final long serialVersionUID = -2251362993764970200L;
	
	private class DBOpenHelper extends SQLiteOpenHelper {
		
		private static final String DATABASE_NAME = "dtnchat_user";
		private static final int DATABASE_VERSION = 9;
		
		// Database creation sql statement
		private static final String DATABASE_CREATE_ROSTER = "create table roster (_id integer primary key autoincrement, "
				+ "nickname text not null, endpoint text not null, lastseen text, presence text, status text, draftmsg text);";
		
		private static final String DATABASE_CREATE_MESSAGES = "create table messages (_id integer primary key autoincrement, "
				+ "buddy integer not null, direction text not null, created text not null, received text not null, payload text not null, sentid text, flags integer not null);";

		public DBOpenHelper(Context context) {
			super(context, DATABASE_NAME, null, DATABASE_VERSION);
		}

		@Override
		public void onCreate(SQLiteDatabase db) {
			db.execSQL(DATABASE_CREATE_ROSTER);
			db.execSQL(DATABASE_CREATE_MESSAGES);
		}

		@Override
		public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
			Log.w(DBOpenHelper.class.getName(),
					"Upgrading database from version " + oldVersion + " to "
							+ newVersion + ", which will destroy all old data");
			db.execSQL("DROP TABLE IF EXISTS roster");
			db.execSQL("DROP TABLE IF EXISTS messages");
			onCreate(db);
		}
	};

	public Roster()
	{
		refresh_timer = new Timer();
	}

	@Override
	protected void finalize() throws Throwable {
		close();
		super.finalize();
	}

	public void open(Context context) throws SQLException
	{
		final DateFormat formatter = new SimpleDateFormat("yyyy-M-d HH:mm:ss");
		
		this.context = context;
		_helper = new DBOpenHelper(context);
		database = _helper.getWritableDatabase();
		
		// load all buddies
		Cursor cur = database.query("roster", new String[] { "_id", "nickname", "endpoint", "presence", "status", "lastseen", "draftmsg" }, null, null, null, null, null, null);
		Log.i(TAG, "query for buddies");
		
		cur.moveToFirst();
		while (!cur.isAfterLast())
		{
			Buddy buddy = new Buddy(cur.getString(1), cur.getString(2), cur.getString(3), cur.getString(4), cur.getString(6) );
			buddy.setId(cur.getLong(0));
			
			// set the last seen parameter
			if (!cur.isNull(5))
			{
				try {
					buddy.setLastSeen( formatter.parse( cur.getString(5) ) );
				} catch (ParseException e) {
					Log.e(TAG, "failed to convert date: " + cur.getString(5));
				}
			}
			
			this.add( buddy );
			
			// schedule new refresh task
			scheduleRefresh(buddy);
			
			cur.moveToNext();
		}
		
		cur.close();
	}
	
	private class RefreshScheduleTask extends TimerTask {
		private Buddy buddy = null;
		
		public RefreshScheduleTask(Buddy buddy) {
			this.buddy = buddy;
		}
		
		public void run() {
			Roster.this.notifyBuddyChanged(this.buddy);
		}
	}
	
	private void scheduleRefresh(Buddy buddy)
	{
		Calendar cal = buddy.getExpiration();
		cal.add(Calendar.MINUTE, 1);
		refresh_timer.schedule(new RefreshScheduleTask(buddy), cal.getTime());
	}
	
	public void close()
	{
		_helper.close();
	}
	
	public List<Message> getMessages(Buddy buddy)
	{
		final DateFormat formatter = new SimpleDateFormat("yyyy-M-d hh:mm:ss");
		LinkedList<Message> msgs = new LinkedList<Message>();
		
		try {
			// load the last 20 messages
			Cursor cur = database.query("messages", new String[] { "_id", "direction", "created", "received", "payload", "sentid", "flags" }, "buddy = ?", new String[] { buddy.getId().toString() }, null, null, "_id", "0, 20");
			Log.i(TAG, "query for messages");
	
			while (cur.moveToNext())
			{
				try {
					Long msgid = cur.getLong(0);
					Boolean incoming = cur.getString(1).equals("in");
					Date created = formatter.parse(cur.getString(2));
					Date received = formatter.parse(cur.getString(3));
					String payload = cur.getString(4);

					Message m = new Message(msgid, incoming, created, received, payload);
					
					m.setSentId(cur.getString(5));
					m.setFlags(cur.getLong(6));
					m.setBuddy(buddy);
					
					msgs.add( m );
				} catch (ParseException e) {
					Log.e(TAG, "failed to convert date: " + cur.getString(1));
				}
			}
			
			cur.close();
		} catch (Exception e) {
			// buddyid not found
		}
		
		return msgs;
	}
	
	public synchronized void store(Buddy buddy)
	{
		ContentValues values = new ContentValues();
		SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
		
		values.put("nickname", buddy.getNickname());
		
		if (buddy.getLastSeen() != null)
		{
			values.put("lastseen", dateFormat.format(buddy.getLastSeen()));
		}
		
		if (buddy.getPresence() != null)
		{
			values.put("presence", buddy.getPresence());
			Log.i(TAG, "New presence for " + buddy.getNickname() + ": " + buddy.getPresence());
		}
		else
		{
			values.putNull("presence");
		}
		
		if (buddy.getStatus() != null)
		{
			values.put("status", buddy.getStatus());
			Log.i(TAG, "New status for " + buddy.getNickname() + ": " + buddy.getStatus());
		}
		else
		{
			values.putNull("status");
		}
		
		if (buddy.getDraftMessage() != null)
		{
			values.put("draftmsg", buddy.getDraftMessage());
		}
		else
		{
			values.putNull("draftmsg");
		}
		
		// update buddy data
		database.update("roster", values, "_id = ?", new String[] { buddy.getId().toString() });
		
		// schedule new refresh task
		scheduleRefresh(buddy);
		
		// send refresh intent
		notifyBuddyChanged(buddy);
	}

	public synchronized void clearMessages(Buddy buddy)
	{
		try {
			database.delete("messages", "buddy = ?", new String[] { buddy.getId().toString() });
			
			// send refresh intent
			notifyBuddyChanged(buddy);
		} catch (Exception e) {
			// buddy not found
		}
	}
	
	public Message getMessage(BundleID id) {
		final DateFormat formatter = new SimpleDateFormat("yyyy-M-d hh:mm:ss");
		Message msg = null;
		
		try {
			// load the last 20 messages
			Cursor cur = database.query("messages", new String[] { "_id", "buddy", "direction", "created", "received", "payload", "sentid", "flags" }, "sentid = ?", new String[] { id.toString() }, null, null, null, "0, 1");
			
			if (cur.moveToNext())
			{
				try {
					Long msgid = cur.getLong(0);
					Long buddyId = cur.getLong(1);
					
					Boolean incoming = cur.getString(2).equals("in");
					Date created = formatter.parse(cur.getString(3));
					Date received = formatter.parse(cur.getString(4));
					String payload = cur.getString(5);

					msg = new Message(msgid, incoming, created, received, payload);
					
					msg.setSentId(cur.getString(6));
					msg.setFlags(cur.getLong(7));
					msg.setBuddy(this.getBuddy(buddyId));
				} catch (ParseException e) {
					Log.e(TAG, "failed to convert date: " + cur.getString(1));
				}
			}
			
			cur.close();
		} catch (Exception e) {
			// buddyid not found
		}
		
		return msg;
	}
	
	public synchronized void storeMessage(Message msg)
	{
		ContentValues values = new ContentValues();
		
		// create a new message
		if (msg.isIncoming())
		{
			values.put("direction", "in");
		}
		else
		{
			values.put("direction", "out");
		}
		
		SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

		try {
			values.put("buddy", msg.getBuddy().getId());
			values.put("created", dateFormat.format(msg.getCreated()));
			values.put("received", dateFormat.format(msg.getReceived()));
			values.put("payload", msg.getPayload());
			values.put("flags", msg.getFlags());
			
			// store the message in the database
			long msgid = database.insert("messages", null, values);
			msg.setMsgId(msgid);
			
			// send refresh intent
			notifyBuddyChanged(msg.getBuddy());
		} catch (Exception e) {
			// could not store buddy message
		}
	}
	
	public synchronized void reportSent(Message msg)
	{
		ContentValues values = new ContentValues();
		
		// updates this message
		if (msg.getSentId() != null)
		{
			values.put("sentid", msg.getSentId());
		}
		
		// set flags to sent
		msg.setFlags(msg.getFlags() | 1);
		
		// updates this message
		values.put("flags", msg.getFlags());

		try {
			// update buddy data
			database.update("messages", values, "_id = ?", new String[] { msg.getMsgId().toString() });
			
			// send refresh intent
			notifyBuddyChanged(msg.getBuddy());
		} catch (Exception e) {
			// could not store buddy message
		}
	}
	
	public synchronized void reportDelivery(SingletonEndpoint source, BundleID id)
	{
		ContentValues values = new ContentValues();
		
		// get message matching the bundle id
		Message msg = this.getMessage(id);
		
		// set flags to delivered
		msg.setFlags(msg.getFlags() | 2);
		
		// updates this message
		values.put("flags", msg.getFlags());

		try {
			// update buddy data
			database.update("messages", values, "_id = ?", new String[] { msg.getMsgId().toString() });
			
			// send refresh intent
			notifyBuddyChanged(msg.getBuddy());
		} catch (Exception e) {
			// could not store buddy message
		}
	}
	
	public void remove(Buddy buddy)
	{
		// remove all messages first
		clearMessages(buddy);
		
		try {
			database.delete("roster", "_id = ?", new String[] { buddy.getId().toString() });
		} catch (Exception e) {
			// buddy not found
		}
		
		// remove the buddy out of the list
		super.remove(buddy);
		
		// send refresh intent
		notifyBuddyChanged(buddy);
	}
	
	public Buddy getBuddy(String endpointid)
	{
		for (Buddy b : this)
		{
			if (b.getEndpoint().equals(endpointid))
			{
				return b;
			}
		}
		
		// buddy not found, create a new one
		Buddy buddy = new Buddy(endpointid, endpointid, null, null, null);
		
		ContentValues values = new ContentValues();
		
		values.put("nickname", endpointid);
		values.putNull("lastseen");
		values.put("endpoint", endpointid);
		values.putNull("presence");
		values.putNull("status");
		values.putNull("draftmsg");
		
		// store the new buddy
		Long rowid = database.insert("roster", null, values);
		
		// set the rowid as buddyid
		buddy.setId( rowid );
		
		// add buddy to global list
		this.add(buddy);
		
		// send refresh intent
		notifyBuddyChanged(buddy);
		
		return buddy;
	}
	
	public Buddy getBuddy(Long id)
	{
		for (Buddy b : this)
		{
			if (b.getId().equals(id))
			{
				return b;
			}
		}
		
		return null;
	}
	
	public void notifyBuddyChanged(Buddy buddy) {
		if (context != null) {
			Intent i = new Intent(Roster.REFRESH);
			i.putExtra("buddy", buddy.getEndpoint());
			context.sendBroadcast(i);
		}
	}
}
