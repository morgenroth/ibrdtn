package de.tubs.ibr.dtn.dtalkie.db;

import java.io.File;
import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import de.tubs.ibr.dtn.dtalkie.R;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.TextView;

public class MessageDatabase {
	
	private final static String TAG = "MessageDatabase"; 
	
	private DBOpenHelper _helper = null;
	private SQLiteDatabase _database = null;
	private OnUpdateListener _listener = null;
	
	private HashMap<Folder, SmartListAdapter> _views = new HashMap<Folder, SmartListAdapter>();
	private HashMap<Folder, LinkedList<Message>> _messages = new HashMap<Folder, LinkedList<Message>>();
	
	public enum Folder {
		INBOX("inbox"),
		OUTBOX("outbox");
		
		private Folder(String name) {
			this.name = name;
		}
		
		private final String name;
		
		public String toString() {
			return name;
		}
	}
	
	public interface OnUpdateListener
	{
		public void update(Folder folder);
	}
	
	private class DBOpenHelper extends SQLiteOpenHelper {
		
		private static final String DATABASE_NAME = "messages";
		private static final int DATABASE_VERSION = 1;
		
		// Database creation sql statement
		private static final String DATABASE_CREATE_OUTBOX = "create table outbox (_id integer primary key autoincrement, "
				+ "source text not null, destination text not null, created text, received text,  filename text not null, marked text, priority integer);";

		private static final String DATABASE_CREATE_INBOX = "create table inbox (_id integer primary key autoincrement, "
				+ "source text not null, destination text not null, created text, received text, filename text not null, marked text, priority integer);";
		
		public DBOpenHelper(Context context) {
			super(context, DATABASE_NAME, null, DATABASE_VERSION);
		}

		@Override
		public void onCreate(SQLiteDatabase db) {
			db.execSQL(DATABASE_CREATE_OUTBOX);
			db.execSQL(DATABASE_CREATE_INBOX);
		}

		@Override
		public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
			Log.w(DBOpenHelper.class.getName(),
					"Upgrading database from version " + oldVersion + " to "
							+ newVersion + ", which will destroy all old data");
			db.execSQL("DROP TABLE IF EXISTS outbox");
			db.execSQL("DROP TABLE IF EXISTS inbox");
			onCreate(db);
		}
	};

	public MessageDatabase(Context context) throws SQLException
	{
		final DateFormat formatter = new SimpleDateFormat("yyyy-M-d HH:mm:ss");
		
		_helper = new DBOpenHelper(context);
		_database = _helper.getWritableDatabase();
		
		for (Folder f : Folder.values())
		{
			// create list
			LinkedList<Message> l = new LinkedList<Message>();
			_messages.put(f, l);
			
			// load all messages
			Cursor cur = _database.query(f.toString(), new String[] { "_id", "source", "destination", "created", "received", "filename", "marked", "priority" }, null, null, null, null, null, null);
			Log.i(TAG, "query for messages");
			
			cur.moveToFirst();
			while (!cur.isAfterLast())
			{
				Message msg = new Message(cur.getLong(0));
				
				msg.setSource(cur.getString(1));
				msg.setDestination(cur.getString(2));
				
				try {
					if (!cur.isNull(3)) msg.setCreated( formatter.parse( cur.getString(3) ));
				} catch (ParseException e) {
					Log.e(TAG, "failed to convert date: " + cur.getString(3));
				}
				
				try {
					if (!cur.isNull(4)) msg.setReceived( formatter.parse( cur.getString(4) ));
				} catch (ParseException e) {
					Log.e(TAG, "failed to convert date: " + cur.getString(4));
				}

				if (!cur.isNull(5)) msg.setFile( new File(cur.getString(5)) );
				if (!cur.isNull(6)) msg.setMarked(cur.getString(6).equals("yes"));
				if (!cur.isNull(7)) msg.setPriority(cur.getInt(7));
				
				l.add( msg );
				
				cur.moveToNext();
			}
			
			cur.close();
			
			// create view
			_views.put(f, new SmartListAdapter(context, f, l));
		}
	}
	
	public void close()
	{
		_database.close();
		_helper.close();
		
		// destroy views
		_views.clear();
	}
	
	public void clear(Folder folder) {
		LinkedList<Message> msgs = _messages.get(folder);
		
		while (!msgs.isEmpty()) {
			remove(folder, msgs.element());
		}
	}
	
	public void remove(Folder folder, Message msg) {
		if (msg.getId() != null) {
			_database.delete(folder.toString(), "_id = ?", new String[] { msg.getId().toString() });
			
			// remove from cached list
			_messages.get(folder).remove(msg);
			
			// refresh smart view
			_views.get(folder).refresh();
			
			// delete the message file
			if (msg.getFile() != null) {
				msg.getFile().delete();
			}
		}
	}
	
	public void mark(Folder folder, Message msg, boolean val) {
		msg.setMarked(val);
		
		ContentValues values = new ContentValues();

		values.put("marked", val ? "yes" : "no");
		
		// update message data
		_database.update(folder.toString(), values, "_id = ?", new String[] { msg.getId().toString() });
		
		// refresh smart view
		_views.get(folder).refresh();
	}
	
	public void put(Folder folder, Message msg) {
		ContentValues values = new ContentValues();
		
		SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
		
		values.put("source", msg.getSource());
		values.put("destination", msg.getDestination());
		
		if (msg.getCreated() != null) {
			values.put("created", dateFormat.format(msg.getCreated()));
		}
		
		if (msg.getReceived() != null) {
			values.put("received", dateFormat.format(msg.getReceived()));
		}
		
		values.put("filename", msg.getFile().getPath());
		
		values.put("marked", (msg.isMarked()) ? "yes" : "no");
		if (msg.getPriority() != null) values.put("priority", msg.getPriority());
		
		// store the message in the database
		Long rowId = _database.insert(folder.toString(), null, values);
		
		// set id of the message
		msg.setId(rowId);
		
		// add to cached list
		_messages.get(folder).add(msg);
		
		// refresh smart view
		_views.get(folder).refresh();
	}
	
	public Message get(Folder folder, Integer pos) {
		return _messages.get(folder).get(pos);
	}
	
	public Message getMessage(Folder folder, Long id) {
		List<Message> msgs = _messages.get(folder);
		for (Message m : msgs) {
			if (m.getId().equals(id)) return m;
		}
		return null;
	}
	
	public Message getMarkedMessage(Folder folder, Boolean val) {
		LinkedList<Message> msgs = _messages.get(folder);
		Iterator<Message> i = msgs.descendingIterator();
		while (i.hasNext()) {
			Message m = i.next();
			if (m.isMarked() == val) return m;
		}
		return null;
	}
	
	public ListAdapter getListAdapter(Folder folder)
	{
		return _views.get(folder);
	}
	
	public class ViewHolder
	{
		public TextView text;
		public TextView bottomText;
		public TextView sideText;
		public Message msg;
		public ImageView icon;
	}
	
	private static final Comparator<Message> _list_sorter = new Comparator<Message>() {
		@Override
		public int compare(Message arg0, Message arg1) {
			return arg1.compareTo(arg0);
		}	
	};
	
	private class SmartListAdapter extends BaseAdapter {
		
		private LayoutInflater inflater = null;
		private List<Message> list = null;
		private Folder folder = null;

		public SmartListAdapter(Context context, Folder folder, List<Message> list)
		{
			this.inflater = LayoutInflater.from(context);
			this.folder = folder;
			this.list = list;
			Collections.sort(this.list, _list_sorter);
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
				convertView = this.inflater.inflate(R.layout.message_item, null, true);
				holder = new ViewHolder();
				holder.text = (TextView) convertView.findViewById(R.id.label);
				holder.bottomText = (TextView) convertView.findViewById(R.id.bottomtext);
				holder.sideText = (TextView) convertView.findViewById(R.id.sidetext);
				holder.icon = (ImageView) convertView.findViewById(R.id.icon);
				convertView.setTag(holder);
			} else {
				holder = (ViewHolder) convertView.getTag();
			}
			
			holder.msg = list.get(position);
			
			if (holder.msg.isMarked()) {
				holder.icon.setImageLevel(0);
			} else {
				holder.icon.setImageLevel(1);
			}
			
			holder.text.setText(holder.msg.getSource());
			holder.bottomText.setText( DateFormat.getDateTimeInstance().format( holder.msg.getCreated()) );
			
			// get delay
			if (holder.msg.getReceived() != null)
			{
				Long delay = holder.msg.getReceived().getTime() - holder.msg.getCreated().getTime();
				
				if (holder.msg.getReceived().before(holder.msg.getCreated()))
				{
					holder.sideText.setText("");
				}
				else if (delay <= 1000)
				{
					holder.sideText.setText("1 second");
				}
				else if (delay < 60000)
				{
					holder.sideText.setText((delay / 1000) + " seconds");
				}
				else if (delay < 120000)
				{
					holder.sideText.setText("~1 minute");
				}
				else
				{
					holder.sideText.setText("~" + delay / 60000 + " minutes");
				}
			}
			else
			{
				holder.sideText.setText("");
			}
			
			return convertView;
		}
		
		public void refresh()
		{
			Collections.sort(this.list, _list_sorter);
			notifyOnUpdateListener(this.folder);
		}
	};
	
	public void setOnUpdateListener(OnUpdateListener listener) {
		_listener = listener;
	}
	
	private void notifyOnUpdateListener(Folder folder) {
		if (_listener != null) {
			_listener.update(folder);
		}
	}
}
