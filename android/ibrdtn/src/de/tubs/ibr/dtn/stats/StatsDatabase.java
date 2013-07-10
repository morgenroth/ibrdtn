package de.tubs.ibr.dtn.stats;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;

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

@SuppressLint("SimpleDateFormat")
public class StatsDatabase {
    private final static String TAG = "StatsDatabase";
    
    public final static String NOTIFY_DATABASE_UPDATED = "de.tubs.ibr.dtn.stats.DATABASE_UPDATED";
    
    private DBOpenHelper mHelper = null;
    private SQLiteDatabase mDatabase = null;
    private Context mContext = null;
    
    private static final String DATABASE_NAME = "stats";
    public static final String[] TABLE_NAMES = { "dtnd", "cl" };
    
    private static final int DATABASE_VERSION = 4;
    
    // Database creation sql statement
    private static final String DATABASE_CREATE_DTND = 
            "CREATE TABLE " + TABLE_NAMES[0] + " (" +
                BaseColumns._ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
                StatsEntry.TIMESTAMP + " TEXT, " +
                StatsEntry.UPTIME + " INTEGER, " +
                StatsEntry.NEIGHBORS + " INTEGER, " +
                StatsEntry.STORAGE_SIZE + " INTEGER, " +
                
                StatsEntry.CLOCK_OFFSET + " DOUBLE, " +
                StatsEntry.CLOCK_RATING + " DOUBLE, " +
                StatsEntry.CLOCK_ADJUSTMENTS + " INTEGER, " +
                
                StatsEntry.BUNDLE_ABORTED + " INTEGER, " +
                StatsEntry.BUNDLE_EXPIRED + " INTEGER, " +
                StatsEntry.BUNDLE_GENERATED + " INTEGER, " +
                StatsEntry.BUNDLE_QUEUED + " INTEGER, " +
                StatsEntry.BUNDLE_RECEIVED + " INTEGER, " +
                StatsEntry.BUNDLE_REQUEUED + " INTEGER, " +
                StatsEntry.BUNDLE_STORED + " INTEGER, " +
                StatsEntry.BUNDLE_TRANSMITTED + " INTEGER" +
            ");";
    
    private static final String DATABASE_CREATE_CL = 
            "CREATE TABLE " + TABLE_NAMES[1] + " (" +
                BaseColumns._ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
                ConvergenceLayerStatsEntry.TIMESTAMP + " TEXT, " +
                ConvergenceLayerStatsEntry.CONVERGENCE_LAYER + " TEXT, " +
                ConvergenceLayerStatsEntry.DATA_TAG + " TEXT, " +
                ConvergenceLayerStatsEntry.DATA_VALUE + " DOUBLE" +
            ");";

    private class DBOpenHelper extends SQLiteOpenHelper {
        
        public DBOpenHelper(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            db.execSQL(DATABASE_CREATE_DTND);
            db.execSQL(DATABASE_CREATE_CL);
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            if ((oldVersion == 2) && (newVersion == 3)) {
                Log.w(TAG, "Upgrading database from version " + oldVersion + " to " + newVersion);
                db.execSQL(DATABASE_CREATE_CL);
            } else {
                Log.w(TAG, "Upgrading database from version " + oldVersion + " to "
                        + newVersion + ", which will destroy all old data");
                
                for (String table : TABLE_NAMES) {
                    db.execSQL("DROP TABLE IF EXISTS " + table);
                }
                onCreate(db);
            }
        }
    };
    
    public StatsDatabase(Context context) throws SQLException
    {
        mHelper = new DBOpenHelper(context);
        mDatabase = mHelper.getWritableDatabase();
        mContext = context;
    }
    
    public void notifyDataChanged() {
        Intent i = new Intent(NOTIFY_DATABASE_UPDATED);
        mContext.sendBroadcast(i);
    }
    
    public SQLiteDatabase getDB() {
        return mDatabase;
    }
    
    public StatsEntry getStats(Long id) {
        StatsEntry se = null;
        
        try {
            Cursor cur = mDatabase.query(StatsDatabase.TABLE_NAMES[0], StatsEntry.PROJECTION, StatsEntry.ID + " = ?", new String[] { id.toString() }, null, null, null, "0, 1");
            
            if (cur.moveToNext())
            {
                se = new StatsEntry(mContext, cur, new StatsEntry.ColumnsMap());
            }
            
            cur.close();
        } catch (Exception e) {
            // entry not found
            Log.e(TAG, "get() failed", e);
        }
        
        return se;
    }
    
    public ConvergenceLayerStatsEntry getClStats(Long id) {
        ConvergenceLayerStatsEntry se = null;
        
        try {
            Cursor cur = mDatabase.query(StatsDatabase.TABLE_NAMES[1], ConvergenceLayerStatsEntry.PROJECTION, ConvergenceLayerStatsEntry.ID + " = ?", new String[] { id.toString() }, null, null, null, "0, 1");
            
            if (cur.moveToNext())
            {
                se = new ConvergenceLayerStatsEntry(mContext, cur, new ConvergenceLayerStatsEntry.ColumnsMap());
            }
            
            cur.close();
        } catch (Exception e) {
            // entry not found
            Log.e(TAG, "get() failed", e);
        }
        
        return se;
    }
    
    public Long put(StatsEntry e) {
        // store the stats object into the database
        ContentValues values = new ContentValues();
        
        SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        
        values.put(StatsEntry.TIMESTAMP, dateFormat.format(e.getTimestamp()));

        values.put(StatsEntry.UPTIME, e.getUptime());
        values.put(StatsEntry.NEIGHBORS, e.getNeighbors());
        values.put(StatsEntry.STORAGE_SIZE, e.getStorageSize());
        
        values.put(StatsEntry.CLOCK_OFFSET, e.getClockOffset());
        values.put(StatsEntry.CLOCK_RATING, e.getClockRating());
        values.put(StatsEntry.CLOCK_ADJUSTMENTS, e.getClockAdjustments());
        
        values.put(StatsEntry.BUNDLE_ABORTED, e.getBundleAborted());
        values.put(StatsEntry.BUNDLE_EXPIRED, e.getBundleExpired());
        values.put(StatsEntry.BUNDLE_GENERATED, e.getBundleGenerated());
        values.put(StatsEntry.BUNDLE_QUEUED, e.getBundleQueued());
        values.put(StatsEntry.BUNDLE_RECEIVED, e.getBundleReceived());
        values.put(StatsEntry.BUNDLE_REQUEUED, e.getBundleRequeued());
        values.put(StatsEntry.BUNDLE_STORED, e.getBundleStored());
        values.put(StatsEntry.BUNDLE_TRANSMITTED, e.getBundleTransmitted());
        
        // store the message in the database
        Long id = mDatabase.insert(StatsDatabase.TABLE_NAMES[0], null, values);
        
        Log.d(TAG, "stats stored: " + e.toString());
        
        // finally purge old data
        purge();
        
        notifyDataChanged();
        
        return id;
    }
    
    public Long put(ConvergenceLayerStatsEntry e) {
        // store the stats object into the database
        ContentValues values = new ContentValues();
        
        SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        
        values.put(ConvergenceLayerStatsEntry.TIMESTAMP, dateFormat.format(e.getTimestamp()));

        values.put(ConvergenceLayerStatsEntry.CONVERGENCE_LAYER, e.getConvergenceLayer());
        values.put(ConvergenceLayerStatsEntry.DATA_TAG, e.getDataTag());
        values.put(ConvergenceLayerStatsEntry.DATA_VALUE, e.getDataValue());
        
        // store the message in the database
        Long id = mDatabase.insert(StatsDatabase.TABLE_NAMES[1], null, values);
        
        Log.d(TAG, "cl stats stored: " + e.toString());
        
        notifyDataChanged();
        
        return id;
    }
    
    public void purge() {
        final DateFormat formatter = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        
        // generate a time limit (14 days)
        Calendar cal = Calendar.getInstance();
        cal.add(Calendar.DATE, -14);
        String timestamp_limit = formatter.format(cal.getTime());
        
        mDatabase.delete(StatsDatabase.TABLE_NAMES[0], StatsEntry.TIMESTAMP + " < ?", new String[] { timestamp_limit });
        mDatabase.delete(StatsDatabase.TABLE_NAMES[1], ConvergenceLayerStatsEntry.TIMESTAMP + " < ?", new String[] { timestamp_limit });
    }

    public void close()
    {
        mDatabase.close();
        mHelper.close();
    }
}
