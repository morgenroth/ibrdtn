package de.tubs.ibr.dtn.streaming;

import java.io.DataInputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.os.ParcelFileDescriptor;
import android.util.Log;
import de.tubs.ibr.dtn.api.Block;
import de.tubs.ibr.dtn.api.Bundle;
import de.tubs.ibr.dtn.api.BundleID;
import de.tubs.ibr.dtn.api.DTNClient;
import de.tubs.ibr.dtn.api.DataHandler;
import de.tubs.ibr.dtn.api.SessionDestroyedException;
import de.tubs.ibr.dtn.api.TransferMode;
import de.tubs.ibr.dtn.util.TruncatedInputStream;

public class DtnStreamReceiver {
    
    private static final String TAG = "DtnStreamEndpoint";
    
    public static final int RECEIVE_BUNDLE = 1;
    public static final int MARK_DELIVERED = 2;
    
    private DTNClient.Session mSession = null;
    private Context mContext = null;
    
    private HandlerThread mHandlerThread = null;
    private Handler mHandler = null;
    
    private ScheduledExecutorService mProcessor = null;
    private Future<?> mGarbageCollector = null;
    
    private StreamListener mListener = null;
    private StreamFilter mFilter = null;
    private DataHandler mPlainBundleHandler = null;
    
    public interface StreamListener {
        public void onInitial(StreamId id, MediaType type, byte[] data);
        public void onFrameReceived(StreamId id, Frame frame);
        public void onFinish(StreamId id);
    };

    public DtnStreamReceiver(Context context, DTNClient.Session session, StreamListener listener, DataHandler handler) {
        mSession = session;
        mPlainBundleHandler = handler;
        mListener = listener;
        mSession.setDataHandler(mDataHandler);
        mContext = context;
        
        // create handler for receiver and data processor
        mHandlerThread = new HandlerThread("Receiver");
        
        // start both handlers
        mHandlerThread.start();
        
        // create new handler
        mHandler = new Handler(mHandlerThread.getLooper(), mReceiverCallback);
        
        // create a new executor service
        mProcessor = Executors.newScheduledThreadPool(2);
        
        // schedule garbage collection
        mGarbageCollector = mProcessor.scheduleWithFixedDelay(new Runnable() {
            @Override
            public void run() {
                mDataHandler.gc();
            }
        }, 1, 1, TimeUnit.MINUTES);
        
        // register to RECEIVE intents
        IntentFilter filter = new IntentFilter(de.tubs.ibr.dtn.Intent.RECEIVE);
        filter.addCategory(context.getPackageName());
        context.registerReceiver(mReceiver, filter);
        
        // query for new bundles once
        Message msg = mHandler.obtainMessage();
        msg.what = RECEIVE_BUNDLE;
        msg.obj = null;
        mHandler.sendMessage(msg);
    }
    
    public void release() {
        // release registration to RECEIVE intents
        mContext.unregisterReceiver(mReceiver);
        
        // abort garbage collection
        mGarbageCollector.cancel(false);
        
        // stop executor service
        mProcessor.shutdown();
        
        // stop handlers
        mHandlerThread.quit();
    }
    
    public void setFilter(StreamFilter filter) {
        mFilter = filter;
    }
    
    private final Handler.Callback mReceiverCallback = new Handler.Callback() {
        @Override
        public boolean handleMessage(Message msg) {
            if (msg.what == RECEIVE_BUNDLE) {
                try {
                    BundleID id = (BundleID)msg.obj;
                    if (id == null) {
                        while (mSession.queryNext());
                    } else {
                        while (mSession.query(id));
                    }
                } catch (SessionDestroyedException e) {
                    e.printStackTrace();
                }
                return true;
            }
            else if (msg.what == MARK_DELIVERED) {
                try {
                    BundleID id = (BundleID)msg.obj;
                    mSession.delivered(id);
                } catch (SessionDestroyedException e) {
                    e.printStackTrace();
                }
                return true;
            }
            return false;
        }
    };
    
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Message msg = mHandler.obtainMessage();
            
            msg.what = RECEIVE_BUNDLE;
            msg.obj = null;
            
            mHandler.sendMessage(msg);
        }
    };
    
    private final StreamDataHandler mDataHandler = new StreamDataHandler();
    
    private class StreamDataHandler implements DataHandler, Runnable {
        
        private HashMap<StreamId, StreamBuffer> mStreams = new HashMap<StreamId, StreamBuffer>();

        private Bundle mBundle = null;
        private StreamId mStreamId = null;
        private Block mBlock = null;
         
        private DataInputStream mInput = null;

        @Override
        public void startBundle(Bundle bundle) {
            if ((mFilter == null) || mFilter.onHandleStream(bundle)) {
                mStreamId = new StreamId();
                mStreamId.correlator = 0;
                mStreamId.source = bundle.getSource();
                
                mBundle = bundle;
            }
            else if (mPlainBundleHandler != null) {
                mStreamId = null;
                mPlainBundleHandler.startBundle(bundle);
            }
        }

        @Override
        public void endBundle() {
            if ((mStreamId == null) && (mPlainBundleHandler != null)) {
                mPlainBundleHandler.endBundle();
                return;
            }
            
            // mark as delivered
            Message msg = mHandler.obtainMessage();
            msg.what = MARK_DELIVERED;
            msg.obj = new BundleID(mBundle);
            mHandler.sendMessage(msg);
        }

        @Override
        public TransferMode startBlock(Block block) {
            if ((mStreamId == null) && (mPlainBundleHandler != null)) {
                return mPlainBundleHandler.startBlock(block);
            }
            
            // only parse payload
            if (block.type == 1) {
                mBlock = block;
                return TransferMode.FILEDESCRIPTOR;
            }
            return TransferMode.NULL;
        }

        @Override
        public void endBlock() {
            if ((mStreamId == null) && (mPlainBundleHandler != null)) {
                mPlainBundleHandler.endBlock();
                return;
            }
            
            try {
                // need to wait here until run is done
                synchronized(this) {
                    while (mInput != null) {
                        this.wait();
                    }
                }
            } catch (InterruptedException e) {
                Log.e(TAG, "interrupted while waiting for completion", e);
            }
            
            mBlock = null;
        }

        @Override
        public void payload(byte[] data) {
            if ((mStreamId == null) && (mPlainBundleHandler != null)) {
                mPlainBundleHandler.payload(data);
            }
        }

        @Override
        public ParcelFileDescriptor fd() {
            if ((mStreamId == null) && (mPlainBundleHandler != null)) {
                return mPlainBundleHandler.fd();
            }
            
            if (mBlock != null) {
                try {
                    ParcelFileDescriptor[] fds = ParcelFileDescriptor.createPipe();
                    mInput = new DataInputStream(new TruncatedInputStream(new ParcelFileDescriptor.AutoCloseInputStream(fds[0]), mBlock.length));
                    mProcessor.submit(this);
                    return fds[1];
                } catch (IOException e) {
                    Log.e(TAG, "failed to create pipe", e);
                }
            }
            
            return null;
        }

        @Override
        public void progress(long current, long length) {
            if ((mStreamId == null) && (mPlainBundleHandler != null)) {
                mPlainBundleHandler.progress(current, length);
            }
        }
        
        @Override
        public void run() {
            try {
                StreamHeader header = StreamHeader.parse(mInput);
                
                // assign correlator to stream id
                mStreamId.correlator = header.correlator;
                
                StreamBuffer target = null;
                
                synchronized(mStreams) {
                    // get existing input stream
                    target = mStreams.get(mStreamId);
                    
                    if (target == null) {
                        // create new input stream
                        target = new StreamBuffer(mStreamId, mListener);
                        mStreams.put(mStreamId, target);
                    }
                    
                    // prolong stream validity
                    target.prolong(mBundle.getLifetime().intValue());
                }
                
                switch (header.type) {
                    case DATA: {
                        // read all frames
                        LinkedList<Frame> frames = new LinkedList<Frame>();
    
                        try {
                            int index = 0;
                            while (true) {
                                Frame f = target.obtainFrame();
                                f.parse(mInput);
                                f.number = index;
                                f.offset = header.offset;
                                frames.add(f);
                                index++;
                            }
                        } catch (IOException e) {
                            //Log.d(TAG, "decoding stopped");
                        }
                        
                        // push frames
                        target.pushFrame(frames);
                        break;
                    }
                    case FIN: {
                        Frame f = target.obtainFrame();
                        f.offset = header.offset;
                        target.pushFrame(f);
                        break;
                    }
                    case INITIAL: {
                        // read meta data
                        Frame f = target.obtainFrame();
                        f.parse(mInput);
                        
                        // push data-type and meta-data
                        target.initialize(header.media, f.data);
                        break;
                    }
                    default:
                        break;
                }
                
                if (target.isFinalized()) {
                    synchronized(mStreams) {
                        //Log.d(TAG, "stream released: " + mStreamId);
                        mStreams.remove(mStreamId);
                    }
                }
            } catch (IOException e) {
                Log.e(TAG, "error while parsing", e);
            } finally {
                close();
            }
        }
        
        private synchronized void close() {
            try {
                mInput.close();
            } catch (IOException e) {
                Log.e(TAG, "failed to close", e);
            }
            mInput = null;
            notifyAll();
        }
        
        // garbage collection: clear closed / expired streams
        public void gc() {
            synchronized(mStreams) {
                for (StreamId id : mStreams.keySet()) {
                    StreamBuffer buf = mStreams.get(id);
                    if (buf.isGarbage()) {
                        mStreams.remove(id);
                        //Log.d(TAG, "stream wiped: " + id);
                    }
                }
            }
        }
    };
}
