package de.tubs.ibr.dtn.communicator;

import java.io.ByteArrayOutputStream;
import java.io.Closeable;
import java.io.DataOutputStream;
import java.io.IOException;

import android.content.Context;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaRecorder;
import android.util.Log;

import com.purplefrog.speexjni.FrequencyBand;
import com.purplefrog.speexjni.SpeexEncoder;

import de.tubs.ibr.dtn.api.DTNClient.Session;
import de.tubs.ibr.dtn.api.EID;
import de.tubs.ibr.dtn.streaming.DtnStreamTransmitter;
import de.tubs.ibr.dtn.streaming.MediaType;

public class SpeexTransmitter extends Thread implements Closeable {
    
    private static final String TAG = "SpeexTransmitter";
    
    /**
     * Audio Parameters
     */
    private static final int FREQUENCY = 44100;
    private static final FrequencyBand BAND = FrequencyBand.WIDE_BAND;
    private static final int QUALITY = 8;
    
    private AudioRecord mAudioRec = null;
    private DtnStreamTransmitter mStream = null;
    private EID mDestination = null;
    
    private StateListener mListener = null;
    
    private short mMaxLevel = 0;
    private int mSilenceCounter = 0;
    private int mSkipFrames = 5;
    private int mSkipCounter = 100;
    
    public interface StateListener {
        void onAir();
        void onStopped();
    }
    
    public SpeexTransmitter(Context context, Session session, EID destination, StateListener listener) {
        mStream = new DtnStreamTransmitter(context, session);
        mStream.setLifetime(300);
        mDestination = destination;
        mListener = listener;
    }
    
    @Override
    public void run() {
        // create an encoder
        SpeexEncoder encoder = new SpeexEncoder(BAND, QUALITY);
        
        // get the minimum buffer size
        int buffer_size = 32 * AudioTrack.getMinBufferSize(FREQUENCY, AudioFormat.CHANNEL_OUT_MONO, AudioFormat.ENCODING_PCM_16BIT);
        
        // create an AudioSource
        mAudioRec = new AudioRecord(MediaRecorder.AudioSource.MIC, FREQUENCY, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT, buffer_size);
        
        if (mAudioRec.getState() == AudioRecord.STATE_INITIALIZED) {
            // create meta data header
            ByteArrayOutputStream bytearray_stream = new ByteArrayOutputStream();
            DataOutputStream meta_stream = new DataOutputStream(bytearray_stream);
            
            try {
                meta_stream.writeChar(1); // version
                meta_stream.writeInt(FREQUENCY);
                meta_stream.writeChar(BAND.code);
            } catch (IOException e) {
                Log.e(TAG, "can not create meta data", e);
                return;
            }
            
            // start streaming
            mStream.connect(mDestination, MediaType.MEDIA_AUDIO, bytearray_stream.toByteArray());
    
            // initiate recording
            mAudioRec.startRecording();
        
            // callback for state
            if (mListener != null) mListener.onAir();
            
            int frameSize = encoder.getFrameSize();
            short[] buf = new short[frameSize];
            
            try {
                while (AudioRecord.RECORDSTATE_STOPPED != mAudioRec.getRecordingState()) {
                    mAudioRec.read(buf, 0, frameSize);
                    
                    // check if still speaking
                    if (!isSpeaking(buf)) break;
                    
                    if (mSkipFrames > 0) {
                        mSkipFrames--;
                    } else {
                        byte[] data = encoder.encode(buf);
                        mStream.write(data);
                    }
                }
                
                mStream.close();
            } catch (IOException e) {
                Log.e(TAG, "error while transmitting", e);
            } catch (InterruptedException e) {
                Log.e(TAG, "interrupted while transmitting", e);
            }
        }
        
        // release recording resources
        mAudioRec.release();
        mAudioRec = null;
        
        // callback for state
        if (mListener != null) mListener.onStopped();
    }
    
    @Override
    public void close() {
        if (mAudioRec != null) mAudioRec.stop();
    }
    
    private boolean isSpeaking(short[] buf) {
        short maxLevel = 0;
        
        // ignore the initial seconds completely
        if (mSkipCounter > 0) {
            mSkipCounter--;
            return true;
        }
        
        for (int i = 0; i < buf.length; i++) {
            if (maxLevel < buf[i]) {
                maxLevel = buf[i];
            }
        }
        
        if (mMaxLevel < maxLevel) {
            mMaxLevel = maxLevel;
        }
        
        Float thresh = Float.valueOf(mMaxLevel) * 0.2f;

        // count the number of max
        if (maxLevel > thresh.shortValue()) {
            mSilenceCounter = 0;
        } else {
            mSilenceCounter++;
        }
        
        // user is speaking if there is no gap longer than 2 seconds
        return (mSilenceCounter < 100);
    }
}
