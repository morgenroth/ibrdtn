/*
 * MessageDatabase.java
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
package de.tubs.ibr.dtn.dtalkie.db;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.LinkedList;
import java.util.List;

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
import de.tubs.ibr.dtn.dtalkie.MessageAdapter;

@SuppressLint("SimpleDateFormat")
public class MessageDatabase {
	
	private final static String TAG = "MessageDatabase";
	
	public final static String NOTIFY_DATABASE_UPDATED = "de.tubs.ibr.dtn.dtalkie.DATABASE_UPDATED"; 
	
	private DBOpenHelper mHelper = null;
	private SQLiteDatabase mDatabase = null;
    private Context mContext = null;
	
    private static final String DATABASE_NAME = "messages";
    private static final int DATABASE_VERSION = 1;
    
    public enum Folder {
        INBOX("inbox"),
        OUTBOX("outbox");
        
        private Folder(String table) {
            this.mTable = table;
        }
        
        private final String mTable;
        
        public String getTableName() {
            return mTable;
        }
    }
    
    // Database creation sql statement
    private static final String DATABASE_CREATE_OUTBOX = 
            "CREATE TABLE " + Folder.OUTBOX.getTableName() + " (" +
                BaseColumns._ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
                Message.SOURCE + " TEXT NOT NULL, " +
                Message.DESTINATION + " TEXT NOT NULL, " +
                Message.CREATED + " TEXT, " +
                Message.RECEIVED + "  TEXT, " +
                Message.FILENAME + " TEXT NOT NULL, " +
                Message.MARKED + " TEXT, " +
                Message.PRIORITY + " INTEGER " +
            ");";

    private static final String DATABASE_CREATE_INBOX = 
            "CREATE TABLE " + Folder.INBOX.getTableName() + " (" +
                BaseColumns._ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
                Message.SOURCE + " TEXT NOT NULL, " +
                Message.DESTINATION + " TEXT NOT NULL, " +
                Message.CREATED + " TEXT, " +
                Message.RECEIVED + "  TEXT, " +
                Message.FILENAME + " TEXT NOT NULL, " +
                Message.MARKED + " TEXT, " +
                Message.PRIORITY + " INTEGER " +
            ");";
	
	public SQLiteDatabase getDB() {
	    return mDatabase;
	}
	
	private class DBOpenHelper extends SQLiteOpenHelper {
		
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
		mHelper = new DBOpenHelper(context);
		mDatabase = mHelper.getWritableDatabase();
		mContext = context;
	}
	
    public void notifyDataChanged(Folder f) {
        Intent i = new Intent(NOTIFY_DATABASE_UPDATED);
        if (f != null) i.putExtra("folder", f.toString());
        mContext.sendBroadcast(i);
    }
	
	public void close()
	{
		mDatabase.close();
		mHelper.close();
	}
	
	public Message get(Folder f, Long msgid) {
        Message msg = null;
        
        try {
            Cursor cur = mDatabase.query(f.getTableName(), MessageAdapter.PROJECTION, Message.ID + " = ?", new String[] { msgid.toString() }, null, null, null, "0, 1");
            
            if (cur.moveToNext())
            {
                msg = new Message(mContext, cur, new MessageAdapter.ColumnsMap());
            }
            
            cur.close();
        } catch (Exception e) {
            // message not found
            Log.e(TAG, "getMessage() failed", e);
        }
        
        return msg;
	}
	
	public List<Long> getMarkedMessages(Folder f, Boolean val) {
		LinkedList<Long> ret = new LinkedList<Long>();
		
        try {
            Cursor cur = mDatabase.query(f.getTableName(), new String[] { Message.ID }, Message.MARKED + " = ?", new String[] { val ? "yes" : "no" }, null, null, Message.RECEIVED + " DESC");

            while (cur.moveToNext())
            {
                ret.push(cur.getLong(0));
            }
            
            cur.close();
        } catch (Exception e) {
            // messages not found
            Log.e(TAG, "getMarkedMessages() failed", e);
        }
        
        return ret;
	}
	
	public Integer getMarkedMessageCount(Folder f, Boolean val) {
	    Integer ret = 0;
	    
        try {
            Cursor cur = mDatabase.query(f.getTableName(), new String[] { "COUNT(*)" }, Message.MARKED + " = ?", new String[] { val ? "yes" : "no" }, null, null, null);
            
            if (cur.moveToNext())
            {
                ret = cur.getInt(0);
            }
            
            cur.close();
        } catch (Exception e) {
            Log.e(TAG, "getMarkedMessageCount() failed", e);
        }
        
        return ret;
	}
	
    public Message nextMarked(Folder f, Boolean val) {
        Message msg = null;
        
        try {
            Cursor cur = mDatabase.query(f.getTableName(), MessageAdapter.PROJECTION, Message.MARKED + " = ?", new String[] { val ? "yes" : "no" }, null, null, Message.RECEIVED + " ASC", "0, 1");
            
            if (cur.moveToNext())
            {
                msg = new Message(mContext, cur, new MessageAdapter.ColumnsMap());
            }
            
            cur.close();
        } catch (Exception e) {
            // message not found
            Log.e(TAG, "getMessage() failed", e);
        }
        
        return msg;
    }
	
	public void clear(Folder f) {
	    // delete all files
        try {
            Cursor cur = mDatabase.query(f.toString(), MessageAdapter.PROJECTION, null, null, null, null, null);
            
            if (cur.moveToNext())
            {
                File file = new File(cur.getString(new MessageAdapter.ColumnsMap().mColumnFilename));
                file.delete();
            }
            
            cur.close();
        } catch (Exception e) {
            Log.e(TAG, "clear() failed", e);
        }
	    
	    mDatabase.delete(f.toString(), null, null);
	    notifyDataChanged(f);
	}
	
	public void remove(Folder folder, Long msgid) {
	    Message msg = get(folder, msgid);
	    
		// delete the message file
		if (msg.getFile() != null) {
			msg.getFile().delete();
		}
		
        mDatabase.delete(folder.getTableName(), "_id = ?", new String[] { msgid.toString() });

		notifyDataChanged(folder);
	}
	
	public void mark(Folder folder, Long msgid, boolean val) {
		ContentValues values = new ContentValues();

		values.put("marked", val ? "yes" : "no");
		
		// update message data
		mDatabase.update(folder.getTableName(), values, "_id = ?", new String[] { String.valueOf(msgid) });
		
		notifyDataChanged(folder);
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
		mDatabase.insert(folder.getTableName(), null, values);
		
		notifyDataChanged(folder);
	}
}
