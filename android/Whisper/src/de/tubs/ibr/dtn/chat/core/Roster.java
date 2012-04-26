package de.tubs.ibr.dtn.chat.core;

import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Collections;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import de.tubs.ibr.dtn.chat.R;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.TextView;

public class Roster extends LinkedList<Buddy> {
	
	private final String TAG = "Roster";
	
	private DBOpenHelper _helper = null;
	private SQLiteDatabase database = null;
	private SmartListAdapter smartAdapter = null;
	private RefreshCallback refreshcallback = null;
	
	private Timer refresh_timer = null;

	/**
	 * unique serial id
	 */
	private static final long serialVersionUID = -2251362993764970200L;
	
	public interface RefreshCallback
	{
		public void refreshCallback();
	}
	
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

	public class ViewHolder
	{
		public TextView text;
		public TextView bottomText;
		public Buddy buddy;
		public ImageView icon;
	}
	
	private class SmartListAdapter extends BaseAdapter {
		
		private LayoutInflater inflater = null;
		private List<Buddy> list = null;

		public SmartListAdapter(Context context, List<Buddy> list)
		{
			this.inflater = LayoutInflater.from(context);
			this.list = list;
			Collections.sort(this.list);
		}
		
		public int getCount() {
			return list.size();
		}

		public Object getItem(int position) {
			return list.get(position);
		}

		public long getItemId(int position) {
			return position;
		}
		
		public View getView(int position, View convertView, ViewGroup parent) {
			ViewHolder holder;
			
			if (convertView == null)
			{
				convertView = this.inflater.inflate(R.layout.roster_item, null, true);
				holder = new ViewHolder();
				holder.text = (TextView) convertView.findViewById(R.id.label);
				holder.bottomText = (TextView) convertView.findViewById(R.id.bottomtext);
				holder.icon = (ImageView) convertView.findViewById(R.id.icon);
				convertView.setTag(holder);
			} else {
				holder = (ViewHolder) convertView.getTag();
			}
			
			holder.buddy = list.get(position);
			holder.icon.setImageResource(R.drawable.online);
			holder.text.setText(holder.buddy.getNickname());
			
			String presence = holder.buddy.getPresence();
				
			if (presence != null)
			{
				if (presence.equalsIgnoreCase("unavailable"))
				{
					holder.icon.setImageResource(R.drawable.offline);
				}
				else if (presence.equalsIgnoreCase("xa"))
				{
					holder.icon.setImageResource(R.drawable.xa);
				}
				else if (presence.equalsIgnoreCase("away"))
				{
					holder.icon.setImageResource(R.drawable.away);
				}
				else if (presence.equalsIgnoreCase("dnd"))
				{
					holder.icon.setImageResource(R.drawable.busy);
				}
				else if (presence.equalsIgnoreCase("chat"))
				{
					holder.icon.setImageResource(R.drawable.online);
				}
			}
			
			// if the presence is older than 60 minutes then mark the buddy as offline
			if (!holder.buddy.isOnline())
			{
				holder.icon.setImageResource(R.drawable.offline);
			}
			
			if (holder.buddy.getStatus() != null)
			{
				if (holder.buddy.getStatus().length() > 0) { 
					holder.bottomText.setText(holder.buddy.getStatus());
				} else {
					holder.bottomText.setText(holder.buddy.getEndpoint());
				}
			}
			else
			{
				holder.bottomText.setText(holder.buddy.getEndpoint());
			}
			
			return convertView;
		}
		
		public void refresh()
		{
			Collections.sort(this.list);
			if (refreshcallback != null)
			{
				refreshcallback.refreshCallback();
			}
		}
	};
	
	public Roster()
	{
		refresh_timer = new Timer();
	}

	@Override
	protected void finalize() throws Throwable {
		close();
		refresh_timer.cancel();
		super.finalize();
	}

	public void open(Context context) throws SQLException
	{
		final DateFormat formatter = new SimpleDateFormat("yyyy-M-d HH:mm:ss");
		
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
			scheduleRefresh(buddy.getExpiration());
			
			cur.moveToNext();
		}
		
		cur.close();
		smartAdapter = new SmartListAdapter(context, this);
	}
	
	public ListAdapter getListAdapter()
	{
		return this.smartAdapter;
	}
	
	public void setRefreshCallback(RefreshCallback cb) {
		this.refreshcallback = cb;
	}
	
	public void close()
	{
		_helper.close();
	}
	
	private void scheduleRefresh(Date d)
	{
		refresh_timer.schedule(new TimerTask() {
			@Override
			public void run() {
				if (smartAdapter != null) {
					// update smart list
					smartAdapter.refresh();
				}
			}
		}, d);
	}
	
	public List<Message> getMessages(Buddy buddy)
	{
		final DateFormat formatter = new SimpleDateFormat("yyyy-M-d hh:mm:ss");
		LinkedList<Message> msgs = new LinkedList<Message>();
		
		try {
			// get buddy id
			String buddyid = String.valueOf( getBuddyId(buddy) );

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
	
	public void storeBuddyInfo(Buddy buddy)
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
		scheduleRefresh(buddy.getExpiration());
		
		// update smart list
		smartAdapter.refresh();
	}
	
	private int getBuddyId(Buddy buddy) throws Exception
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
			String buddyid = String.valueOf( getBuddyId(buddy) );
		
			database.delete("messages", "buddy = ?", new String[] { buddyid });
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
			values.put("buddy", getBuddyId(msg.getBuddy()));
			values.put("created", dateFormat.format(msg.getCreated()));
			values.put("received", dateFormat.format(msg.getReceived()));
			values.put("payload", msg.getPayload());
			values.put("flags", 0);
			
			// store the message in the database
			database.insert("messages", null, values);
			
			// add message to the buddy object
			msg.getBuddy().addMessage(msg);
		} catch (Exception e) {
			// could not store buddy message
		}
	}
	
	public void removeBuddy(Buddy buddy)
	{
		// remove all messages first
		clearMessages(buddy);
		
		try {
			// get buddy id
			String buddyid = String.valueOf( getBuddyId(buddy) );
		
			database.delete("roster", "_id = ?", new String[] { buddyid });
		} catch (Exception e) {
			// buddy not found
		}
		
		// remove the buddy out of the list
		this.remove(buddy);
		
		// update smart list
		smartAdapter.refresh();
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
		Buddy buddy = new Buddy(this, endpointid, endpointid, null, null);
		this.add(buddy);
		
		// create a new buddy in the database
		createBuddy(endpointid);
		
		// update smart list
		smartAdapter.refresh();
		
		return buddy;
	}
}
