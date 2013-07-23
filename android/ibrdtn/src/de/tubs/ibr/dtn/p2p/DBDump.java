package de.tubs.ibr.dtn.p2p;

import android.annotation.*;
import android.app.*;
import android.database.*;
import android.os.*;
import android.widget.*;
import de.tubs.ibr.dtn.*;
import de.tubs.ibr.dtn.p2p.db.*;

@TargetApi(16)
public class DBDump extends Activity {

    private SimpleCursorAdapter dataAdapter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.p2p_dbdump_activity);

        Database db = Database.getInstance(this.getApplicationContext());
        db.open();

        Cursor cursor = db.selectPeersCursor();

        // String[] columns = DatabaseOpenHelper.ALL_PEER_TABLE_COLUMNS;
        String[] columns = new String[] {
                DatabaseOpenHelper.MAC_ADDRESS_COLUMN,
                DatabaseOpenHelper.EID_COLUMN,
                DatabaseOpenHelper.LAST_CONTACT_COLUMN,
                DatabaseOpenHelper.EVER_CONNECTED_COLUMN };

        int[] to = new int[] { R.id.mac, R.id.eid, R.id.contact, R.id.connected };

        dataAdapter = new SimpleCursorAdapter(this, R.layout.p2p_dbdumpline,
                cursor, columns, to, 0);

        ListView listView = (ListView) findViewById(R.id.listView1);
        listView.setAdapter(dataAdapter);
    }

}
