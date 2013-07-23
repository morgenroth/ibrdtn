package de.tubs.ibr.dtn.p2p.db;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

public class DatabaseOpenHelper extends SQLiteOpenHelper {

    private static final int DATABASE_VERSION = 1;
    private static final String DATABASE_NAME = "p2p.db";
    private static final String TAG = "DatabaseOpenHelper";

    public static final String PEER_TABLE = "peers";

    public static final String ID_COLUMN = "_id";
    public static final String MAC_ADDRESS_COLUMN = "mac";
    public static final String EID_COLUMN = "eid";
    public static final String LAST_CONTACT_COLUMN = "last_contact";
    public static final String EVER_CONNECTED_COLUMN = "ever_connected";

    public static final String[] ALL_PEER_TABLE_COLUMNS = { ID_COLUMN,
            MAC_ADDRESS_COLUMN, EID_COLUMN, LAST_CONTACT_COLUMN,
            EVER_CONNECTED_COLUMN };

    public static final String CREATE_PEER_TABLE = String
            .format("CREATE TABLE %s (%s INTEGER PRIMARY KEY AUTOINCREMENT, %s TEXT, %s TEXT, %s DATETIME, %s INTEGER)",
                    PEER_TABLE, ID_COLUMN, MAC_ADDRESS_COLUMN, EID_COLUMN,
                    LAST_CONTACT_COLUMN, EVER_CONNECTED_COLUMN);

    public DatabaseOpenHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
//        if (Log.isLoggable(TAG, Log.DEBUG)) {
            Log.d(TAG, CREATE_PEER_TABLE);
//        }
        db.execSQL(CREATE_PEER_TABLE);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
//        if (Log.isLoggable(TAG, Log.DEBUG)) {
            Log.d(TAG, "DROP TABLE IF EXISTS " + PEER_TABLE);
//        }
        db.execSQL("DROP TABLE IF EXISTS " + PEER_TABLE);

        onCreate(db);
    }

}
