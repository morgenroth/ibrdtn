package de.tubs.ibr.dtn.sharebox.data;

import java.text.SimpleDateFormat;

import android.annotation.SuppressLint;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.provider.BaseColumns;
import android.util.Log;

@SuppressLint("SimpleDateFormat")
public class Database {
    @SuppressWarnings("unused")
    private final static String TAG = "Database";
    
    public final static String NOTIFY_DATABASE_UPDATED = "de.tubs.ibr.dtn.sharebox.DATABASE_UPDATED"; 
 
    private DBOpenHelper mHelper = null;
    private SQLiteDatabase mDatabase = null;
    private Context mContext = null;
    
    private static final String DATABASE_NAME = "transmissions";
    public static final String[] TABLE_NAMES = { "download", "files" };
    
    private static final int DATABASE_VERSION = 2;
    
    // Database creation sql statement
    private static final String DATABASE_CREATE_DOWNLOAD = 
            "CREATE TABLE " + TABLE_NAMES[0] + " (" +
                BaseColumns._ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
                Download.SOURCE + " TEXT NOT NULL, " +
                Download.DESTINATION + " TEXT NOT NULL, " +
                Download.TIMESTAMP + " TEXT, " +
                Download.LIFETIME + " INTEGER NOT NULL, " +
                Download.LENGTH + " INTEGER NOT NULL, " +
                Download.PENDING + " INTEGER, " +
                Download.BUNDLE_ID + " TEXT NOT NULL" +
            ");";
    
    private static final String DATABASE_CREATE_FILES = 
            "CREATE TABLE " + TABLE_NAMES[1] + " (" +
                BaseColumns._ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
                PackageFile.DOWNLOAD + " INTEGER NOT NULL, " +
                PackageFile.FILENAME + " TEXT NOT NULL, " +
                PackageFile.LENGTH + " INTEGER NOT NULL " +
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
            db.execSQL(DATABASE_CREATE_DOWNLOAD);
            db.execSQL(DATABASE_CREATE_FILES);
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            Log.w(DBOpenHelper.class.getName(),
                    "Upgrading database from version " + oldVersion + " to "
                            + newVersion + ", which will destroy all old data");
            
            for (String table : TABLE_NAMES) {
                db.execSQL("DROP TABLE IF EXISTS " + table);
            }
            onCreate(db);
        }
    };
    
    public Database(Context context) throws SQLException
    {
        mHelper = new DBOpenHelper(context);
        mDatabase = mHelper.getWritableDatabase();
        mContext = context;
    }
    
    public void notifyDataChanged() {
        Intent i = new Intent(NOTIFY_DATABASE_UPDATED);
        mContext.sendBroadcast(i);
    }
    
    public void close()
    {
        mDatabase.close();
        mHelper.close();
    }
    
    public Long put(Download d) {
        ContentValues values = new ContentValues();
        
        SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        
        values.put(Download.SOURCE, d.getSource());
        values.put(Download.DESTINATION, d.getDestination());
        values.put(Download.TIMESTAMP, dateFormat.format(d.getTimestamp()));
        values.put(Download.LIFETIME, d.getLifetime());
        values.put(Download.LENGTH, d.getLength());
        values.put(Download.PENDING, d.isPending() ? 1 : 0);
        
        // write bundle id to bundle_id
        values.put(Download.BUNDLE_ID, d.getBundleId().toString());
        
        // store the message in the database
        Long id = mDatabase.insert(Database.TABLE_NAMES[0], null, values);
        
        notifyDataChanged();
        
        return id;
    }
}
