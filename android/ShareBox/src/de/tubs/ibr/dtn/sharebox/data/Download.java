package de.tubs.ibr.dtn.sharebox.data;

import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;

import android.annotation.SuppressLint;
import android.content.Context;
import android.database.Cursor;
import android.provider.BaseColumns;
import android.util.Log;
import de.tubs.ibr.dtn.api.Bundle;
import de.tubs.ibr.dtn.api.BundleID;
import de.tubs.ibr.dtn.sharebox.ui.DownloadAdapter;

@SuppressLint("SimpleDateFormat")
public class Download {

    private final static String TAG = "Download";
    
    public static final String ID = BaseColumns._ID;
    public static final String SOURCE = "source";
    public static final String DESTINATION = "destination";
    public static final String TIMESTAMP = "timestamp";
    public static final String LIFETIME = "lifetime";
    public static final String LENGTH = "length";
    public static final String STATE = "state";
    public static final String BUNDLE_ID = "bundleid";
    
    private Long mId = null;
    private String mSource = null;
    private String mDestination = null;
    private Date mTimestamp = null;
    private Long mLifetime = null;
    private Long mLength = null;
    private State mState = null;
    private BundleID mBundleId = null;
    
    public enum State {
    	PENDING(0),
    	ACCEPTED(1),
    	DOWNLOADING(2),
    	COMPLETED(3),
    	ABORTED(4);
    	
    	private final int mValue;
    	
    	State(int value) {
    		mValue = value;
    	}
    	
    	public int getValue() {
    		return mValue;
    	}
    	
    	public static State fromInt(int value) {
			for (State s : State.values()) {
			    if (s.getValue() == value) {
			        return s;
			    }
			}
			return null;
    	}
    }
    
    public Download(Context context, Cursor cursor, DownloadAdapter.ColumnsMap cmap) {
        final DateFormat formatter = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        
        mId = cursor.getLong(cmap.mColumnId);
        mSource = cursor.getString(cmap.mColumnSource);
        mDestination = cursor.getString(cmap.mColumnDestination);
        
        try {
            mTimestamp = formatter.parse(cursor.getString(cmap.mColumnTimestamp));
        } catch (ParseException e) {
            Log.e(TAG, "failed to convert date");
        }
        
        mLifetime = cursor.getLong(cmap.mColumnLifetime);
        mLength = cursor.getLong(cmap.mColumnLength);
        mState = State.fromInt( cursor.getInt(cmap.mColumnState) );
        
        // read bundle id from database
        setBundleId(BundleID.fromString(cursor.getString(cmap.mColumnBundleId)));
    }
    
    public Download(Bundle b) {
        mSource = b.getSource().toString();
        mDestination = b.getDestination().toString();
        mTimestamp = b.getTimestamp().getDate();
        mLifetime = b.getLifetime();
        mState = State.PENDING;
        
        setBundleId(new BundleID(b));
    }
    
    public Long getId() {
        return mId;
    }

    public String getSource() {
        return mSource;
    }

    public void setSource(String source) {
        mSource = source;
    }

    public String getDestination() {
        return mDestination;
    }

    public void setDestination(String destination) {
        mDestination = destination;
    }

    public Date getTimestamp() {
        return mTimestamp;
    }

    public void setTimestamp(Date timestamp) {
        mTimestamp = timestamp;
    }

    public Long getLifetime() {
        return mLifetime;
    }

    public void setLifetime(Long lifetime) {
        mLifetime = lifetime;
    }

    public Long getLength() {
        return mLength;
    }

    public void setLength(Long length) {
        mLength = length;
    }

    public Boolean isPending() {
        return (State.PENDING.equals(mState));
    }
    
    public State getState() {
        return mState;
    }

    public void setState(State state) {
        mState = state;
    }

    public BundleID getBundleId() {
        return mBundleId;
    }

    public void setBundleId(BundleID bundleId) {
        mBundleId = bundleId;
    }
}
