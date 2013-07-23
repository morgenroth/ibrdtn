package de.tubs.ibr.dtn.p2p.db;

import android.content.Context;
import android.database.Cursor;
import android.provider.BaseColumns;
import android.support.v4.util.LruCache;
import android.support.v4.widget.CursorAdapter;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.p2p.PeerItem;

public class PeerAdapter extends CursorAdapter {
    
    @SuppressWarnings("unused")
    private final static String TAG = "PeerAdapter";
    
    private LayoutInflater mInflater = null;

    public static final String[] PROJECTION = new String[] {
        BaseColumns._ID,
        Peer.MAC_ADDRESS,
        Peer.EID,
        Peer.LAST_CONTACT,
        Peer.EVER_CONNECTED
    };
    
    // The indexes of the default columns which must be consistent
    // with above PROJECTION.
    static final int COLUMN_PEER_ID             = 0;
    static final int COLUMN_PEER_MAC_ADDRESS    = 1;
    static final int COLUMN_PEER_EID            = 2;
    static final int COLUMN_PEER_LAST_CONTACT   = 3;
    static final int COLUMN_PEER_EVER_CONNECTED = 4;
    
    private static final int CACHE_SIZE     = 50;

    private final PeerCache mPeerCache;
    private final ColumnsMap mColumnsMap;
    
    public PeerAdapter(Context context, Cursor c, ColumnsMap cmap)
    {
        super(context, c, FLAG_REGISTER_CONTENT_OBSERVER);
        this.mInflater = LayoutInflater.from(context);
        
        mPeerCache = new PeerCache(CACHE_SIZE);
        mColumnsMap = cmap;
    }
    
    public static class ColumnsMap {
        public int mColumnId;
        public int mColumnMacAddress;
        public int mColumnEid;
        public int mColumnLastContact;
        public int mColumnEverConnected;

        public ColumnsMap() {
            mColumnId               = COLUMN_PEER_ID;
            mColumnMacAddress       = COLUMN_PEER_MAC_ADDRESS;
            mColumnEid              = COLUMN_PEER_EID;
            mColumnLastContact      = COLUMN_PEER_LAST_CONTACT;
            mColumnEverConnected    = COLUMN_PEER_EVER_CONNECTED;
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
                mColumnMacAddress = cursor.getColumnIndexOrThrow(Peer.MAC_ADDRESS);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
                mColumnEid = cursor.getColumnIndexOrThrow(Peer.EID);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
                mColumnLastContact = cursor.getColumnIndexOrThrow(Peer.LAST_CONTACT);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
                mColumnEverConnected = cursor.getColumnIndexOrThrow(Peer.EVER_CONNECTED);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }
        }
    }
    
    @Override
    public void notifyDataSetChanged() {
        super.notifyDataSetChanged();
        mPeerCache.evictAll();
    }

    @Override
    public void bindView(View view, Context context, Cursor cursor) {
        if (view instanceof PeerItem) {
            long peerId = cursor.getLong(mColumnsMap.mColumnId);

            Peer p = getCachedPeer(peerId, cursor);
            if (p != null) {
                PeerItem pli = (PeerItem) view;
                int position = cursor.getPosition();
                pli.bind(p, position);
            }
        }
    }

    @Override
    public View newView(Context context, Cursor cursor, ViewGroup parent) {
        return mInflater.inflate(R.layout.p2p_peer_item, parent, false);
    }

    private boolean isCursorValid(Cursor cursor) {
        // Check whether the cursor is valid or not.
        if (cursor == null || cursor.isClosed() || cursor.isBeforeFirst() || cursor.isAfterLast()) {
            return false;
        }
        return true;
    }
    
    public Peer getCachedPeer(long msgId, Cursor c) {
        Peer item = mPeerCache.get(msgId);
        if (item == null && c != null && isCursorValid(c)) {
            item = new Peer(c, mColumnsMap);
            mPeerCache.put(msgId, item);
        }
        return item;
    }
    
    private static class PeerCache extends LruCache<Long, Peer> {
        public PeerCache(int maxSize) {
            super(maxSize);
        }

        @Override
        protected void entryRemoved(boolean evicted, Long key,
                Peer oldValue, Peer newValue) {
            //oldValue.cancelPduLoading();
        }
    }
}
