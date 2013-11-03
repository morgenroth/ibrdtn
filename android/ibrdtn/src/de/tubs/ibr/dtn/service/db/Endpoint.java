package de.tubs.ibr.dtn.service.db;

import android.content.Context;
import android.database.Cursor;
import android.provider.BaseColumns;

public class Endpoint {
	
	private static final String TAG = "Endpoint";

	public static final String ID = BaseColumns._ID;
	public static final String SESSION = "session";
	public static final String ENDPOINT = "endpoint";
	public static final String SINGLETON = "singleton";
	public static final String FQEID = "fqeid";
	
	private Long mId = null;
	private Long mSession = null;
	private String mEndpoint = null;
	private Boolean mSingleton = null;
	private Boolean mFqeid = null;		// Full qualified endpoint identifier
	
    public static final String[] PROJECTION = new String[] {
    	BaseColumns._ID,
    	Endpoint.SESSION,
    	Endpoint.ENDPOINT,
    	Endpoint.SINGLETON,
    	Endpoint.FQEID
    };
    
    // The indexes of the default columns which must be consistent
    // with above PROJECTION.
    static final int COLUMN_ENDPOINT_ID        = 0;
    static final int COLUMN_ENDPOINT_SESSION   = 1;
    static final int COLUMN_ENDPOINT_ENDPOINT  = 2;
    static final int COLUMN_ENDPOINT_SINGLETON = 3;
    static final int COLUMN_ENDPOINT_FQEID     = 4;
	
	public Endpoint(Context context, Cursor cursor) {
		mId = cursor.getLong(COLUMN_ENDPOINT_ID);
		mSession = cursor.getLong(COLUMN_ENDPOINT_SESSION);
		mEndpoint = cursor.getString(COLUMN_ENDPOINT_ENDPOINT);
		mSingleton = (cursor.getInt(COLUMN_ENDPOINT_SINGLETON) == 1);
		mFqeid = (cursor.getInt(COLUMN_ENDPOINT_FQEID) == 1);
	}

	public Long getId() {
		return mId;
	}

	public Long getSession() {
		return mSession;
	}

	public void setSession(Long session) {
		mSession = session;
	}

	public String getEndpoint() {
		return mEndpoint;
	}

	public void setEndpoint(String endpoint) {
		mEndpoint = endpoint;
	}

	public Boolean isSingleton() {
		return mSingleton;
	}

	public void setSingleton(Boolean singleton) {
		mSingleton = singleton;
	}

	public Boolean isFqeid() {
		return mFqeid;
	}

	public void setFqeid(Boolean fqeid) {
		mFqeid = fqeid;
	}	
}
