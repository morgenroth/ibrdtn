package de.tubs.ibr.dtn.streaming;

import java.util.Collection;
import java.util.PriorityQueue;
import java.util.Queue;

public class StreamBuffer {

    private int mNext = 0;
    private Queue<Frame> mDataQueue = new PriorityQueue<Frame>();
    private DtnStreamReceiver.StreamListener mListener = null;
    private MediaType mType = null;
    private boolean mFinalized = false;
    private StreamId mId = null;
    
    public StreamBuffer(StreamId id, DtnStreamReceiver.StreamListener listener) {
        mListener = listener;
        mId = id;
    }
    
    public StreamId getStreamId() {
        return mId;
    }
    
    /**
     * put initial data into the stream
     * @param type
     * @param data
     */
    public synchronized void initialize(MediaType type, byte[] data) {
        // only initialize once
        if (mType != null) return;
        
        mType = type;
        
        if (mListener != null) mListener.onInitial(mId, mType, data);
        
        deliverFrames();
    }
    
    /**
     * put data frame into the stream
     * @param frames
     */
    public synchronized void push(Collection<Frame> frames) {
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
        
        deliverFrames();
    }
    
    public synchronized void push(Frame f) {
        // drop all frames if finalized
        if (mFinalized) return;
        
        // insert expected frames only into the queue
        // (re-ordering)
        if (f.offset >= mNext) {
            mDataQueue.offer(f);
        }
        
        deliverFrames();
    }
    
    private void deliverFrames() {
        // no delivery until the initial is received
        if (mType == null) return;
        
        // check head for delivery
        Frame head = mDataQueue.peek();
        
        // deliver all pending frames in the right order
        while ((head != null) && (head.offset <= mNext)) {
            // retrieve the head of the queue
            head = mDataQueue.poll();
            
            if (head.data == null) {
                // final frame
                if (mListener != null) mListener.onFinish(mId);
                close();
                return;
            } else {
                // deliver this frame
                if (mListener != null) mListener.onFrameReceived(mId, head);
            }
            
            // remind the expected next frame
            mNext = head.offset + 1;
            
            // peek at the next frame
            head = mDataQueue.peek();
        }
    }
    
    /**
     * finalize the stream
     */
    private synchronized void close() {
        mFinalized = true;
        mDataQueue.clear();
    }
}
