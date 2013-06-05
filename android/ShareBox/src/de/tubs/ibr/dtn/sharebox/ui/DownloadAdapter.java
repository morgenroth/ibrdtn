package de.tubs.ibr.dtn.sharebox.ui;

import android.content.Context;
import android.database.Cursor;
import android.provider.BaseColumns;
import android.support.v4.util.LruCache;
import android.support.v4.widget.CursorAdapter;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import de.tubs.ibr.dtn.sharebox.R;
import de.tubs.ibr.dtn.sharebox.data.Download;

public class DownloadAdapter extends CursorAdapter {

    @SuppressWarnings("unused")
    private final static String TAG = "DownloadAdapter";
    
    private LayoutInflater mInflater = null;
    private Context mContext = null;
    
    public static final String[] PROJECTION = new String[] {
        BaseColumns._ID,
        Download.SOURCE,
        Download.DESTINATION,
        Download.TIMESTAMP,
        Download.LIFETIME,
        Download.LENGTH,
        Download.STATE,
        Download.BUNDLE_ID
    };
    
    // The indexes of the default columns which must be consistent
    // with above PROJECTION.
    static final int COLUMN_DOWNLOAD_ID          = 0;
    static final int COLUMN_DOWNLOAD_SOURCE      = 1;
    static final int COLUMN_DOWNLOAD_DESTINATION = 2;
    static final int COLUMN_DOWNLOAD_TIMESTAMP   = 3;
    static final int COLUMN_DOWNLOAD_LIFETIME    = 4;
    static final int COLUMN_DOWNLOAD_LENGTH      = 5;
    static final int COLUMN_DOWNLOAD_STATE       = 6;
    static final int COLUMN_DOWNLOAD_BUNDLE_ID   = 7;
    
    private static final int CACHE_SIZE     = 50;

    private final DownloadCache mCache;
    private final ColumnsMap mColumnsMap;
    
    public DownloadAdapter(Context context, Cursor c, ColumnsMap cmap)
    {
        super(context, c, FLAG_REGISTER_CONTENT_OBSERVER);
        this.mContext = context;
        this.mInflater = LayoutInflater.from(context);
        
        mCache = new DownloadCache(CACHE_SIZE);
        mColumnsMap = cmap;
    }
    
    public static class ColumnsMap {
        public int mColumnId;
        public int mColumnSource;
        public int mColumnDestination;
        public int mColumnTimestamp;
        public int mColumnLifetime;
        public int mColumnLength;
        public int mColumnState;
        public int mColumnBundleId;

        public ColumnsMap() {
            mColumnSource      = COLUMN_DOWNLOAD_SOURCE;
            mColumnDestination = COLUMN_DOWNLOAD_DESTINATION;
            mColumnTimestamp   = COLUMN_DOWNLOAD_TIMESTAMP;
            mColumnLifetime    = COLUMN_DOWNLOAD_LIFETIME;
            mColumnLength      = COLUMN_DOWNLOAD_LENGTH;
            mColumnState       = COLUMN_DOWNLOAD_STATE;
            mColumnBundleId    = COLUMN_DOWNLOAD_BUNDLE_ID;
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
                mColumnSource = cursor.getColumnIndexOrThrow(Download.SOURCE);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
                mColumnDestination = cursor.getColumnIndexOrThrow(Download.DESTINATION);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
                mColumnTimestamp = cursor.getColumnIndexOrThrow(Download.TIMESTAMP);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
                mColumnLifetime = cursor.getColumnIndexOrThrow(Download.LIFETIME);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
                mColumnLength = cursor.getColumnIndexOrThrow(Download.LENGTH);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
                mColumnState = cursor.getColumnIndexOrThrow(Download.STATE);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }
            
            try {
                mColumnBundleId = cursor.getColumnIndexOrThrow(Download.BUNDLE_ID);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }
        }
    }
    
    @Override
    public void notifyDataSetChanged() {
        super.notifyDataSetChanged();
        mCache.evictAll();
    }

    @Override
    public int getItemViewType(int position) {
        Cursor cursor = (Cursor)getItem(position);
        return getItemViewType(cursor);
    }

    private int getItemViewType(Cursor cursor) {
        int type = cursor.getInt(COLUMN_DOWNLOAD_STATE);
        switch (type) {
            default:
                // is pending
                return 1;
                
            case 1:
                // is completed
                return 2;
                
            case 2:
                // is accepted
                return 2;
        }
    }
    
    @Override
    public int getViewTypeCount() {
        return 3;   // marked and non-marked messages
    }

    @Override
    public void bindView(View view, Context context, Cursor cursor) {
        if (view instanceof DownloadItem) {
            long id = cursor.getLong(COLUMN_DOWNLOAD_ID);

            Download d = getCachedItem(id, cursor);
            if (d != null) {
                DownloadItem itm = (DownloadItem) view;
                int position = cursor.getPosition();
                itm.bind(d, position);
            }
        }
    }

    @Override
    public View newView(Context context, Cursor cursor, ViewGroup parent) {
        View view = null;
        if (getItemViewType(cursor) == 1) {
            view = mInflater.inflate(R.layout.download_item, parent, false);
        } else {
            view = mInflater.inflate(R.layout.download_item, parent, false);
        }
        return view;
    }
    
    private boolean isCursorValid(Cursor cursor) {
        // Check whether the cursor is valid or not.
        if (cursor == null || cursor.isClosed() || cursor.isBeforeFirst() || cursor.isAfterLast()) {
            return false;
        }
        return true;
    }
    
    public Download getCachedItem(long id, Cursor c) {
        Download item = mCache.get(id);
        if (item == null && c != null && isCursorValid(c)) {
            item = new Download(mContext, c, mColumnsMap);
            mCache.put(id, item);
        }
        return item;
    }
    
    private static class DownloadCache extends LruCache<Long, Download> {
        public DownloadCache(int maxSize) {
            super(maxSize);
        }

        @Override
        protected void entryRemoved(boolean evicted, Long key,
                Download oldValue, Download newValue) {
        }
    }
}
