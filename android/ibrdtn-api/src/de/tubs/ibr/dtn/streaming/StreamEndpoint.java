package de.tubs.ibr.dtn.streaming;

import java.io.Closeable;
import java.io.DataInputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.UUID;

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
import de.tubs.ibr.dtn.api.EID;
import de.tubs.ibr.dtn.api.SessionDestroyedException;
import de.tubs.ibr.dtn.api.TransferMode;

public class StreamEndpoint implements Callback {
    
    private static final String TAG = "DtnStreamEndpoint";
    
    public static final int RECEIVE_BUNDLE = 1;
    public static final int MARK_DELIVERED = 2;
    
    private DTNClient.Session mSession = null;
    private Context mContext = null;
    private HandlerThread mHandlerThread = null;
    private Handler mHandler = null;
    
    private DtnInputStream.PacketListener mPacketListener = null;
    
    // TODO: clear closed / expired streams
    private HashMap<StreamId, DtnInputStream> mStreams = new HashMap<StreamId, DtnInputStream>();

    public StreamEndpoint(Context context, DTNClient.Session session, DtnInputStream.PacketListener listener) {
        mSession = session;
        mSession.setDataHandler(mDataHandler);
        mContext = context;
        
        mHandlerThread = new HandlerThread("Receiver");
        Looper looper = mHandlerThread.getLooper();
        mHandler = new Handler(looper, this);
        
        // start own handler
        mHandlerThread.start();
        
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
    
    public DtnOutputStream createStream(EID destination, long lifetime) {
        int correlator = generateCorrelator();
        return new DtnOutputStream(mSession, correlator, destination, lifetime);
    }
    
    private int generateCorrelator() {
        UUID uuid = UUID.randomUUID();
        return Long.valueOf(uuid.getMostSignificantBits()).intValue();
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
        private StreamId mId = null;
        
        public PayloadParser(StreamId id) {
            mId = id;
            
            try {
                ParcelFileDescriptor[] fds = ParcelFileDescriptor.createPipe();
                mInput = new DataInputStream(new ParcelFileDescriptor.AutoCloseInputStream(fds[0]));
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
                mInput.close();
                join();
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
                mId.correlator = header.correlator;
                
                DtnInputStream target = null;
                
                // get existing input stream
                target = mStreams.get(mId);
                
                if (target == null) {
                    // create new input stream
                    target = new DtnInputStream(mId, mPacketListener);
                    mStreams.put(mId, target);
                }
                
                switch (header.type) {
                    case DATA:
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
                            Log.d(TAG, "decoding stopped");
                        }
                        
                        // push frames
                        target.put(frames);
                        break;
                    case FIN:
                        target.close();
                        break;
                    case INITIAL:
                        // read meta data
                        Frame f = Frame.parse(mInput);
                        
                        // push data-type and meta-data
                        target.put(header.media, f.data);
                        break;
                    default:
                        break;
                }
            } catch (IOException e) {
                Log.e(TAG, "error while parsing", e);
            }
        }
    }
    
    private DataHandler mDataHandler = new DataHandler() {

        private StreamId id = null;
        private PayloadParser parser = null;

        @Override
        public void startBundle(Bundle bundle) {
            id = new StreamId();
            id.correlator = 0;
            id.source = bundle.getSource();
        }

        @Override
        public void endBundle() {
            id = null;
        }

        @Override
        public TransferMode startBlock(Block block) {
            // only parse payload
            if (block.type == 1) {
                parser = new PayloadParser(id);
                return TransferMode.FILEDESCRIPTOR;
            }
            return TransferMode.NULL;
        }

        @Override
        public void endBlock() {
            parser.close();
            parser = null;
        }

        @Override
        public void payload(byte[] data) {
            // not used here
        }

        @Override
        public ParcelFileDescriptor fd() {
            return parser.getSinkFd();
        }

        @Override
        public void progress(long current, long length) {
            // not used here
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
