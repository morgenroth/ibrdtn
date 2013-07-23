package de.tubs.ibr.dtn.p2p.db;

import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

public class Database {

    private static final String TAG = "Database";

    private static Object syncObject = new Object();

    private static volatile Database _instance;

    public static Database getInstance(Context context) {
        if (_instance == null) {
            synchronized (syncObject) {
                if (_instance == null) {
                    _instance = new Database(context.getApplicationContext());
                }
            }
        }
        _instance.open();
        return _instance;
    }

    public SQLiteDatabase db;
    private DatabaseOpenHelper helper;
    
    public static final String NOTIFY_PEERS_CHANGED = "de.tubs.ibr.dtn.p2p.NOTIFY_PEERS_CHANGED";
    public static final String PEER_TABLE = "peers";

    private static final String CREATE_PEER_TABLE = 
                "CREATE TABLE " + PEER_TABLE + " (" + 
                    Peer.ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
                    Peer.MAC_ADDRESS + " TEXT, " +
                    Peer.EID + " TEXT, " +
                    Peer.LAST_CONTACT + " DATETIME, " +
                    Peer.EVER_CONNECTED + " INTEGER" +
                ");";

    private static class DatabaseOpenHelper extends SQLiteOpenHelper {

        private static final int DATABASE_VERSION = 1;
        private static final String DATABASE_NAME = "p2p.db";
        private static final String TAG = "DatabaseOpenHelper";

        public DatabaseOpenHelper(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            if (Log.isLoggable(TAG, Log.DEBUG)) {
                Log.d(TAG, CREATE_PEER_TABLE);
            }
            db.execSQL(CREATE_PEER_TABLE);
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            if (Log.isLoggable(TAG, Log.DEBUG)) {
                Log.d(TAG, "DROP TABLE IF EXISTS " + PEER_TABLE);
            }
            db.execSQL("DROP TABLE IF EXISTS " + PEER_TABLE);

            onCreate(db);
        }

    }

    private Database(Context context) {
        this.helper = new DatabaseOpenHelper(context);
    }

    public void open() throws SQLException {
        db = helper.getWritableDatabase();
    }

    /**
     * @deprecated unimportant
     */
    public void close() {
        helper.close();
    }

    /**
     * Inserts peer into database. If this peer already exists in the database
     * it is just updated.
     * 
     * @param peer
     *            Peer to insert or update in the database
     */
    public synchronized void put(Context context, Peer peer) {
        ContentValues values = new ContentValues();
        
        values.put(Peer.MAC_ADDRESS, peer.getMacAddress());
        values.put(Peer.EID, peer.getEid());
        values.put(Peer.LAST_CONTACT, peer.getLastContact().getTime());
        values.put(Peer.EVER_CONNECTED, peer.isEverConnected() ? 1 : 0);

        // check if the entry already exists
        Cursor c = db.query(PEER_TABLE, new String[] { Peer.MAC_ADDRESS }, Peer.MAC_ADDRESS + " = ?",
                new String[] { peer.getMacAddress() }, null, null, null);
        
        if (c.getCount() == 0) {
            long ret = db.insert(PEER_TABLE, null, values);
            // if (Log.isLoggable(TAG, Log.DEBUG)) {
            Log.d(TAG, "Insert:" + peer.toString() + ":" + ret);
            // }
        } else {
            long ret = db.update(PEER_TABLE, values, Peer.MAC_ADDRESS + " = ?", new String[] { peer.getMacAddress() });
            // if (Log.isLoggable(TAG, Log.DEBUG)) {
            Log.d(TAG, "Update:" + peer.toString() + ":" + ret);
            // }
        }
        c.close();
        
        Database.notifyPeersChanged(context);
    }
    
    public static void notifyPeersChanged(Context context) {
        Intent i = new Intent(Database.NOTIFY_PEERS_CHANGED);
        context.sendBroadcast(i);
    }

    public synchronized Peer find(String mac) {
        Peer p = null;
        String selection = String.format("%s = ?", Peer.MAC_ADDRESS);
        String[] selectionArgs = { mac };
        Cursor cursor = db.query(PEER_TABLE, PeerAdapter.PROJECTION, selection,
                selectionArgs, null, null, null);
        // if (Log.isLoggable(TAG, Log.DEBUG)) {
        Log.d(TAG, "Select peer " + mac + ",count:" + cursor.getCount());
        // }
        if (cursor.getCount() == 1 && cursor.moveToFirst()) {
            p = new Peer(cursor, new PeerAdapter.ColumnsMap());
        }
        cursor.close();
        return p;
    }
}
