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
		private static final int DATABASE_VERSION = 7;
		
		// Database creation sql statement
		private static final String DATABASE_CREATE_ROSTER = "create table roster (_id integer primary key autoincrement, "
				+ "nickname text not null, endpoint text not null, lastseen text, presence text, status text);";
		
		private static final String DATABASE_CREATE_MESSAGES = "create table messages (_id integer primary key autoincrement, "
				+ "buddy integer not null, direction text not null, created text not null, received text not null, payload text not null, flags integer not null);";

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
		Cursor cur = database.query("roster", new String[] { "_id", "nickname", "endpoint", "presence", "status", "lastseen" }, null, null, null, null, null);
		Log.i(TAG, "query for buddies");
		
		cur.moveToFirst();
		while (!cur.isAfterLast())
		{
			Buddy buddy = new Buddy(this, cur.getString(1), cur.getString(2), cur.getString(3), cur.getString(4) );
			
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
			// get buddy id
			String buddyid = String.valueOf( getId(buddy) );

			// load the last 20 messages
			Cursor cur = database.query("messages", new String[] { "direction", "created", "received", "payload" }, "buddy = ?", new String[] { buddyid }, null, null, "created DESC", "0, 20");
			Log.i(TAG, "query for messages");
	
			cur.moveToLast();
			while (!cur.isBeforeFirst())
			{
				try {
					Boolean incoming = cur.getString(0).equals("in");
					Date created = formatter.parse(cur.getString(1));
					Date received = formatter.parse(cur.getString(2));
					String payload = cur.getString(3);
					
					msgs.add( new Message(incoming, created, received, payload) );
				} catch (ParseException e) {
					Log.e(TAG, "failed to convert date: " + cur.getString(1));
				}
				cur.moveToPrevious();
			}
			
			cur.close();
		} catch (Exception e) {
			// buddyid not found
		}
		
		return msgs;
	}
	
	private void createBuddy(String endpointid)
	{
		ContentValues values = new ContentValues();
		
		values.put("nickname", endpointid);
		values.putNull("lastseen");
		values.put("endpoint", endpointid);
		values.putNull("presence");
		values.putNull("status");
		
		// store the new buddy
		database.insert("roster", null, values);
	}
	
	public void store(Buddy buddy)
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
		
		// update buddy data
		database.update("roster", values, "endpoint = ?", new String[] { buddy.getEndpoint() });
		
		// schedule new refresh task
		scheduleRefresh(buddy);
		
		// send refresh intent
		notifyBuddyChanged(buddy);
	}
	
	private int getId(Buddy buddy) throws Exception
	{
		Cursor cur = database.query("roster", new String[] { "_id" }, "endpoint = ?", new String[] { buddy.getEndpoint() }, null, null, null);
		
		try {
			cur.moveToFirst();
			
			if (cur.isAfterLast()) throw new Exception("buddy not found!");
			
			return cur.getInt(0);
		} finally {
			cur.close();
		}
	}
	
	public void clearMessages(Buddy buddy)
	{
		try {
			// get buddy id
			String buddyid = String.valueOf( getId(buddy) );
		
			database.delete("messages", "buddy = ?", new String[] { buddyid });
			
			// send refresh intent
			notifyBuddyChanged(buddy);
		} catch (Exception e) {
			// buddy not found
		}
	}
	
	public void storeMessage(Message msg)
	{
		ContentValues values = new ContentValues();
		
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
			values.put("buddy", getId(msg.getBuddy()));
			values.put("created", dateFormat.format(msg.getCreated()));
			values.put("received", dateFormat.format(msg.getReceived()));
			values.put("payload", msg.getPayload());
			values.put("flags", 0);
			
			// store the message in the database
			database.insert("messages", null, values);
			
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
			// get buddy id
			String buddyid = String.valueOf( getId(buddy) );
		
			database.delete("roster", "_id = ?", new String[] { buddyid });
		} catch (Exception e) {
			// buddy not found
		}
		
		// remove the buddy out of the list
		this.remove(buddy);
		
		// send refresh intent
		notifyBuddyChanged(buddy);
	}
	
	public Buddy get(String endpointid)
	{
		for (Buddy b : this)
		{
			if (b.getEndpoint().equals(endpointid))
			{
				return b;
			}
		}
		
		// buddy not found, create a new one
		Buddy buddy = new Buddy(this, endpointid, endpointid, null, null);
		this.add(buddy);
		
		// create a new buddy in the database
		createBuddy(endpointid);
		
		// send refresh intent
		notifyBuddyChanged(buddy);
		
		return buddy;
	}
	
	public void notifyBuddyChanged(Buddy buddy) {
		if (context != null) {
			Intent i = new Intent(Roster.REFRESH);
			i.putExtra("buddy", buddy.getEndpoint());
			context.sendBroadcast(i);
		}
	}
}
