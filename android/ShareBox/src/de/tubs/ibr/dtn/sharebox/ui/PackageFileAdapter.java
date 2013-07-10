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
import de.tubs.ibr.dtn.sharebox.data.PackageFile;

public class PackageFileAdapter extends CursorAdapter {

    @SuppressWarnings("unused")
    private final static String TAG = "PackageFileAdapter";
    
    private LayoutInflater mInflater = null;
    private Context mContext = null;
    
    public static final String[] PROJECTION = new String[] {
        BaseColumns._ID,
        PackageFile.DOWNLOAD,
        PackageFile.FILENAME,
        PackageFile.LENGTH
    };
    
    // The indexes of the default columns which must be consistent
    // with above PROJECTION.
    static final int COLUMN_FILE_ID          = 0;
    static final int COLUMN_FILE_DOWNLOAD    = 1;
    static final int COLUMN_FILE_FILENAME    = 2;
    static final int COLUMN_FILE_LENGTH      = 3;
    
    private static final int CACHE_SIZE     = 50;

    private final PackageFileCache mCache;
    private final ColumnsMap mColumnsMap;
    
    public PackageFileAdapter(Context context, Cursor c, ColumnsMap cmap)
    {
        super(context, c, FLAG_REGISTER_CONTENT_OBSERVER);
        this.mContext = context;
        this.mInflater = LayoutInflater.from(context);
        
        mCache = new PackageFileCache(CACHE_SIZE);
        mColumnsMap = cmap;
    }
    
    public static class ColumnsMap {
        public int mColumnId;
        public int mColumnDownload;
        public int mColumnFilename;
        public int mColumnLength;

        public ColumnsMap() {
            mColumnDownload    = COLUMN_FILE_DOWNLOAD;
            mColumnFilename    = COLUMN_FILE_FILENAME;
            mColumnLength      = COLUMN_FILE_LENGTH;
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
                mColumnDownload = cursor.getColumnIndexOrThrow(PackageFile.DOWNLOAD);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
                mColumnFilename = cursor.getColumnIndexOrThrow(PackageFile.FILENAME);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
                mColumnLength = cursor.getColumnIndexOrThrow(PackageFile.LENGTH);
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
    public void bindView(View view, Context context, Cursor cursor) {
        if (view instanceof PackageFileItem) {
            long id = cursor.getLong(COLUMN_FILE_ID);

            PackageFile f = getCachedItem(id, cursor);
            if (f != null) {
                PackageFileItem itm = (PackageFileItem) view;
                int position = cursor.getPosition();
                itm.bind(f, position);
            }
        }
    }

    @Override
    public View newView(Context context, Cursor cursor, ViewGroup parent) {
        return mInflater.inflate(R.layout.packagefile_list_item, parent, false);
    }
    
    private boolean isCursorValid(Cursor cursor) {
        // Check whether the cursor is valid or not.
        if (cursor == null || cursor.isClosed() || cursor.isBeforeFirst() || cursor.isAfterLast()) {
            return false;
        }
        return true;
    }
    
    public PackageFile getCachedItem(long id, Cursor c) {
        PackageFile item = mCache.get(id);
        if (item == null && c != null && isCursorValid(c)) {
            item = new PackageFile(mContext, c, mColumnsMap);
            mCache.put(id, item);
        }
        return item;
    }
    
    private static class PackageFileCache extends LruCache<Long, PackageFile> {
        public PackageFileCache(int maxSize) {
            super(maxSize);
        }

        @Override
        protected void entryRemoved(boolean evicted, Long key,
                PackageFile oldValue, PackageFile newValue) {
        }
    }
}
