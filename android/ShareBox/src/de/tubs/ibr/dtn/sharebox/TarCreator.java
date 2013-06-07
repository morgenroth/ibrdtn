package de.tubs.ibr.dtn.sharebox;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.List;

import org.apache.commons.compress.archivers.ArchiveEntry;
import org.apache.commons.compress.archivers.ArchiveException;
import org.apache.commons.compress.archivers.ArchiveStreamFactory;
import org.apache.commons.compress.archivers.tar.TarArchiveOutputStream;
import org.apache.commons.compress.utils.IOUtils;

import android.util.Log;

public class TarCreator implements Runnable {
	
	private static final String TAG = "TarCreator";
	
	private OutputStream mOutput = null;
	private List<File> mFiles = null;
	private OnStateChangeListener mListener = null;
	
    public interface OnStateChangeListener {
        void onProgress(TarCreator creator, File currentFile, int currentFileNum, int maxFiles);
        void onStateChanged(TarCreator creator, int state);
    }

	public TarCreator(OutputStream output, List<File> files) {
		mOutput = output;
		mFiles = files;
	}

	@Override
	public void run() {
		try {
			// open a new tar output stream
			final TarArchiveOutputStream taos = (TarArchiveOutputStream) new ArchiveStreamFactory().createArchiveOutputStream("tar", mOutput);
			
			int currentFile = 0;
			
			for (File f : mFiles) {
			    currentFile++;
			    if (mListener != null) mListener.onProgress(this, f, currentFile, mFiles.size());
			    
				// create a new entry
				ArchiveEntry entry = taos.createArchiveEntry(f, f.getName());
				
				// add entry to the archive
				taos.putArchiveEntry(entry);
				
				// open the source file
                final InputStream ifs = new FileInputStream(f.getAbsolutePath());
                
                // copy the payload into the archive
                IOUtils.copy(ifs, taos);
                
                // close the source file
                ifs.close();
                
                // finalize the archive entry
                taos.closeArchiveEntry();
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
	
    public OnStateChangeListener getOnStateChangeListener() {
        return mListener;
    }

    public void setOnStateChangeListener(OnStateChangeListener listener) {
        mListener = listener;
    }
}
