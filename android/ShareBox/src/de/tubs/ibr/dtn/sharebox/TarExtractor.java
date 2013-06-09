package de.tubs.ibr.dtn.sharebox;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.LinkedList;

import org.apache.commons.compress.archivers.ArchiveException;
import org.apache.commons.compress.archivers.ArchiveStreamFactory;
import org.apache.commons.compress.archivers.tar.TarArchiveEntry;
import org.apache.commons.compress.archivers.tar.TarArchiveInputStream;
import org.apache.commons.compress.utils.IOUtils;

import android.util.Log;

public class TarExtractor implements Runnable {
    
    private static final String TAG = "TarExtractor";
    
    private InputStream mInput = null;
    private File mTargetDir = null;
    private LinkedList<File> mExtractedFiles = null;
    private OnStateChangeListener mListener = null;
    
    public interface OnStateChangeListener {
        void onStateChanged(TarExtractor extractor, int state);
    }
    
    public TarExtractor(InputStream is, File targetDir) {
        mInput = is;
        mTargetDir = targetDir;
    }

    @Override
    public void run() {
        mExtractedFiles = new LinkedList<File>();
        
        if (mListener != null) mListener.onStateChanged(this, 0);
        
        try {
            final TarArchiveInputStream tais = (TarArchiveInputStream) new ArchiveStreamFactory().createArchiveInputStream("tar", mInput);
            
            TarArchiveEntry entry = null;
            
            while ((entry = (TarArchiveEntry)tais.getNextEntry()) != null) {
                File outputFile = new File(mTargetDir, entry.getName());
                if (entry.isDirectory()) {
                    if (!outputFile.exists()) {
                        if (!outputFile.mkdirs()) {
                            throw new IllegalStateException(String.format("Couldn't create directory %s.", outputFile.getAbsolutePath()));
                        }
                    }
                } else {
                    // check if the full path exists
                    File parent = outputFile.getParentFile();
                    if (!parent.exists()) {
                        if (!parent.mkdirs()) {
                            throw new IllegalStateException(String.format("Couldn't create directory %s.", parent.getAbsolutePath()));
                        }
                    }
                    
                    // rename if the file already exists
                    if (outputFile.exists()) {
                        String full_path = outputFile.getAbsolutePath();
                        int ext_delimiter = full_path.lastIndexOf('.');
                        int i = 1;
                        
                        File newfile = new File(full_path.substring(0, ext_delimiter - 1) + "_" + String.valueOf(i) + full_path.substring(ext_delimiter));
                        
                        while (newfile.exists()) {
                            i++;
                            newfile = new File(full_path.substring(0, ext_delimiter - 1) + "_" + String.valueOf(i) + full_path.substring(ext_delimiter));
                        }
                        
                        outputFile = newfile;
                    }
                    
                    final OutputStream ofs = new FileOutputStream(outputFile.getAbsolutePath());
                    IOUtils.copy(tais, ofs);
                    ofs.close();
                }
                mExtractedFiles.add(outputFile);
            }
            
            tais.close();
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
    
    public LinkedList<File> getFiles() {
        return mExtractedFiles;
    }

    public OnStateChangeListener getOnStateChangeListener() {
        return mListener;
    }

    public void setOnStateChangeListener(OnStateChangeListener listener) {
        mListener = listener;
    }
}
