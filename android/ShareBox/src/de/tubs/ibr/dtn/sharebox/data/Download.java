package de.tubs.ibr.dtn.sharebox.data;

import java.util.Date;

import de.tubs.ibr.dtn.sharebox.ui.DownloadAdapter;

import android.content.Context;
import android.database.Cursor;
import android.provider.BaseColumns;

public class Download {

    @SuppressWarnings("unused")
    private final static String TAG = "Download";
    
    public static final String ID = BaseColumns._ID;
    public static final String SOURCE = "source";
    public static final String DESTINATION = "destination";
    public static final String TIMESTAMP = "timestamp";
    public static final String LIFETIME = "lifetime";
    public static final String LENGTH = "length";
    public static final String PENDING = "pending";
    
    private Long mId = null;
    private String mSource = null;
    private String mDestination = null;
    private Date mTimestamp = null;
    private Long mLifetime = null;
    private Long mLength = null;
    private Boolean mPending = null;
    
    public Download(Context context, Cursor cursor, DownloadAdapter.ColumnsMap cmap) {
        mId = cursor.getLong(cmap.mColumnId);
        mSource = cursor.getString(cmap.mColumnSource);
        mDestination = cursor.getString(cmap.mColumnDestination);
    }
    
    public Download(String source, String destination) {
        mSource = source;
        mDestination = destination;
        mTimestamp = new Date();
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
        return mPending;
    }

    public void setPending(Boolean pending) {
        mPending = pending;
    }
}
