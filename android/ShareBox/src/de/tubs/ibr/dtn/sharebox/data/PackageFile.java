package de.tubs.ibr.dtn.sharebox.data;

import java.io.File;

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
    private File mFile = null;
    private Long mLength = null;
    
    public PackageFile(Context context, Cursor cursor, PackageFileAdapter.ColumnsMap cmap) {
        mId = cursor.getLong(cmap.mColumnId);
        mDownload = cursor.getLong(cmap.mColumnDownload);
        mFile = new File(cursor.getString(cmap.mColumnFilename));
        mLength = cursor.getLong(cmap.mColumnLength);

    }
    
    public PackageFile(File file) {
        mFile = file;
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

    public File getFile() {
        return mFile;
    }

    public void setFile(File file) {
        mFile = file;
    }

    public Long getLength() {
        return mLength;
    }

    public void setLength(Long length) {
        mLength = length;
    }
}
