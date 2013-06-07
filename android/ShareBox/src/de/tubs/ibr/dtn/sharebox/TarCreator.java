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
import org.apache.commons.compress.utils.IOUtils;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.MediaStore;
import android.util.Log;
import android.webkit.MimeTypeMap;

public class TarCreator implements Runnable {
	
	private static final String TAG = "TarCreator";
	
	private OutputStream mOutput = null;
	private List<Uri> mUris = null;
	private OnStateChangeListener mListener = null;
	private Context mContext = null;
	
    public interface OnStateChangeListener {
        void onProgress(TarCreator creator, String currentFile, int currentFileNum, int maxFiles);
        void onStateChanged(TarCreator creator, int state);
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
			
			for (Uri uri : mUris) {
			    currentFile++;
			    
				if (uri.getScheme() == "file") {
				    File f = new File(uri.getPath());
				    
				    if (f.exists()) {
				        // update notification
    	                if (mListener != null) mListener.onProgress(this, f.getName(), currentFile, mUris.size());
    				    
    	                // add file to tar archive
    				    add(taos, f);
				    }
				} else {
			        ContentResolver resolver = mContext.getContentResolver();
			        
			        // create projection for meta data
			        String[] proj = { MediaStore.Images.Media.TITLE, MediaStore.Images.Media.SIZE };
			        
			        // create a cursor
			        Cursor cursor = resolver.query(uri, proj, null, null, null);
			        
			        if (cursor != null) {
			            int columnIndexTitle = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.TITLE);
			            int columnIndexSize = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.SIZE);
			            
			            if (cursor.moveToFirst()) {
		                    MimeTypeMap mime = MimeTypeMap.getSingleton();
		                    String type = mime.getExtensionFromMimeType(resolver.getType(uri));

			                String filename = cursor.getString(columnIndexTitle) + "." + type;
			                Long filesize = cursor.getLong(columnIndexSize);
			                
			                // update notification
		                    if (mListener != null) mListener.onProgress(this, filename, currentFile, mUris.size());

		                    // add uri to tar archive
		                    add(taos, uri, filename, filesize);
			            }
			        }
				}
			}
			
			// close the tar output stream
			taos.close();
			
			if (mListener != null) mListener.onStateChanged(this, 1);
        } catch (ArchiveException e) {
            Log.e(TAG, null, e);
            if (mListener != null) mListener.onStateChanged(this, -1);
        } catch (IOException e) {
            Log.e(TAG, null, e);
            if (mListener != null) mListener.onStateChanged(this, -1);
        } catch (IllegalStateException e) {
            Log.e(TAG, null, e);
            if (mListener != null) mListener.onStateChanged(this, -1);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, null, e);
            if (mListener != null) mListener.onStateChanged(this, -1);
        }
	}
	
	private void add(TarArchiveOutputStream taos, Uri uri, String filename, long length) throws IOException {
        // open the content as stream
        final InputStream is = mContext.getContentResolver().openInputStream(uri);
        
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
        
        // copy the payload into the archive
        IOUtils.copy(is, taos);
        
        // close the source file
        is.close();
        
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
        
        // copy the payload into the archive
        IOUtils.copy(is, taos);
        
        // close the source file
        is.close();
        
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
