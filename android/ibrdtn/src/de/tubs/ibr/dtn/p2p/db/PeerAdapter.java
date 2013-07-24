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
        Peer.P2P_ADDRESS,
        Peer.ENDPOINT,
        Peer.LAST_SEEN,
        Peer.CONNECT_STATE,
        Peer.CONNECT_UPDATE
    };
    
    // The indexes of the default columns which must be consistent
    // with above PROJECTION.
    static final int COLUMN_PEER_ID             = 0;
    static final int COLUMN_PEER_P2P_ADDRESS    = 1;
    static final int COLUMN_PEER_ENDPOINT       = 2;
    static final int COLUMN_PEER_LAST_SEEN      = 3;
    static final int COLUMN_PEER_CONNECT_STATE  = 4;
    static final int COLUMN_PEER_CONNECT_UPDATE = 5;
    
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
        public int mColumnP2pAddress;
        public int mColumnEndpoint;
        public int mColumnLastSeen;
        public int mColumnConnectState;
        public int mColumnConnectUpdate;

        public ColumnsMap() {
            mColumnId               = COLUMN_PEER_ID;
            mColumnP2pAddress       = COLUMN_PEER_P2P_ADDRESS;
            mColumnEndpoint         = COLUMN_PEER_ENDPOINT;
            mColumnLastSeen         = COLUMN_PEER_LAST_SEEN;
            mColumnConnectState     = COLUMN_PEER_CONNECT_STATE;
            mColumnConnectUpdate    = COLUMN_PEER_CONNECT_UPDATE;
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
                mColumnP2pAddress = cursor.getColumnIndexOrThrow(Peer.P2P_ADDRESS);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
                mColumnEndpoint = cursor.getColumnIndexOrThrow(Peer.ENDPOINT);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
                mColumnLastSeen = cursor.getColumnIndexOrThrow(Peer.LAST_SEEN);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
                mColumnConnectState = cursor.getColumnIndexOrThrow(Peer.CONNECT_STATE);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }
            
            try {
                mColumnConnectUpdate = cursor.getColumnIndexOrThrow(Peer.CONNECT_UPDATE);
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
