package de.tubs.ibr.dtn.streaming;

import java.util.Calendar;
import java.util.Collection;
import java.util.Date;
import java.util.LinkedList;
import java.util.PriorityQueue;
import java.util.Queue;

public class StreamBuffer {

    private int mNext = 0;
    private Queue<Frame> mFramePool = new LinkedList<Frame>();
    private Queue<Frame> mDataQueue = new PriorityQueue<Frame>();
    private DtnStreamReceiver.StreamListener mListener = null;
    private MediaType mType = null;
    private boolean mFinalized = false;
    private StreamId mId = null;
    
    private Date mExpiration = null;
    
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
    
    public synchronized Frame obtainFrame() {
        Frame f = mFramePool.poll();
        if (f == null) {
            f = new Frame();
        }
        return f;
    }
    
    private synchronized void releaseFrame(Frame f) {
        f.clear();
        mFramePool.add(f);
    }
    
    /**
     * put data frame into the stream
     * @param frames
     */
    public synchronized void pushFrame(Collection<Frame> frames) {
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
    
    public synchronized void pushFrame(Frame f) {
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
                close();
                return;
            } else {
                // deliver this frame
                if (mListener != null) mListener.onFrameReceived(mId, head);
            }
            
            // remind the expected next frame
            mNext = head.offset + 1;
            
            // release the frame back to the pool
            releaseFrame(head);
            
            // peek at the next frame
            head = mDataQueue.peek();
        }
    }
    
    /**
     * finalize the stream
     */
    private synchronized void close() {
        if (mListener != null) mListener.onFinish(mId);
        mFinalized = true;
        mDataQueue.clear();
    }
    
    public void prolong(int lifetime) {
        Calendar c = Calendar.getInstance();
        c.add(Calendar.SECOND, lifetime);
        mExpiration = c.getTime();
    }
    
    public boolean isFinalized() {
        return mFinalized;
    }
    
    public boolean isGarbage() {
        if (isFinalized()) return true;
        
        // without an expiration date, this will never be garbage
        if (mExpiration == null) return false;
        
        if (mExpiration.before(new Date())) {
            close();
            return true;
        }

        return false;
    }
}
