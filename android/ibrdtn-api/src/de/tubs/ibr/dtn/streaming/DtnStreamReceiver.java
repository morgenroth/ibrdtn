package de.tubs.ibr.dtn.streaming;

import java.io.DataInputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;

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
    
    private StreamListener mListener = null;
    private StreamFilter mFilter = null;
    private DataHandler mPlainBundleHandler = null;
    
    // TODO: clear closed / expired streams
    private HashMap<StreamId, StreamBuffer> mStreams = new HashMap<StreamId, StreamBuffer>();
    
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
        
        // stop executor service
        mProcessor.shutdown();
        
        // stop handlers
        mHandlerThread.quit();
    }
    
    public void setFilter(StreamFilter filter) {
        mFilter = filter;
    }
    
    private Handler.Callback mReceiverCallback = new Handler.Callback() {
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
    
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Message msg = mHandler.obtainMessage();
            
            msg.what = RECEIVE_BUNDLE;
            msg.obj = null;
            
            mHandler.sendMessage(msg);
        }
    };
    
    private class PayloadParser implements Runnable {
        private ParcelFileDescriptor mSinkFd = null;
        private DataInputStream mInput = null;
        private StreamId mStreamId = null;
        
        public PayloadParser(StreamId id, long payload_length) {
            mStreamId = id;
            
            try {
                ParcelFileDescriptor[] fds = ParcelFileDescriptor.createPipe();
                mInput = new DataInputStream(new TruncatedInputStream(new ParcelFileDescriptor.AutoCloseInputStream(fds[0]), payload_length));
                mSinkFd = fds[1];
            } catch (IOException e) {
                Log.e(TAG, "failed to create pipe", e);
            }
        }
        
        public ParcelFileDescriptor getSinkFd() {
            return mSinkFd;
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
                }
                
                switch (header.type) {
                    case DATA: {
                        // read all frames
                        LinkedList<Frame> frames = new LinkedList<Frame>();
    
                        try {
                            int index = 0;
                            while (true) {
                                Frame f = Frame.parse(mInput);
                                f.number = index;
                                f.offset = header.offset;
                                frames.add(f);
                                index++;
                            }
                        } catch (IOException e) {
                            //Log.d(TAG, "decoding stopped");
                        }
                        
                        // push frames
                        target.push(frames);
                        break;
                    }
                    case FIN: {
                        Frame f = new Frame();
                        f.offset = header.offset;
                        target.push(f);
                        break;
                    }
                    case INITIAL: {
                        // read meta data
                        Frame f = Frame.parse(mInput);
                        
                        // push data-type and meta-data
                        target.initialize(header.media, f.data);
                        break;
                    }
                    default:
                        break;
                }
            } catch (IOException e) {
                Log.e(TAG, "error while parsing", e);
            } finally {
                try {
                    mInput.close();
                } catch (IOException e) {
                    Log.e(TAG, "failed to close", e);
                }
            }
        }
    }
    
    private DataHandler mDataHandler = new DataHandler() {

        private BundleID mBundleId = null;
        private StreamId mStreamId = null;
        private Block mBlock = null;

        @Override
        public void startBundle(Bundle bundle) {
            if ((mFilter == null) || mFilter.onHandleStream(bundle)) {
                mStreamId = new StreamId();
                mStreamId.correlator = 0;
                mStreamId.source = bundle.getSource();
                
                mBundleId = new BundleID(bundle);
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
            msg.obj = mBundleId;
            mHandler.sendMessage(msg);
            
            mBundleId = null;
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
                PayloadParser parser = new PayloadParser(mStreamId, mBlock.length);
                mProcessor.submit(parser);
                return parser.getSinkFd();
            } else {
                return null;
            }
        }

        @Override
        public void progress(long current, long length) {
            if ((mStreamId == null) && (mPlainBundleHandler != null)) {
                mPlainBundleHandler.progress(current, length);
            }
        }
    };
}
