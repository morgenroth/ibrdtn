package de.tubs.ibr.dtn.streaming;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;

import android.os.ParcelFileDescriptor;
import android.util.Log;
import de.tubs.ibr.dtn.api.DTNClient;
import de.tubs.ibr.dtn.api.EID;
import de.tubs.ibr.dtn.api.SessionDestroyedException;

public class DtnOutputStream {
    
    private static final String TAG = "DtnOutputStream";
    
    private DTNClient.Session mSession = null;
    private StreamHeader mHeader = null;
    private EID mDestination = null;
    private long mLifetime = 3600L;
    private byte[] mMeta = null;

    private boolean mHeaderWritten = false;
    private boolean mFinalized = false;
    
    private DataOutputStream mOutput = null;
    
    public DtnOutputStream(DTNClient.Session session, int correlator, EID destination, long lifetime, MediaType media, byte[] meta) {
        mSession = session;
        mDestination = destination;
        mLifetime = lifetime;
        
        mHeader = new StreamHeader();
        mHeader.type = BundleType.INITIAL;
        mHeader.correlator = correlator;
        mHeader.media = media;
        mHeader.offset = 0;
        
        // start the sending thread
        mSender.start();
    }
    
    private Thread mSender = new Thread() {
        @Override
        public void run() {
            try {
                byte[] dataHeader = createHeaderMessage();
                mSession.send(mDestination, mLifetime, dataHeader);
                
                while (!isInterrupted()) {
                    ParcelFileDescriptor[] fds = null;
                    
                    synchronized(DtnOutputStream.this) {
                        // abort processing if finalized
                        if (mFinalized) break;
                        
                        // create a new pipe and an outputstream
                        fds = ParcelFileDescriptor.createPipe();
                        
                        // switch header type to DATA
                        mHeader.type = BundleType.DATA;
                        
                        // we need to write the header once again
                        mHeaderWritten = false;

                        // create an output stream
                        mOutput = new DataOutputStream(new ParcelFileDescriptor.AutoCloseOutputStream(fds[1]));
                        
                        // notify all waiting threads
                        DtnOutputStream.this.notifyAll();
                    }
                    
                    // start send process
                    mSession.send(mDestination, mLifetime, fds[0]);
                }
                
                byte[] dataFin = createFinMessage();
                mSession.send(mDestination, mLifetime, dataFin);
            } catch (IOException e) {
                Log.e(TAG, "error while creating a pipe", e);
            } catch (SessionDestroyedException e) {
                Log.e(TAG, "error while sending a stream", e);
            }
        }
    };

    /**
     * write a frame into the packet stream
     * @param data
     * @throws InterruptedException 
     * @throws IOException 
     */
    public synchronized void put(byte[] data) throws InterruptedException, IOException {
        // wait until a stream is ready
        while (mOutput == null) wait();
        
        // write initial header
        if (!mHeaderWritten) {
            StreamHeader.write(mOutput, mHeader);
            mHeader.offset++;
            mHeaderWritten = true;
        }
        
        // new frame to write
        Frame f = new Frame();
        f.data = data;
        Frame.write(mOutput, f);
    }
    
    public synchronized void flush() throws IOException {
        // do not flush if the header has not been written yet
        if (!mHeaderWritten) return;
        
        if (mOutput != null) mOutput.close();
        mOutput = null;
        notifyAll();
    }
    
    /**
     * Close the packet stream
     * (transmits a FIN)
     * @throws IOException 
     * @throws InterruptedException 
     */
    public void close() throws IOException, InterruptedException {
        synchronized(this) {
            // mark this stream as finalized
            mFinalized = true;
            
            // close the output stream
            if (mOutput != null) mOutput.close();
            
            // notify all waiting threads
            notifyAll();
        }
        
        // wait until the sender is finished
        mSender.join();
    }
    
    public byte[] createHeaderMessage() throws IOException {
        ByteArrayOutputStream output = new ByteArrayOutputStream();
        DataOutputStream stream = new DataOutputStream(output);
        
        StreamHeader header = new StreamHeader();
        header.type = BundleType.INITIAL;
        header.correlator = mHeader.correlator;
        header.media = mHeader.media;
        
        StreamHeader.write(stream, header);
        
        // write meta data
        Frame meta = new Frame();
        meta.data = mMeta;
        
        Frame.write(stream, meta);
        
        return output.toByteArray();
    }
    
    public byte[] createFinMessage() throws IOException {
        ByteArrayOutputStream output = new ByteArrayOutputStream();
        DataOutputStream stream = new DataOutputStream(output);
        
        StreamHeader header = new StreamHeader();
        header.type = BundleType.FIN;
        header.correlator = mHeader.correlator;
        header.offset = mHeader.offset;
        
        StreamHeader.write(stream, header);
        
        return output.toByteArray();
    }
}
