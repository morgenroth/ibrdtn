package de.tubs.ibr.dtn.service.db;

import java.io.Closeable;
import java.util.LinkedList;
import java.util.List;

import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.provider.BaseColumns;
import android.util.Log;

public class ApiDatabase implements Closeable {
	private final String TAG = "ApiDatabase";
	
	private DBOpenHelper mHelper = null;
	private SQLiteDatabase mDatabase = null;
	private Context mContext = null;

	public static final String TABLE_NAME_SESSIONS = "sessions";
	public static final String TABLE_NAME_ENDPOINTS = "endpoints";
	
	private static final String DATABASE_CREATE_SESSIONS =
			"CREATE TABLE " + TABLE_NAME_SESSIONS + " (" +
				BaseColumns._ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
				Session.PACKAGE_NAME + " TEXT NOT NULL, " +
				Session.SESSION_KEY + " TEXT NOT NULL, " +
				Session.DEFAULT_ENDPOINT + " TEXT" +
			");";
	
	private static final String DATABASE_CREATE_ENDPOINTS = 
			"CREATE TABLE " + TABLE_NAME_ENDPOINTS + " (" +
				BaseColumns._ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
				Endpoint.SESSION + " INTEGER NOT NULL, " +
				Endpoint.ENDPOINT + " TEXT NOT NULL, " +
				Endpoint.SINGLETON + " INTEGER NOT NULL, " +
				Endpoint.FQEID + " INTEGER NOT NULL" +
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

	public void open(Context context) throws SQLException
	{
		mContext = context;
		mHelper = new DBOpenHelper(mContext);
		mDatabase = mHelper.getWritableDatabase();
	}
	
	public SQLiteDatabase getDatabase()
	{
		return mDatabase;
	}
	
	public void close()
	{
		mHelper.close();
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
	
	public void removeSession(Session s) {
		// TODO: implement this		
	}
	
	public Session getSessionByPackageName(String packageName) {
		// TODO: implement this
		return null;
	}
	
	public Session getSession(String sessionKey) {
		// TODO: implement this
		return null;
	}
	
	public Session createSession(String packageName, String defaultEndpoint) {
		// TODO: implement this
		return null;
	}
	
	public void removeEndpoint(Endpoint e) {
		// TODO: implement this
	}
	
	public Endpoint createEndpoint(Session s, String endpoint, boolean singleton, boolean fqeid) {
		// TODO: implement this
		return null;
	}
}
