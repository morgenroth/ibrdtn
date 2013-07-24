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

    private Context mContext = null;
    public SQLiteDatabase db = null;
    private DatabaseOpenHelper helper = null;
    
    public static final String NOTIFY_PEERS_CHANGED = "de.tubs.ibr.dtn.p2p.NOTIFY_PEERS_CHANGED";
    public static final String PEER_TABLE = "peers";

    private static final String CREATE_PEER_TABLE = 
                "CREATE TABLE " + PEER_TABLE + " (" + 
                    Peer.ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
                    Peer.P2P_ADDRESS + " TEXT, " +
                    Peer.ENDPOINT + " TEXT, " +
                    Peer.LAST_SEEN + " DATETIME, " +
                    Peer.CONNECT_STATE + " INTEGER, " +
                    Peer.CONNECT_UPDATE + " DATETIME" +
                ");";

    private static class DatabaseOpenHelper extends SQLiteOpenHelper {

        private static final int DATABASE_VERSION = 2;
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
        this.mContext = context;
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
    public synchronized void put(Peer peer) {
        ContentValues values = new ContentValues();
        
        values.put(Peer.P2P_ADDRESS, peer.getP2pAddress());
        
        if (peer.getEndpoint() == null) {
            values.putNull(Peer.ENDPOINT);
        } else {
            values.put(Peer.ENDPOINT, peer.getEndpoint());
        }
        
        values.put(Peer.LAST_SEEN, peer.getLastSeen().getTime());

        // check if the entry already exists
        Cursor c = db.query(PEER_TABLE, new String[] { Peer.P2P_ADDRESS }, Peer.P2P_ADDRESS + " = ?",
                new String[] { peer.getP2pAddress() }, null, null, null);
        
        if (c.getCount() == 0) {
            values.put(Peer.CONNECT_STATE, peer.getConnectState());
            
            if (peer.getConnectUpdate() == null) {
                values.putNull(Peer.CONNECT_UPDATE);
            } else {
                values.put(Peer.CONNECT_UPDATE, peer.getConnectUpdate().getTime());
            }
            
            db.insert(PEER_TABLE, null, values);
        } else {
            db.update(PEER_TABLE, values, Peer.P2P_ADDRESS + " = ?", new String[] { peer.getP2pAddress() });
        }
        
        // close the cursor
        c.close();
        
        Database.notifyPeersChanged(mContext);
    }
    
    public static void notifyPeersChanged(Context context) {
        Intent i = new Intent(Database.NOTIFY_PEERS_CHANGED);
        context.sendBroadcast(i);
    }

    public synchronized Peer find(String mac) {
        Peer p = null;
        Cursor cursor = db.query(PEER_TABLE, PeerAdapter.PROJECTION, Peer.P2P_ADDRESS + " = ?", new String[] { mac }, null, null, null);

        if (cursor.moveToFirst()) {
            p = new Peer(cursor, new PeerAdapter.ColumnsMap());
        }
        
        cursor.close();
        return p;
    }
    
    public synchronized void updateState(Peer peer) {
        ContentValues values = new ContentValues();

        values.put(Peer.CONNECT_STATE, peer.getConnectState());

        if (peer.getConnectUpdate() == null) {
            values.putNull(Peer.CONNECT_UPDATE);
        } else {
            values.put(Peer.CONNECT_UPDATE, peer.getConnectUpdate().getTime());
        }

        db.update(PEER_TABLE, values, Peer.P2P_ADDRESS + " = ?", new String[] { peer.getP2pAddress() });
        
        Database.notifyPeersChanged(mContext);
    }
}
