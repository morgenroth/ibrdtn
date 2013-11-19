package de.tubs.ibr.dtn.streaming;

import java.io.Closeable;
import java.util.Collection;
import java.util.PriorityQueue;
import java.util.Queue;

public class DtnInputStream implements Closeable {

    private int mNext = 0;
    private Queue<Frame> mDataQueue = new PriorityQueue<Frame>();
    private PacketListener mListener = null;
    private MediaType mType = null;
    private boolean mFinalized = false;
    private StreamId mId = null;
    
    public interface PacketListener {
        public void onInitial(StreamId id, MediaType type, byte[] data);
        public void onFrameReceived(StreamId id, Frame frame);
        public void onFinish(StreamId id);
    }
    
    public DtnInputStream(StreamId id, PacketListener listener) {
        mListener = listener;
        mId = id;
    }
    
    public StreamId getStreamId() {
        return mId;
    }
    
    // put initial data into the stream
    public synchronized void put(MediaType type, byte[] data) {
        mType = type;
        
        if (mListener != null) mListener.onInitial(mId, mType, data);
        
        deliverFrames();
    }
    
    // put data frame into the stream
    public synchronized void put(Collection<Frame> frames) {
        // drop all frames if finalized
        if (mFinalized) return;
        
        // insert all frames into the queue
        // (re-ordering)
        for (Frame f : frames) {
            // only insert expected frames
            if (f.offset >= mNext) {
                mDataQueue.offer(f);
            }
        }
        
        // no delivery until the initial is received
        if (mType == null) return;
        
        deliverFrames();
    }
    
    private void deliverFrames() {
        // check head for delivery
        Frame head = mDataQueue.peek();
        
        // deliver all pending frames in the right order
        while ((head != null) && (head.offset <= mNext)) {
            // retrieve the head of the queue
            head = mDataQueue.poll();
            
            // deliver this frame
            if (mListener != null) mListener.onFrameReceived(mId, head);
            
            // remind the expected next frame
            mNext = head.offset + 1;
            
            // peek at the next frame
            head = mDataQueue.peek();
        }
    }
    
    // finalize the stream
    public synchronized void close() {
        mFinalized = true;
        mDataQueue.clear();
        
        if (mListener != null) mListener.onFinish(mId);
    }
}
