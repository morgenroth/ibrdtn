package de.tubs.ibr.dtn.stats;

import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;

import android.annotation.SuppressLint;
import android.content.Context;
import android.database.Cursor;
import android.os.Parcel;
import android.os.Parcelable;
import android.provider.BaseColumns;
import android.util.Log;
import de.tubs.ibr.dtn.api.Timestamp;
import de.tubs.ibr.dtn.swig.NativeStats;

public class ConvergenceLayerStatsEntry implements Parcelable {
    
    private final static String TAG = "ClStatsEntry";
    
    public static final String ID = BaseColumns._ID;
    public static final String TIMESTAMP = "timestamp";
    
    public static final String CONVERGENCE_LAYER = "convergence_layer";
    public static final String DATA_TAG = "data_tag";
    public static final String DATA_VALUE = "data_value";
    
    private Long mId = null;
    private Date mTimestamp = null;
    private String mConvergenceLayer = null;
    private String mDataTag = null;
    private Double mDataValue = null;
    
    public static final String[] PROJECTION = new String[] {
        BaseColumns._ID,
        ConvergenceLayerStatsEntry.TIMESTAMP,
        ConvergenceLayerStatsEntry.CONVERGENCE_LAYER,
        ConvergenceLayerStatsEntry.DATA_TAG,
        ConvergenceLayerStatsEntry.DATA_VALUE
    };
    
    // The indexes of the default columns which must be consistent
    // with above PROJECTION.
    static final int COLUMN_STATS_ID                 = 0;
    static final int COLUMN_STATS_TIMESTAMP          = 1;
    
    static final int COLUMN_STATS_CONVERGENCE_LAYER  = 2;
    static final int COLUMN_STATS_DATA_TAG           = 3;
    static final int COLUMN_STATS_DATA_VALUE         = 4;

    @SuppressLint("SimpleDateFormat")
    public ConvergenceLayerStatsEntry(Context context, Cursor cursor, ColumnsMap cmap) {
        final DateFormat formatter = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        
        mId = cursor.getLong(cmap.mColumnId);
        
        setConvergenceLayer(cursor.getString(cmap.mConvergenceLayer));
        setDataTag(cursor.getString(cmap.mDataTag));
        setDataValue(cursor.getDouble(cmap.mDataValue));
        
        try {
            mTimestamp = formatter.parse(cursor.getString(cmap.mColumnTimestamp));
        } catch (ParseException e) {
            Log.e(TAG, "failed to convert date");
        }
    }
    
    public ConvergenceLayerStatsEntry(NativeStats stats, String tag, int index) {
        mTimestamp = (new Timestamp(stats.getTimestamp())).getDate();
        
        // split tag at "|"
        String[] tag_split = tag.split("\\|");
        
        mConvergenceLayer = tag_split[0];
        mDataTag = tag_split[1];
        mDataValue = Double.valueOf( stats.getData(index) );
    }
    
    public Long getId() {
        return mId;
    }
    
    public Date getTimestamp() {
        return mTimestamp;
    }

    public void setTimestamp(Date timestamp) {
        mTimestamp = timestamp;
    }

    public String getConvergenceLayer() {
        return mConvergenceLayer;
    }

    public void setConvergenceLayer(String convergenceLayer) {
        mConvergenceLayer = convergenceLayer;
    }

    public String getDataTag() {
        return mDataTag;
    }

    public void setDataTag(String dataTag) {
        mDataTag = dataTag;
    }

    public Double getDataValue() {
        return mDataValue;
    }

    public void setDataValue(Double dataValue) {
        mDataValue = dataValue;
    }

    public static class ColumnsMap {
        public int mColumnId;
        public int mColumnTimestamp;
        public int mConvergenceLayer;
        public int mDataTag;
        public int mDataValue;

        public ColumnsMap() {
            mColumnTimestamp   = COLUMN_STATS_TIMESTAMP;
            mConvergenceLayer = COLUMN_STATS_CONVERGENCE_LAYER;
            mDataTag = COLUMN_STATS_DATA_TAG;
            mDataValue = COLUMN_STATS_DATA_VALUE;
        }

        public ColumnsMap(Cursor cursor) {
            // Ignore all 'not found' exceptions since the custom columns
            // may be just a subset of the default columns.
            try {
                mColumnId = cursor.getColumnIndexOrThrow(BaseColumns._ID);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
                mColumnTimestamp = cursor.getColumnIndexOrThrow(ConvergenceLayerStatsEntry.TIMESTAMP);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }
        }
    }
    
    public static final Parcelable.Creator<ConvergenceLayerStatsEntry> CREATOR = new Parcelable.Creator<ConvergenceLayerStatsEntry>() {
        public ConvergenceLayerStatsEntry createFromParcel(Parcel in) {
            return new ConvergenceLayerStatsEntry(in);
        }

        public ConvergenceLayerStatsEntry[] newArray(int size) {
            return new ConvergenceLayerStatsEntry[size];
        }
    };
    
    public ConvergenceLayerStatsEntry(Parcel in) {
        mTimestamp = new Date(in.readLong());
        mConvergenceLayer = in.readString();
        mDataTag = in.readString();
        mDataValue = in.readDouble();
    }

	@Override
	public int describeContents() {
		return 0;
	}

	@Override
	public void writeToParcel(Parcel dest, int flags) {
		dest.writeLong( mTimestamp.getTime() );
		dest.writeString( mConvergenceLayer );
		dest.writeString( mDataTag );
		dest.writeDouble( mDataValue );
	}
}
