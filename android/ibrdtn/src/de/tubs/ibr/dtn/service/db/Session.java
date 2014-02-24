package de.tubs.ibr.dtn.service.db;

import android.content.Context;
import android.database.Cursor;
import android.provider.BaseColumns;

public class Session implements Comparable<Session> {
	
	public static final String ID = BaseColumns._ID;
	public static final String PACKAGE_NAME = "packageName";
	public static final String SESSION_KEY = "sessionKey";
	public static final String DEFAULT_ENDPOINT = "defaultEndpoint";
	
	private Long mId = null;
	private String mPackageName = null;
	private String mSessionKey = null;
	private String mDefaultEndpoint = null;

    public static final String[] PROJECTION = new String[] {
    	BaseColumns._ID,
    	Session.PACKAGE_NAME,
    	Session.SESSION_KEY,
    	Session.DEFAULT_ENDPOINT
    };
    
    // The indexes of the default columns which must be consistent
    // with above PROJECTION.
    static final int COLUMN_SESSION_ID             = 0;
    static final int COLUMN_SESSION_PACKAGE_NAME   = 1;
    static final int COLUMN_SESSION_KEY            = 2;
    static final int COLUMN_SESSION_ENDPOINT       = 3;
	
	public Session(Context context, Cursor cursor) {
		mId = cursor.getLong(COLUMN_SESSION_ID);
		mPackageName = cursor.getString(COLUMN_SESSION_PACKAGE_NAME);
		mSessionKey = cursor.getString(COLUMN_SESSION_KEY);
		mDefaultEndpoint = cursor.isNull(COLUMN_SESSION_ENDPOINT) ? null : cursor.getString(COLUMN_SESSION_ENDPOINT);
	}

	public Long getId() {
		return mId;
	}

	public String getPackageName() {
		return mPackageName;
	}

	public void setPackageName(String packageName) {
		mPackageName = packageName;
	}

	public String getSessionKey() {
		return mSessionKey;
	}

	public void setSessionKey(String sessionKey) {
		mSessionKey = sessionKey;
	}

	@Override
	public int compareTo(Session other) {
		return mId.compareTo(other.mId);
	}

	@Override
	public boolean equals(Object other) {
		if (other instanceof Session) {
			return mId.equals(((Session)other).mId);
		}
		return super.equals(other);
	}

	@Override
	public int hashCode() {
		return mId.hashCode();
	}

	@Override
	public String toString() {
		return mId.toString();
	}

	public String getDefaultEndpoint() {
		return mDefaultEndpoint;
	}

	public void setDefaultEndpoint(String defaultEndpoint) {
		mDefaultEndpoint = defaultEndpoint;
	}
}
