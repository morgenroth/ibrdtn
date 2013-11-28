package de.tubs.ibr.dtn.streaming;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.util.UUID;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.ParcelFileDescriptor;
import android.util.Log;
import de.tubs.ibr.dtn.api.Bundle;
import de.tubs.ibr.dtn.api.Bundle.ProcFlags;
import de.tubs.ibr.dtn.api.BundleID;
import de.tubs.ibr.dtn.api.DTNClient;
import de.tubs.ibr.dtn.api.EID;
import de.tubs.ibr.dtn.api.SessionDestroyedException;
import de.tubs.ibr.dtn.api.SingletonEndpoint;

public class DtnStreamTransmitter {
    
    private static final String TAG = "DtnOutputStream";
    
    private BundleID mPrevAckedId = null;
    private BundleID mAckedId = null;
    private BundleID mSentId = null;
    
    private Context mContext = null;
    private DTNClient.Session mSession = null;
    private StreamHeader mHeader = null;
    private EID mDestination = null;
    private long mLifetime = 3600L;
    private byte[] mMeta = null;

    private boolean mHeaderWritten = false;
    private boolean mFinalized = false;
    
    private long mRttBegin = 0L;
    private Float mRttValue = null;
    private Future<?> mFlushSchedule = null;
    
    private DataOutputStream mOutput = null;
    private ScheduledExecutorService mExecutor = Executors.newScheduledThreadPool(2);
    
    private int generateCorrelator() {
        UUID uuid = UUID.randomUUID();
        return Long.valueOf(uuid.getMostSignificantBits()).intValue();
    }
    
    public DtnStreamTransmitter(Context context, DTNClient.Session session) {
        mContext = context;
        mSession = session;
        
        // register to RECEIVE intents
        IntentFilter filter = new IntentFilter(de.tubs.ibr.dtn.Intent.STATUS_REPORT);
        filter.addCategory(context.getPackageName());
        context.registerReceiver(mReceiver, filter);
    }
    
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            BundleID acked = intent.getParcelableExtra("bundleid");
            
            synchronized(DtnStreamTransmitter.this) {
                if (acked.compareTo(mPrevAckedId) > 0) {
                    mPrevAckedId = acked;
                    mAckedId = acked;
                    
                    // get measured RTT
                    mRttValue = Float.valueOf(System.currentTimeMillis() - mRttBegin);
    
                    scheduleFlush();
                }
            }
        }
    };
    
    private Thread mSender = new Thread() {
        @Override
        public void run() {
            // create bundle proto-type
            Bundle bundle = new Bundle();
            
            // set destination
            bundle.setDestination(mDestination);
            
            // set lifetime
            bundle.setLifetime(mLifetime);
            
            // set status report requests for bundle reception
            bundle.set(ProcFlags.REQUEST_REPORT_OF_BUNDLE_DELIVERY, true);
            
            // set destination for status reports
            bundle.setReportto(SingletonEndpoint.ME);
            
            try {
                // compose initial bundle payload
                byte[] dataHeader = composeInitial();
                
                // start time measurement
                mRttBegin = System.currentTimeMillis();
                
                // send initial bundle
                mSentId = mSession.send(bundle, dataHeader);
                
                while (!isInterrupted()) {
                    ParcelFileDescriptor[] fds = null;
                    
                    synchronized(DtnStreamTransmitter.this) {
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
                        DtnStreamTransmitter.this.notifyAll();
                    }
                    
                    // start send process
                    mSession.send(mDestination, mLifetime, fds[0]);
                }
                
                // compose final bundle payload
                byte[] dataFin = composeFin();
                
                // send final bundle
                mSession.send(mDestination, mLifetime, dataFin);
            } catch (IOException e) {
                Log.e(TAG, "error while creating a pipe", e);
            } catch (SessionDestroyedException e) {
                Log.e(TAG, "error while sending a stream", e);
            }
        }
    };
    
    public void setLifetime(long lifetime) {
        mLifetime = lifetime;
    }
    
    public synchronized void connect(EID destination, MediaType media, byte[] meta) {
        mDestination = destination;
        mMeta = meta;
        
        mHeader = new StreamHeader();
        mHeader.type = BundleType.INITIAL;
        mHeader.correlator = generateCorrelator();
        mHeader.media = media;
        mHeader.offset = 0;
        
        // start the sending thread
        mSender.start();
    }

    /**
     * write a frame into the packet stream
     * @param data
     * @throws InterruptedException 
     * @throws IOException 
     */
    public synchronized void write(byte[] data) throws InterruptedException, IOException {
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
        mOutput.flush();
        
        scheduleFlush();
    }
    
    private synchronized void flush() throws IOException {
        if (mOutput != null) mOutput.close();
        mOutput = null;
        notifyAll();
    }
    
    private synchronized void scheduleFlush() {
        // need sent and ack'd bundle ID
        if ((mSentId == null) || (mAckedId == null)) return;
        
        // only schedule once
        if (mFlushSchedule != null) return;
        
        // do not flush if the header has not been written yet
        if (!mHeaderWritten) return;
        
        if (!mSentId.equals(mAckedId)) return;
        
        // clear ackedId to prevent further calls
        mAckedId = null;
        
        // delay
        Float delay = mRttValue * 0.2f;
        
        // schedule flush
        mFlushSchedule = mExecutor.scheduleAtFixedRate(new Runnable() {
            @Override
            public void run() {
                try {
                    flush();
                } catch (IOException e) {
                    Log.e(TAG, "flush failed", e);
                }
            }
        }, delay.intValue(), mRttValue.intValue(), TimeUnit.MILLISECONDS);
    }
    
    /**
     * Close the packet stream
     * (transmits a FIN)
     * @throws IOException 
     * @throws InterruptedException 
     */
    public void close() throws IOException, InterruptedException {
        // unregister status report receiver
        mContext.unregisterReceiver(mReceiver);
        
        if (mFlushSchedule != null)
            mFlushSchedule.cancel(false);
        
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
        
        // close executor
        mExecutor.shutdown();
    }
    
    private byte[] composeInitial() throws IOException {
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
    
    private byte[] composeFin() throws IOException {
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
