package de.tubs.ibr.dtn.p2p.db;

import static de.tubs.ibr.dtn.p2p.db.DatabaseOpenHelper.*;

import java.util.*;

import android.content.*;
import android.database.*;
import android.database.sqlite.*;
import android.util.*;

public class Database {

    private static final String TAG = "Database";

    private static Object syncObject = new Object();

    private static Database _instance;

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

    private SQLiteDatabase db;

    private DatabaseOpenHelper helper;

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
    public synchronized void insertPeer(Peer peer) {
        ContentValues values = new ContentValues();
        values.put(MAC_ADDRESS_COLUMN, peer.getMacAddress());
        values.put(EID_COLUMN, peer.getEid());
        values.put(LAST_CONTACT_COLUMN, peer.getLastContact().getTime());
        values.put(EVER_CONNECTED_COLUMN, peer.isEverConnected() ? 1 : 0);

        String selection = MAC_ADDRESS_COLUMN + " = ?";
        String[] selectionArgs = new String[] { peer.getMacAddress() };
        Cursor c = db.query(PEER_TABLE, ALL_PEER_TABLE_COLUMNS, selection,
                selectionArgs, null, null, null);
        if (c.getCount() == 0) {
            long ret = db.insert(PEER_TABLE, null, values);
//            if (Log.isLoggable(TAG, Log.DEBUG)) {
                Log.d(TAG, "Insert:" + peer.toString() + ":" + ret);
//            }
        } else {
            long ret = db.update(PEER_TABLE, values, selection, selectionArgs);
//            if (Log.isLoggable(TAG, Log.DEBUG)) {
                Log.d(TAG, "Update:" + peer.toString() + ":" + ret);
//            }
        }
        c.close();
    }

    public Peer selectPeer(String mac) {
        Peer p = null;
        String selection = String.format("%s = ?", MAC_ADDRESS_COLUMN);
        String[] selectionArgs = { mac };
        Cursor cursor = db.query(PEER_TABLE, ALL_PEER_TABLE_COLUMNS, selection,
                selectionArgs, null, null, null);
//        if (Log.isLoggable(TAG, Log.DEBUG)) {
            Log.d(TAG, "Select peer " + mac + ",count:" + cursor.getCount());
//        }
        if (cursor.getCount() == 1 && cursor.moveToFirst()) {
            p = cursorToPeer(cursor);
        }
        cursor.close();
        return p;
    }

    public Peer[] selectPeers() {
        Peer[] peers;
        Cursor cursor = db.query(PEER_TABLE, ALL_PEER_TABLE_COLUMNS, null,
                null, null, null, null);
//        if (Log.isLoggable(TAG, Log.DEBUG)) {
            Log.d(TAG, "Select peers:" + cursor.getCount());
//        }
        peers = new Peer[cursor.getCount()];
        int i = 0;
        while (cursor.moveToNext()) {
            peers[i++] = cursorToPeer(cursor);
        }
        cursor.close();
        return peers;
    }

    public Cursor selectPeersCursor() {
        Cursor c = db.query(PEER_TABLE, ALL_PEER_TABLE_COLUMNS, null, null,
                null, null, null);
//        if (Log.isLoggable(TAG, Log.DEBUG)) {
            Log.d(TAG, "Select peers(cursor):" + c.getCount());
//        }
        return c;
    }

    private synchronized Peer cursorToPeer(Cursor cursor) {
        String mac;
        String eid;
        Date lastContact;
        boolean everConnected;

        mac = cursor.getString(cursor.getColumnIndex(MAC_ADDRESS_COLUMN));
        eid = cursor.getString(cursor.getColumnIndex(EID_COLUMN));
        lastContact = new Date(cursor.getLong(cursor
                .getColumnIndex(LAST_CONTACT_COLUMN)));
        everConnected = cursor.getInt(cursor
                .getColumnIndex(EVER_CONNECTED_COLUMN)) == 1;

        return new Peer(mac, eid, lastContact, everConnected);
    }

}
