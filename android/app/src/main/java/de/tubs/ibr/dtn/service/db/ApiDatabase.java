package de.tubs.ibr.dtn.service.db;

import java.io.Closeable;
import java.security.SecureRandom;
import java.util.LinkedList;
import java.util.List;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.provider.BaseColumns;
import android.util.Log;
import de.tubs.ibr.dtn.api.GroupEndpoint;

public class ApiDatabase implements Closeable {
	private final String TAG = "ApiDatabase";
	
	private DBOpenHelper mHelper = null;
	private SQLiteDatabase mDatabase = null;
	private Context mContext = null;
	
	private SecureRandom mRandom = new SecureRandom();

	public static final String TABLE_NAME_SESSIONS = "sessions";
	public static final String TABLE_NAME_ENDPOINTS = "endpoints";
	
	private static final String DATABASE_CREATE_SESSIONS =
			"CREATE TABLE " + TABLE_NAME_SESSIONS + " (" +
				BaseColumns._ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
				Session.PACKAGE_NAME + " TEXT NOT NULL UNIQUE, " +
				Session.SESSION_KEY + " TEXT NOT NULL, " +
				Session.DEFAULT_ENDPOINT + " TEXT" +
			");";
	
	private static final String DATABASE_CREATE_ENDPOINTS = 
			"CREATE TABLE " + TABLE_NAME_ENDPOINTS + " (" +
				BaseColumns._ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
				Endpoint.SESSION + " INTEGER NOT NULL, " +
				Endpoint.ENDPOINT + " TEXT NOT NULL, " +
				Endpoint.SINGLETON + " INTEGER NOT NULL, " +
				Endpoint.FQEID + " INTEGER NOT NULL, " +
				"UNIQUE(" + Endpoint.SESSION + ", " + Endpoint.ENDPOINT + ") ON CONFLICT FAIL" +
			");";
	
	private class DBOpenHelper extends SQLiteOpenHelper {
		
		private static final String DATABASE_NAME = "dtnchat_user";
		private static final int DATABASE_VERSION = 12;
		
		public DBOpenHelper(Context context) {
			super(context, DATABASE_NAME, null, DATABASE_VERSION);
		}

		@Override
		public void onCreate(SQLiteDatabase db) {
			db.execSQL(DATABASE_CREATE_SESSIONS);
			db.execSQL(DATABASE_CREATE_ENDPOINTS);
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

	public synchronized void open(Context context) throws SQLException
	{
		mContext = context;
		mHelper = new DBOpenHelper(mContext);
		mDatabase = mHelper.getWritableDatabase();
		
		Log.d(TAG, "API database opened");
	}
	
	public SQLiteDatabase getDatabase()
	{
		return mDatabase;
	}
	
	public synchronized void close()
	{
		if (mHelper != null) {
		    mHelper.close();
		    mHelper = null;
		    Log.d(TAG, "API database closed");
		}
	}
	
	public List<Session> getSessions() {
		List<Session> ret = new LinkedList<Session>();
		
		Cursor cur = mDatabase.query(TABLE_NAME_SESSIONS, Session.PROJECTION, null, null, null, null, null);
		try {
			while (cur.moveToNext()) {
				Session s = new Session(mContext, cur);
				ret.add(s);
			}
		} finally {
			cur.close();
		}
		return ret;
	}
	
	public List<Endpoint> getEndpoints(Session s) {
		List<Endpoint> ret = new LinkedList<Endpoint>();
		
		Cursor cur = mDatabase.query(TABLE_NAME_ENDPOINTS, Endpoint.PROJECTION, Endpoint.SESSION + " = ?", new String[] { s.getId().toString() }, null, null, null);
		try {
			while (cur.moveToNext()) {
				Endpoint e = new Endpoint(mContext, cur);
				ret.add(e);
			}
		} finally {
			cur.close();
		}
		return ret;
	}
	
	private Endpoint getEndpoint(Long id) {
		Endpoint ret = null;
		
		Cursor cur = mDatabase.query(TABLE_NAME_ENDPOINTS, Endpoint.PROJECTION, Endpoint.ID + " = ?", new String[] { id.toString() }, null, null, null);
		try {
			if (cur.moveToNext()) {
				ret = new Endpoint(mContext, cur);
			}
		} finally {
			cur.close();
		}
		return ret;
	}
	
	public void removeSession(Session s) {
		// remove the session
		mDatabase.delete(TABLE_NAME_SESSIONS, Session.ID + " = ?", new String[] { s.getId().toString() });
		
		// remove all endpoints of the session
		mDatabase.delete(TABLE_NAME_ENDPOINTS, Endpoint.SESSION + " = ?", new String[] { s.getId().toString() });
	}
	
	public Session getSession(String packageName) {
		Session ret = null;
		
		Cursor cur = mDatabase.query(TABLE_NAME_SESSIONS, Session.PROJECTION, Session.PACKAGE_NAME + " = ?", new String[] { packageName }, null, null, null);
		try {
			if (cur.moveToNext()) {
				ret = new Session(mContext, cur);
			}
		} finally {
			cur.close();
		}
		return ret;
	}
	
	public Session getSession(String packageName, String sessionKey) {
		Session ret = null;
		
		Cursor cur = mDatabase.query(TABLE_NAME_SESSIONS, Session.PROJECTION, Session.PACKAGE_NAME + " = ? AND " + Session.SESSION_KEY + " = ?", new String[] { packageName, sessionKey }, null, null, null);
		try {
			if (cur.moveToNext()) {
				ret = new Session(mContext, cur);
			}
		} finally {
			cur.close();
		}
		return ret;
	}
	
	public Session getSession(Long id) {
		Session ret = null;
		
		Cursor cur = mDatabase.query(TABLE_NAME_SESSIONS, Session.PROJECTION, Session.ID + " = ?", new String[] { id.toString() }, null, null, null);
		try {
			if (cur.moveToNext()) {
				ret = new Session(mContext, cur);
			}
		} finally {
			cur.close();
		}
		return ret;
	}
	
	private String createSessionKey() {
		// generate 16 random bytes
		byte[] bytes = new byte[16];
		mRandom.nextBytes(bytes);
		
		StringBuilder sb = new StringBuilder();
		for (byte b : bytes) {
			sb.append(String.format("%02X", b));
		}
		
		return sb.toString();
	}
	
	public Session createSession(String packageName, String defaultEndpoint) {
		ContentValues values = new ContentValues();
		
		values.put(Session.PACKAGE_NAME, packageName);
		values.put(Session.DEFAULT_ENDPOINT, defaultEndpoint);
		values.put(Session.SESSION_KEY, createSessionKey());
		
		long rowid = mDatabase.insert(TABLE_NAME_SESSIONS, null, values);
		
		// return null on failure
		if (rowid == -1) return null;
		
		return getSession(rowid);
	}
	
	public void removeEndpoint(Endpoint e) {
		mDatabase.delete(TABLE_NAME_ENDPOINTS, Endpoint.ID + " = ?", new String[] { e.getId().toString() });
	}
	
	public Endpoint createEndpoint(Session s, String endpoint, boolean singleton, boolean fqeid) {
		ContentValues values = new ContentValues();
		
		values.put(Endpoint.SESSION, s.getId());
		values.put(Endpoint.ENDPOINT, endpoint);
		values.put(Endpoint.SINGLETON, singleton ? 1 : 0);
		values.put(Endpoint.FQEID, fqeid ? 1 : 0);
		
		try {
			long rowid = mDatabase.insert(TABLE_NAME_ENDPOINTS, null, values);
			
			// return null if the endpoint is already registered
			if (rowid == -1) return null;
			
			return getEndpoint(rowid);
		} catch (SQLException e) {
			return null;
		}
	}
	
	public Endpoint createEndpoint(Session s, GroupEndpoint group) {
		return createEndpoint(s, group.toString(), false, true);
	}
	
	public void setDefaultEndpoint(Session s, String defaultEndpoint) {
		ContentValues values = new ContentValues();
		values.put(Session.DEFAULT_ENDPOINT, defaultEndpoint);
		mDatabase.update(TABLE_NAME_SESSIONS, values, Session.ID + " = ?", new String[] { s.getId().toString() });
	}
}
