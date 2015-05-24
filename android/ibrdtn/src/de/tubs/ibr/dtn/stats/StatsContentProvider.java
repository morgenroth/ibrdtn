
package de.tubs.ibr.dtn.stats;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.net.Uri;
import android.os.Bundle;
import android.provider.BaseColumns;
import android.util.Log;

public class StatsContentProvider extends ContentProvider {
	private final static String TAG = "StatsContentProvider";
	
	private static final String AUTHORITY = "de.tubs.ibr.dtn.contentprovider";
	
	public static final Uri STATS_URI = Uri.parse("content://" + AUTHORITY + "/stats");
	public static final Uri CL_STATS_URI = Uri.parse("content://" + AUTHORITY + "/stats/cl");
	
    public final static String NOTIFY_DATABASE_UPDATED = "de.tubs.ibr.dtn.stats.DATABASE_UPDATED";
    
    private DBOpenHelper mHelper = null;
    private SQLiteDatabase mDatabase = null;
    
    private static final String DATABASE_NAME = "stats";
    private static final String[] TABLE_NAMES = { "dtnd", "cl" };
	
    private static final int DATABASE_VERSION = 6;
    
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
                StatsEntry.BUNDLE_QUEUED + " INTEGER, " +
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
            } else if ((oldVersion == 5) && (newVersion == 6)) {
                Log.w(TAG, "Upgrading database from version " + oldVersion + " to " + newVersion);
                // drop stats table
                db.execSQL("DROP TABLE IF EXISTS " + TABLE_NAMES[0]);
                db.execSQL(DATABASE_CREATE_DTND);
            } else if ((oldVersion == 4) && (newVersion == 5)) {
                Log.w(TAG, "Upgrading database from version " + oldVersion + " to " + newVersion);
                // drop stats table
                db.execSQL("DROP TABLE IF EXISTS " + TABLE_NAMES[0]);
                db.execSQL(DATABASE_CREATE_DTND);
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
	
	public StatsContentProvider() {
	}

	@Override
	public int delete(Uri uri, String selection, String[] selectionArgs) {
		// Implement this to handle requests to delete one or more rows.
		throw new UnsupportedOperationException("Not yet implemented");
	}

	@Override
	public String getType(Uri uri) {
		// TODO: Implement this to handle requests for the MIME type of the data
		// at the given URI.
		throw new UnsupportedOperationException("Not yet implemented");
	}

	@Override
	public Uri insert(Uri uri, ContentValues values) {
		if (STATS_URI.equals(uri)) {
			mDatabase.insert(TABLE_NAMES[0], null, values);
			
			// finally purge old data
			purge();
		}
		else if (CL_STATS_URI.equals(uri)) {
			mDatabase.insert(TABLE_NAMES[1], null, values);
		}

		notifyDataChanged();

		return uri;
	}

	@Override
	public boolean onCreate() {
		mHelper = new DBOpenHelper(getContext());
		mDatabase = mHelper.getWritableDatabase();
		return true;
	}

	private void notifyDataChanged() {
		Intent i = new Intent(NOTIFY_DATABASE_UPDATED);
		getContext().sendBroadcast(i);
	}

	@Override
	public Cursor query(Uri uri, String[] projection, String selection,
			String[] selectionArgs, String sortOrder) {
		
		if (STATS_URI.equals(uri)) {
			return mDatabase.query(TABLE_NAMES[0], projection, selection, selectionArgs, null, null, sortOrder);			
		}
		else if (CL_STATS_URI.equals(uri)) {
			return mDatabase.query(TABLE_NAMES[1], projection, selection, selectionArgs, null, null, sortOrder);
		}
		else {
			return null;
		}
	}

	@Override
	public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
		throw new UnsupportedOperationException("Not yet implemented");
	}

	public static void store(Context context, Bundle stats) {
		try {
			// get stats entry
			stats.setClassLoader(context.getClassLoader());
			StatsEntry e = stats.getParcelable("stats");

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
			values.put(StatsEntry.BUNDLE_QUEUED, e.getBundleQueued());
			values.put(StatsEntry.BUNDLE_REQUEUED, e.getBundleRequeued());
			values.put(StatsEntry.BUNDLE_STORED, e.getBundleStored());
			values.put(StatsEntry.BUNDLE_TRANSMITTED, e.getBundleTransmitted());

			// insert stats into the database
			context.getContentResolver().insert(StatsContentProvider.STATS_URI, values);

			// store all convergence-layer stats
			ArrayList<ConvergenceLayerStatsEntry> cl_stats = stats.getParcelableArrayList("clstats");

			for (ConvergenceLayerStatsEntry cl : cl_stats) {
				store(context, cl);
			}
		} catch (Exception e) {
			Log.e(TAG, "store failed", e);
		}
	}

	public static void store(Context context, ConvergenceLayerStatsEntry e) {
		try {
			// store the stats object into the database
			ContentValues values = new ContentValues();
	
			SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
	
			values.put(ConvergenceLayerStatsEntry.TIMESTAMP, dateFormat.format(e.getTimestamp()));
	
			values.put(ConvergenceLayerStatsEntry.CONVERGENCE_LAYER, e.getConvergenceLayer());
			values.put(ConvergenceLayerStatsEntry.DATA_TAG, e.getDataTag());
			values.put(ConvergenceLayerStatsEntry.DATA_VALUE, e.getDataValue());
	
			// store the message in the database
			context.getContentResolver().insert(StatsContentProvider.CL_STATS_URI, values);
		} catch (Exception e1) {
			Log.e(TAG, "store failed", e1);
		}
	}

	private void purge() {
		final DateFormat formatter = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

		// generate a time limit (14 days)
		Calendar cal = Calendar.getInstance();
		cal.add(Calendar.DATE, -14);
		String timestamp_limit = formatter.format(cal.getTime());

		mDatabase.delete(TABLE_NAMES[0], StatsEntry.TIMESTAMP + " < ?", new String[] {
			timestamp_limit
		});
		mDatabase.delete(TABLE_NAMES[1], ConvergenceLayerStatsEntry.TIMESTAMP + " < ?",
				new String[] {
					timestamp_limit
				});
	}
}
