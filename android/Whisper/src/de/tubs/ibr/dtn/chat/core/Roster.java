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

import java.text.SimpleDateFormat;
import java.util.Date;

import android.annotation.SuppressLint;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.provider.BaseColumns;
import android.util.Log;
import de.tubs.ibr.dtn.api.BundleID;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.chat.MessageAdapter;
import de.tubs.ibr.dtn.chat.RosterAdapter;

@SuppressLint("SimpleDateFormat")
public class Roster {
	
	public final static String NOTIFY_ROSTER_CHANGED = "de.tubs.ibr.dtn.chat.NOTIFY_ROSTER_CHANGED";
	
	private final String TAG = "Roster";
	
	private DBOpenHelper _helper = null;
	private SQLiteDatabase database = null;
	private Context context = null;
	
	public static final String TABLE_NAME_ROSTER = "roster";
	public static final String TABLE_NAME_MESSAGES = "messages";
	
	private static final String DATABASE_CREATE_ROSTER =
			"CREATE TABLE " + TABLE_NAME_ROSTER + " (" +
				BaseColumns._ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
				Buddy.NICKNAME + " TEXT NOT NULL, " +
				Buddy.ENDPOINT + " TEXT NOT NULL, " +
				Buddy.LASTSEEN + " TEXT, " +
				Buddy.PRESENCE + " TEXT, " +
				Buddy.STATUS + " TEXT, " +
				Buddy.DRAFTMSG + " TEXT, " +
				Buddy.VOICEEID + " TEXT" +
			");";
	
	private static final String DATABASE_CREATE_MESSAGES = 
			"CREATE TABLE " + TABLE_NAME_MESSAGES + " (" +
				BaseColumns._ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
				Message.BUDDY + " INTEGER NOT NULL, " + 
				Message.DIRECTION + " TEXT NOT NULL, " +
				Message.CREATED + " TEXT NOT NULL, " +
				Message.RECEIVED + " TEXT NOT NULL, " +
				Message.PAYLOAD + " TEXT NOT NULL, " +
				Message.SENTID + " TEXT, " +
				Message.FLAGS + " INTEGER NOT NULL" +
			");";

	private class DBOpenHelper extends SQLiteOpenHelper {
		
		private static final String DATABASE_NAME = "dtnchat_user";
		private static final int DATABASE_VERSION = 10;
		
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
	
	@Override
	protected void finalize() throws Throwable {
		close();
		super.finalize();
	}

	public void open(Context context) throws SQLException
	{
		this.context = context;
		_helper = new DBOpenHelper(context);
		database = _helper.getWritableDatabase();
	}
	
	public SQLiteDatabase getDatabase()
	{
		return database;
	}
	
	public void close()
	{
		_helper.close();
	}
	
	public String getDraftMessage(Long buddyId) {
		Buddy b = getBuddy(buddyId);
		if (b == null) return "";
		return b.getDraftMessage();
	}
	
	public void setDraftMessage(Long buddyId, String message) {
		ContentValues values = new ContentValues();
		
		if (message != null)
		{
			values.put(Buddy.DRAFTMSG, message);
		}
		else
		{
			values.putNull(Buddy.DRAFTMSG);
		}
		
		// update buddy data
		database.update(TABLE_NAME_ROSTER, values, Buddy.ID + " = ?", new String[] { buddyId.toString() });
		
        // send refresh intent
        notifyBuddyChanged(buddyId);
	}
	
	public synchronized void clearMessages(Long buddyId)
	{
		try {
			database.delete(TABLE_NAME_MESSAGES, Message.BUDDY + " = ?", new String[] { String.valueOf(buddyId) });
			
			// send refresh intent
			notifyBuddyChanged(buddyId);
		} catch (Exception e) {
			// buddy not found
		}
	}
	
	public Message getMessage(BundleID id) {
		Message msg = null;
		
		try {
			Cursor cur = database.query(TABLE_NAME_MESSAGES, MessageAdapter.PROJECTION, Message.SENTID + " = ?", new String[] { id.toString() }, null, null, null, "0, 1");
			
			if (cur.moveToNext())
			{
				msg = new Message(this.context, cur, new MessageAdapter.ColumnsMap());
			}
			
			cur.close();
		} catch (Exception e) {
			// buddyid not found
			Log.e(TAG, "getMessage() failed", e);
		}
		
		return msg;
	}
	
	public Message getMessage(Long id)
	{
		Message msg = null;
		
		try {
			Cursor cur = database.query(TABLE_NAME_MESSAGES, MessageAdapter.PROJECTION, Message.ID + " = ?", new String[] { id.toString() }, null, null, null, "0, 1");
			
			if (cur.moveToNext())
			{
				msg = new Message(this.context, cur, new MessageAdapter.ColumnsMap());
			}
			
			cur.close();
		} catch (Exception e) {
			// message not found
		}
		
		return msg;
	}
	
	public synchronized Long createMessage(String buddy, Date created, Date received, Boolean incoming, String payload, Long flags)
	{
		Long bid = getBuddyId(buddy);
		return createMessage(bid, created, received, incoming, payload, flags);
	}
	
	public synchronized Long createMessage(Long buddyId, Date created, Date received, Boolean incoming, String payload, Long flags)
	{
		ContentValues values = new ContentValues();
		
		// create a new message
		if (incoming)
		{
			values.put(Message.DIRECTION, "in");
		}
		else
		{
			values.put(Message.DIRECTION, "out");
		}
		
		SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

		try {
			values.put(Message.BUDDY, buddyId);
			
			if (created == null)
				values.putNull(Message.CREATED);
			else
				values.put(Message.CREATED, dateFormat.format(created));
			
			if (received == null)
				values.putNull(Message.RECEIVED);
			else
				values.put(Message.RECEIVED, dateFormat.format(received));
			
			values.put(Message.PAYLOAD, payload);
			values.put(Message.FLAGS, flags);
			
			// store the message in the database
			long msgid = database.insert(TABLE_NAME_MESSAGES, null, values);
			
			// send refresh intent
			notifyBuddyChanged(buddyId);
			
			return msgid;
		} catch (Exception e) {
			// could not store buddy message
		}
		
		return null;
	}
	
	public void updatePresence(String buddyId, Date created, String presence, String nickname, String status, String voiceeid)
	{
		Long bid = getBuddyId(buddyId);
		
		// check if lastseen is newer than this message
		Buddy buddy = getBuddy(bid);
		if ((buddy.getLastSeen() != null) && buddy.getLastSeen().after(created))
			return;
		
		ContentValues values = new ContentValues();
		
		SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
		
		values.put(Buddy.LASTSEEN, dateFormat.format(created));
		values.put(Buddy.PRESENCE, presence);
		values.put(Buddy.NICKNAME, nickname);
		values.put(Buddy.STATUS, status);
		
		if (voiceeid != null) {
		    values.put(Buddy.VOICEEID, voiceeid);
		} else {
		    values.putNull(Buddy.VOICEEID);
		}
		
		// update buddy data
		database.update(TABLE_NAME_ROSTER, values, Buddy.ID + " = ?", new String[] { bid.toString() });
		
        // send refresh intent
        notifyBuddyChanged(bid);
	}

	public synchronized void reportSent(Long msgId, String sentId)
	{
		ContentValues values = new ContentValues();
		
		Message msg = getMessage(msgId);
		
		if (msg == null) return;
		
		// updates this message
		if (sentId != null)
		{
			values.put(Message.SENTID, sentId);
		}
		
		// set flags to sent
		msg.setFlags(msg.getFlags() | 1);
		
		// updates this message
		values.put(Message.FLAGS, msg.getFlags());

		try {
			// update buddy data
			database.update(TABLE_NAME_MESSAGES, values, Message.ID + " = ?", new String[] { msgId.toString() });
			
			// send refresh intent
			notifyBuddyChanged(msg.getBuddyId());
		} catch (Exception e) {
			// could not store buddy message
		}
	}
	
	public synchronized void reportDelivery(SingletonEndpoint source, BundleID id)
	{
		ContentValues values = new ContentValues();
		
		// get message matching the bundle id
		Message msg = this.getMessage(id);
		
		if (msg == null) return;
		
		// set flags to delivered
		msg.setFlags(msg.getFlags() | 2);
		
		// updates this message
		values.put(Message.FLAGS, msg.getFlags());

		try {
			// update buddy data
			database.update(TABLE_NAME_MESSAGES, values, Message.ID + " = ?", new String[] { msg.getMsgId().toString() });
			
			// send refresh intent
			notifyBuddyChanged(msg.getBuddyId());
		} catch (Exception e) {
			// could not store buddy message
		}
	}
	
	public void removeBuddy(Long buddyId)
	{
		// remove all messages first
		clearMessages(buddyId);
		
		try {
			database.delete(TABLE_NAME_ROSTER, Buddy.ID + " = ?", new String[] { String.valueOf(buddyId) });
		} catch (Exception e) {
			// buddy not found
		}
		
		// send refresh intent
		notifyBuddyChanged(buddyId);
	}
	
	public Buddy getBuddy(Long buddyId) {
		Buddy ret = null;
		
		try {
			Cursor cur = database.query(TABLE_NAME_ROSTER, RosterAdapter.PROJECTION, Buddy.ID + " = ?",
					new String[] { String.valueOf(buddyId) }, null, null, null, "0, 1");

			if (cur.moveToNext()) {
				ret = new Buddy(this.context, cur, new RosterAdapter.ColumnsMap());
			}

			cur.close();
		} catch (Exception e) {
			// buddy not found
		}
		
		return ret;
	}
	
	public Long getBuddyId(String endpoint) {
		Long ret = null;
		
		try {
			Cursor cur = database.query(TABLE_NAME_ROSTER, new String[] {
					Buddy.ID }, Buddy.ENDPOINT + " = ?",
					new String[] { endpoint }, null, null, null, "0, 1");

			if (cur.moveToNext()) {
				ret = cur.getLong(0);
			}

			cur.close();
		} catch (Exception e) {
			// buddy not found
		}
		
		// if there is no buddy entry, create one
		if (ret == null) {
			ContentValues values = new ContentValues();

			try {
				values.put(Buddy.NICKNAME, endpoint);
				values.put(Buddy.ENDPOINT, endpoint);
				
				// store the message in the database
				ret = database.insert(TABLE_NAME_ROSTER, null, values);
				
				// send refresh intent
				notifyBuddyChanged(ret);
			} catch (Exception e) {
				// could not store buddy message
			}
		}
		
		return ret;
	}
	
	public void notifyBuddyChanged(Long buddyId) {
		if (context != null) {
			Intent i = new Intent(Roster.NOTIFY_ROSTER_CHANGED);
			i.putExtra("buddyId", buddyId);
			context.sendBroadcast(i);
		}
	}
}
