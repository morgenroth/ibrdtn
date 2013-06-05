package de.tubs.ibr.dtn.sharebox.data;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.LinkedList;
import java.util.List;

import android.annotation.SuppressLint;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.net.Uri;
import android.provider.BaseColumns;
import android.util.Log;
import de.tubs.ibr.dtn.api.BundleID;
import de.tubs.ibr.dtn.sharebox.ui.DownloadAdapter;
import de.tubs.ibr.dtn.sharebox.ui.PackageFileAdapter;

@SuppressLint("SimpleDateFormat")
public class Database {

    private final static String TAG = "Database";
    
    public final static String NOTIFY_DATABASE_UPDATED = "de.tubs.ibr.dtn.sharebox.DATABASE_UPDATED"; 
 
    private DBOpenHelper mHelper = null;
    private SQLiteDatabase mDatabase = null;
    private Context mContext = null;
    
    private static final String DATABASE_NAME = "transmissions";
    public static final String[] TABLE_NAMES = { "download", "files" };
    
    private static final int DATABASE_VERSION = 3;
    
    // Database creation sql statement
    private static final String DATABASE_CREATE_DOWNLOAD = 
            "CREATE TABLE " + TABLE_NAMES[0] + " (" +
                BaseColumns._ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
                Download.SOURCE + " TEXT NOT NULL, " +
                Download.DESTINATION + " TEXT NOT NULL, " +
                Download.TIMESTAMP + " TEXT, " +
                Download.LIFETIME + " INTEGER NOT NULL, " +
                Download.LENGTH + " INTEGER NOT NULL, " +
                Download.STATE + " INTEGER, " +
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
    
    public Download get(BundleID id) {
        Download d = null;
        
        try {
            Cursor cur = mDatabase.query(Database.TABLE_NAMES[0], DownloadAdapter.PROJECTION, Download.BUNDLE_ID + " = ?", new String[] { id.toString() }, null, null, null, "0, 1");
            
            if (cur.moveToNext())
            {
                d = new Download(mContext, cur, new DownloadAdapter.ColumnsMap());
            }
            
            cur.close();
        } catch (Exception e) {
            // download not found
            Log.e(TAG, "get() failed", e);
        }
        
        return d;
    }
    
    public Download get(Long downloadId) {
        Download d = null;
        
        try {
            Cursor cur = mDatabase.query(Database.TABLE_NAMES[0], DownloadAdapter.PROJECTION, Download.ID + " = ?", new String[] { downloadId.toString() }, null, null, null, "0, 1");
            
            if (cur.moveToNext())
            {
                d = new Download(mContext, cur, new DownloadAdapter.ColumnsMap());
            }
            
            cur.close();
        } catch (Exception e) {
            // download not found
            Log.e(TAG, "get() failed", e);
        }
        
        return d;
    }
    
    public List<PackageFile> getFiles(Long downloadId) {
        LinkedList<PackageFile> files = new LinkedList<PackageFile>();
        
        try {
            Cursor cur = mDatabase.query(Database.TABLE_NAMES[1], PackageFileAdapter.PROJECTION, PackageFile.DOWNLOAD + " = ?", new String[] { downloadId.toString() }, null, null, null);
            
            if (cur.moveToNext())
            {
                files.add( new PackageFile(mContext, cur, new PackageFileAdapter.ColumnsMap()) );
            }
            
            cur.close();
        } catch (Exception e) {
            // error
            Log.e(TAG, "getFiles() failed", e);
        }
        
        return files;
    }
    
    public Long put(Download d) {
        ContentValues values = new ContentValues();
        
        SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        
        values.put(Download.SOURCE, d.getSource());
        values.put(Download.DESTINATION, d.getDestination());
        values.put(Download.TIMESTAMP, dateFormat.format(d.getTimestamp()));
        values.put(Download.LIFETIME, d.getLifetime());
        values.put(Download.LENGTH, d.getLength());
        values.put(Download.STATE, d.getState());
        
        // write bundle id to bundle_id
        values.put(Download.BUNDLE_ID, d.getBundleId().toString());
        
        // store the message in the database
        Long id = mDatabase.insert(Database.TABLE_NAMES[0], null, values);
        
        notifyDataChanged();
        
        return id;
    }
    
    public Long put(BundleID bundle_id, File f) {
        ContentValues values = new ContentValues();
        
        Download d = get(bundle_id);
        
        if (d == null) return null;

        values.put(PackageFile.DOWNLOAD, d.getId());
        values.put(PackageFile.FILENAME, f.getAbsolutePath());
        values.put(PackageFile.LENGTH, f.length());
        
        // store the message in the database
        Long id = mDatabase.insert(Database.TABLE_NAMES[1], null, values);
        
        notifyDataChanged();
        
        return id;
    }
    
    public Integer getPending() {
        Integer ret = 0;
        
        try {
            Cursor cur = mDatabase.query(Database.TABLE_NAMES[0], new String[] { "COUNT(*)" }, Download.STATE + " = ?", new String[] { "0" }, null, null, null);

            if (cur.moveToNext())
            {
                ret = cur.getInt(0);
            }
            
            cur.close();
        } catch (Exception e) {
            // error
            Log.e(TAG, "getPending() failed", e);
        }
        
        return ret;
    }
    
    public void setState(Long id, Integer state) {
        ContentValues values = new ContentValues();

        values.put(Download.STATE, state);
        
        // update message data
        mDatabase.update(Database.TABLE_NAMES[0], values, "_id = ?", new String[] { String.valueOf(id) });
        
        notifyDataChanged();
    }
    
    public void clear() {
        // delete all files
        try {
            Cursor cur = mDatabase.query(Database.TABLE_NAMES[1], PackageFileAdapter.PROJECTION, null, null, null, null, null);
            
            if (cur.moveToNext())
            {
                File file = new File(cur.getString(new PackageFileAdapter.ColumnsMap().mColumnFilename));
                
                // delete the file
                file.delete();
                
                // announce removed file from the media library
                mContext.sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, Uri.fromFile(file.getParentFile())));
            }
            
            cur.close();
        } catch (Exception e) {
            Log.e(TAG, "clear() failed", e);
        }
        
        mDatabase.delete(Database.TABLE_NAMES[0], null, null);
        mDatabase.delete(Database.TABLE_NAMES[1], null, null);
        notifyDataChanged();
    }
    
    public void remove(BundleID id) {
        Download d = get(id);
        if (d == null) return;
        remove(d.getId());
    }
    
    public void remove(Long downloadId) {
        List<PackageFile> files = getFiles(downloadId);
        
        // delete the files
        for (PackageFile pf : files) {
            File f = new File(pf.getFilename());
            f.delete();
            
            // announce removed file from the media library
            mContext.sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, Uri.fromFile(f.getParentFile())));
        }
        
        mDatabase.delete(Database.TABLE_NAMES[0], Download.ID + " = ?", new String[] { downloadId.toString() });

        notifyDataChanged();
    }
}
