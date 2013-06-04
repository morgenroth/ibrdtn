package de.tubs.ibr.dtn.sharebox.data;

import android.content.Context;
import android.database.Cursor;
import android.provider.BaseColumns;
import de.tubs.ibr.dtn.sharebox.ui.PackageFileAdapter;

public class PackageFile {
    
    @SuppressWarnings("unused")
    private final static String TAG = "PackageFile";
    
    public static final String ID = BaseColumns._ID;
    public static final String DOWNLOAD = "download";
    public static final String FILENAME = "filename";
    public static final String LENGTH = "length";
    
    private Long mId = null;
    private Long mDownload = null;
    private String mFilename = null;
    private Long mLength = null;
    
    public PackageFile(Context context, Cursor cursor, PackageFileAdapter.ColumnsMap cmap) {
        mId = cursor.getLong(cmap.mColumnId);
    }
    
    public PackageFile(String filename) {
        mFilename = filename;
    }
    
    public Long getId() {
        return mId;
    }

    public Long getDownload() {
        return mDownload;
    }

    public void setDownload(Long download) {
        mDownload = download;
    }

    public String getFilename() {
        return mFilename;
    }

    public void setFilename(String filename) {
        mFilename = filename;
    }

    public Long getLength() {
        return mLength;
    }

    public void setLength(Long length) {
        mLength = length;
    }
}
