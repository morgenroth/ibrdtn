package de.tubs.ibr.dtn.streaming;

import java.io.Closeable;
import java.io.DataInputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.LinkedList;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.Handler.Callback;
import android.os.HandlerThread;
import android.os.Looper;
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

public class StreamEndpoint implements Callback {
    
    private static final String TAG = "DtnStreamEndpoint";
    
    public static final int RECEIVE_BUNDLE = 1;
    public static final int MARK_DELIVERED = 2;
    
    private DTNClient.Session mSession = null;
    private Context mContext = null;
    private HandlerThread mHandlerThread = null;
    private Handler mHandler = null;
    
    private DtnInputStream.PacketListener mPacketListener = null;
    private StreamFilter mFilter = null;
    private DataHandler mPlainBundleHandler = null;
    
    // TODO: clear closed / expired streams
    private HashMap<StreamId, DtnInputStream> mStreams = new HashMap<StreamId, DtnInputStream>();

    public StreamEndpoint(Context context, DTNClient.Session session, DtnInputStream.PacketListener listener, DataHandler handler) {
        mSession = session;
        mPlainBundleHandler = handler;
        mPacketListener = listener;
        mSession.setDataHandler(mDataHandler);
        mContext = context;
        
        mHandlerThread = new HandlerThread("Receiver");
        
        // start own handler
        mHandlerThread.start();
        
        Looper looper = mHandlerThread.getLooper();
        mHandler = new Handler(looper, this);
        
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
        
        // stop handler thread
        mHandlerThread.quit();
    }
    
    public void setFilter(StreamFilter filter) {
        mFilter = filter;
    }
    
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Message msg = mHandler.obtainMessage();
            
            msg.what = RECEIVE_BUNDLE;
            msg.obj = null;
            
            mHandler.sendMessage(msg);
        }
    };
    
    private class PayloadParser extends Thread implements Closeable {
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
        
        public void close() {
            try {
                join();
                mInput.close();
            } catch (InterruptedException e) {
                Log.e(TAG, "failed to join", e);
            } catch (IOException e) {
                Log.e(TAG, "failed to close", e);
            }
        }

        @Override
        public void run() {
            try {
                StreamHeader header = StreamHeader.parse(mInput);
                
                // assign correlator to stream id
                mStreamId.correlator = header.correlator;
                
                DtnInputStream target = null;
                
                synchronized(mStreams) {
                    // get existing input stream
                    target = mStreams.get(mStreamId);
                    
                    if (target == null) {
                        // create new input stream
                        target = new DtnInputStream(mStreamId, mPacketListener);
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
                        target.put(frames);
                        break;
                    }
                    case FIN: {
                        Frame f = new Frame();
                        f.offset = header.offset;
                        target.put(f);
                        break;
                    }
                    case INITIAL: {
                        // read meta data
                        Frame f = Frame.parse(mInput);
                        
                        // push data-type and meta-data
                        target.put(header.media, f.data);
                        break;
                    }
                    default:
                        break;
                }
            } catch (IOException e) {
                Log.e(TAG, "error while parsing", e);
            }
        }
    }
    
    private DataHandler mDataHandler = new DataHandler() {

        private BundleID bundle_id = null;
        private StreamId stream_id = null;
        private PayloadParser parser = null;

        @Override
        public void startBundle(Bundle bundle) {
            if ((mFilter == null) || mFilter.onHandleStream(bundle)) {
                stream_id = new StreamId();
                stream_id.correlator = 0;
                stream_id.source = bundle.getSource();
                
                bundle_id = new BundleID(bundle);
            }
            else if (mPlainBundleHandler != null) {
                stream_id = null;
                mPlainBundleHandler.startBundle(bundle);
            }
        }

        @Override
        public void endBundle() {
            if ((stream_id == null) && (mPlainBundleHandler != null)) {
                mPlainBundleHandler.endBundle();
                return;
            }
            
            // mark as delivered
            Message msg = mHandler.obtainMessage();
            msg.what = MARK_DELIVERED;
            msg.obj = bundle_id;
            mHandler.sendMessage(msg);
            
            bundle_id = null;
        }

        @Override
        public TransferMode startBlock(Block block) {
            if ((stream_id == null) && (mPlainBundleHandler != null)) {
                return mPlainBundleHandler.startBlock(block);
            }
            
            // only parse payload
            if (block.type == 1) {
                parser = new PayloadParser(stream_id, block.length);
                parser.start();
                return TransferMode.FILEDESCRIPTOR;
            }
            return TransferMode.NULL;
        }

        @Override
        public void endBlock() {
            if ((stream_id == null) && (mPlainBundleHandler != null)) {
                mPlainBundleHandler.endBlock();
                return;
            }

            if (parser != null) {
                parser.close();
                parser = null;
            }
        }

        @Override
        public void payload(byte[] data) {
            if ((stream_id == null) && (mPlainBundleHandler != null)) {
                mPlainBundleHandler.payload(data);
            }
        }

        @Override
        public ParcelFileDescriptor fd() {
            if ((stream_id == null) && (mPlainBundleHandler != null)) {
                return mPlainBundleHandler.fd();
            }
            return parser.getSinkFd();
        }

        @Override
        public void progress(long current, long length) {
            if ((stream_id == null) && (mPlainBundleHandler != null)) {
                mPlainBundleHandler.progress(current, length);
            }
        }
    };

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
    }}
