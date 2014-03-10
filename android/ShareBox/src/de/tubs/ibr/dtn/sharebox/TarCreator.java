package de.tubs.ibr.dtn.sharebox;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Date;
import java.util.List;

import org.apache.commons.compress.archivers.ArchiveException;
import org.apache.commons.compress.archivers.ArchiveStreamFactory;
import org.apache.commons.compress.archivers.tar.TarArchiveEntry;
import org.apache.commons.compress.archivers.tar.TarArchiveOutputStream;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.MediaStore;
import android.provider.OpenableColumns;
import android.util.Log;
import android.webkit.MimeTypeMap;

public class TarCreator implements Runnable {
	
	private static final String TAG = "TarCreator";
	
	private OutputStream mOutput = null;
	private List<Uri> mUris = null;
	private OnStateChangeListener mListener = null;
	private Context mContext = null;
	
    public interface OnStateChangeListener {
        void onFileProgress(TarCreator creator, String currentFile, int currentFileNum, int maxFiles);
        void onCopyProgress(TarCreator creator, long current, long max);
        void onStateChanged(TarCreator creator, int state, Long bytes);
    }

	public TarCreator(Context context, OutputStream output, List<Uri> uris) {
	    mContext = context;
		mOutput = output;
		mUris = uris;
	}

	@Override
	public void run() {
		try {
			// open a new tar output stream
			final TarArchiveOutputStream taos = (TarArchiveOutputStream) new ArchiveStreamFactory().createArchiveOutputStream("tar", mOutput);
			
			int currentFile = 0;
			long bytes = 0;
			
			try {
				for (Uri uri : mUris) {
				    currentFile++;
				    
					if (ContentResolver.SCHEME_FILE.equals(uri.getScheme())) {
					    File f = new File(uri.getPath());
					    
					    if (f.exists()) {
					        // update notification
	    	                if (mListener != null) mListener.onFileProgress(this, f.getName(), currentFile, mUris.size());
	    				    
	    	                // add file to tar archive
	    				    add(taos, f);
					    }
					}
					else if (ContentResolver.SCHEME_CONTENT.equals(uri.getScheme())) {
				        ContentResolver resolver = mContext.getContentResolver();
				        
				        // create a cursor
				        Cursor cursor = resolver.query(uri, null, null, null, null);
				        
				        if (cursor != null) {
				        	try {
					        	String filename = uri.getLastPathSegment();
					        	
					        	// standard columns
					        	int columnIndexDisplayName = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
					            int columnIndexSize = cursor.getColumnIndex(OpenableColumns.SIZE);
					            
					            // media columns
					        	int columnIndexTitle = cursor.getColumnIndex(MediaStore.MediaColumns.TITLE);
					            int columnIndexData = cursor.getColumnIndex(MediaStore.MediaColumns.DATA);
					            
					            if (cursor.moveToFirst()) {
				                    MimeTypeMap mime = MimeTypeMap.getSingleton();
				                    String displayName = (columnIndexDisplayName == -1) ? null : cursor.getString(columnIndexDisplayName);
				                    String data = (columnIndexData == -1) ? null : cursor.getString(columnIndexData);
				                    String title = (columnIndexTitle == -1) ? null : cursor.getString(columnIndexTitle);
					                Long filesize = (columnIndexSize == -1) ? null : cursor.getLong(columnIndexSize);
					                
					                Log.d(TAG, "Uri: " + uri.toString() + ", Name: " + displayName + ", Data: " + data + ", Title: " + title + ", Size: " + filesize);
					                
					                if (displayName != null) {
					                	// use displayName as filename if available
					                	filename = displayName;
					                } else if (data != null) {
						                // use data as filename if available
					                	filename = (new File(data)).getName();
					                } else if (title != null) {
						                // use title as filename if available
					                	filename = title;
					                }
					                
					                // check if the file has an extension
					                int type_delim = filename.indexOf('.');
					                if ((type_delim <= 1) || (type_delim >= filename.length())) {
					                	String type = mime.getExtensionFromMimeType(resolver.getType(uri));
					                	
					                	// attach a type extension if there is none
					                	filename += "." + type;
					                }
					                
					                // update notification
				                    if (mListener != null) mListener.onFileProgress(this, filename, currentFile, mUris.size());
		
				                    // add uri to tar archive
				                    add(taos, uri, filename, filesize);
				                    
				                    bytes += filesize;
					            }
				        	} finally {
					            // close the cursor
					            cursor.close();
				        	}
				        }
					} else {
						// type not supported - skip the file
					}
				}
	        } catch (IOException e) {
	            Log.e(TAG, null, e);
	            throw e;
			} finally {
				// close the tar output stream
				taos.close();
			}
			
			if (mListener != null) mListener.onStateChanged(this, 1, bytes);
        } catch (ArchiveException e) {
            Log.e(TAG, null, e);
            if (mListener != null) mListener.onStateChanged(this, -1, null);
        } catch (IOException e) {
            Log.e(TAG, null, e);
            if (mListener != null) mListener.onStateChanged(this, -1, null);
        } catch (IllegalStateException e) {
            Log.e(TAG, null, e);
            if (mListener != null) mListener.onStateChanged(this, -1, null);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, null, e);
            if (mListener != null) mListener.onStateChanged(this, -1, null);
        }
	}
	
	private int copy(InputStream is, OutputStream os, long length)
			throws IOException {
		if (mListener != null)
			mListener.onCopyProgress(this, 0, length);

		byte[] buffer = new byte[8196];
		int count = 0;
		int n = 0;
		while (-1 != (n = is.read(buffer))) {
			os.write(buffer, 0, n);
			count += n;

			if (mListener != null)
				mListener.onCopyProgress(this, count, length);
		}
		return count;
	}
	
	private void add(TarArchiveOutputStream taos, Uri uri, String filename, long length) throws IOException {
        // create a new entry
        TarArchiveEntry entry = new TarArchiveEntry(filename);
        
        entry.setModTime(new Date());
        entry.setSize(length);
        entry.setUserId(0);
        entry.setGroupId(0);
        entry.setUserName("root");
        entry.setGroupName("root");
        entry.setMode(TarArchiveEntry.DEFAULT_FILE_MODE);

        // add entry to the archive
        taos.putArchiveEntry(entry);
        
        // open the content as stream
        final InputStream is = mContext.getContentResolver().openInputStream(uri);
        
        try {
	        // copy the payload into the archive
	        copy(is, taos, length);
        } finally {
            // close the source file
            is.close();
        }
        
        // finalize the archive entry
        taos.closeArchiveEntry();
	}
	
	private void add(TarArchiveOutputStream taos, File f) throws IOException {
        // create a new entry
        TarArchiveEntry entry = new TarArchiveEntry(f.getName());
        
        entry.setModTime(new Date());
        entry.setSize(f.length());
        entry.setUserId(0);
        entry.setGroupId(0);
        entry.setUserName("root");
        entry.setGroupName("root");
        entry.setMode(TarArchiveEntry.DEFAULT_FILE_MODE);

        // add entry to the archive
        taos.putArchiveEntry(entry);
        
        // open the content as stream
        final InputStream is = new FileInputStream(f);
        
        try {
	        // copy the payload into the archive
	        copy(is, taos, f.length());
        } finally {
	        // close the source file
	        is.close();
        }
        
        // finalize the archive entry
        taos.closeArchiveEntry();
	}
	
    public OnStateChangeListener getOnStateChangeListener() {
        return mListener;
    }

    public void setOnStateChangeListener(OnStateChangeListener listener) {
        mListener = listener;
    }
}
