package de.tubs.ibr.dtn.dtalkie;

import java.util.List;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.graphics.drawable.LayerDrawable;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.AudioManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.preference.PreferenceManager;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.widget.ImageButton;
import de.tubs.ibr.dtn.dtalkie.service.RecorderService;
import de.tubs.ibr.dtn.dtalkie.service.Utils;

public class RecordingFragment extends Fragment {

    @SuppressWarnings("unused")
    private static final String TAG = "RecordingFragment";
    
    private RecorderService mService = null;
    private Boolean mBound = false;
    
    private ImageButton mRecordButton = null;
    
    private Boolean mUpdateAmplitudeSwitch = false;
    private int mMaxAmplitude = 0;
    private int mIdleCounter = 0;
    
    private Handler mHandler = null;
    
    private Boolean mRecording = false;
    
    private SensorEventListener mSensorListener = new SensorEventListener() {
        public void onSensorChanged(SensorEvent event) {
            if (event.values.length > 0) {
                float current = event.values[0];
                float maxRange = event.sensor.getMaximumRange();
                boolean far = (current == maxRange);
                
                SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
                if (prefs.getBoolean("sensor", false)) 
                {
                    if (far) {
                        stopRecording();
                    } else {
                        startRecording();
                    }
                }
            }
        }

        public void onAccuracyChanged(Sensor sensor, int accuracy) {
        }
    };
    
    private OnTouchListener mTouchListener = new OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            switch ( event.getAction() ) {
                case MotionEvent.ACTION_DOWN:
                    SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
                    if (prefs.getBoolean("ptt", false)) {
                        startRecording();
                    }
                    break;
                    
                case MotionEvent.ACTION_UP:
                    toggleRecording();
                    break;
            }
            
            return true;
        }
    };
    
    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, IBinder service) {
            mService = ((RecorderService.LocalBinder)service).getService();
        }

        public void onServiceDisconnected(ComponentName name) {
            mService = null;
        }
    };
    
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.recording_fragment, container, false);
        
        mRecordButton = (ImageButton) v.findViewById(R.id.button_record);
        mRecordButton.setOnTouchListener(mTouchListener);
        
        return v;
    }
    
    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        
        // set volume control to MUSIC
        getActivity().setVolumeControlStream(AudioManager.STREAM_MUSIC);
    }
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mBound = false;
        mHandler = new Handler();
    }

    @Override
    public void onDestroy() {
        if (mBound) {
            getActivity().unbindService(mConnection);
            mBound = false;
        }
        
        mHandler = null;
        super.onDestroy();
    }

    @Override
    public void onPause() {
        SensorManager sm = (SensorManager) getActivity().getSystemService(Context.SENSOR_SERVICE);
        sm.unregisterListener(mSensorListener);

        // we are going out of scope - stop recording
        stopRecording();
        
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
        
        if (!mBound) {
            getActivity().bindService(new Intent(getActivity(), RecorderService.class), mConnection, Context.BIND_AUTO_CREATE);
            mBound = true;
        }
        
        SensorManager sm = (SensorManager) getActivity().getSystemService(Context.SENSOR_SERVICE);
        List<Sensor> sensors = sm.getSensorList(Sensor.TYPE_PROXIMITY);
        if (sensors.size() > 0) {
            Sensor sensor = sensors.get(0);
            sm.registerListener(mSensorListener, sensor, SensorManager.SENSOR_DELAY_NORMAL);
        }
    }
    
    private void startRecording() {
        if (mRecording) return;
        mRecording = true;
        
        // lock screen orientation
        Utils.lockScreenOrientation(getActivity());
        
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
        if (!prefs.getBoolean("ptt", false)) {
            // start amplitude update
            mUpdateAmplitudeSwitch = true;
            mHandler.postDelayed(mUpdateAmplitude, 100);
        } else {
            // set indicator level to zero
            setIndicator(1.0f);
        }
        
        if (mService != null) {
            mService.startRecording(RecorderService.TALKIE_GROUP_EID);
        }
    }
    
    private void stopRecording() {
        if (!mRecording) return;
        mRecording = false;
        
        // stop amplitude update
        mUpdateAmplitudeSwitch = false;
        mHandler.removeCallbacks(mUpdateAmplitude);
        
        // set indicator level to zero
        setIndicator(0.0f);
        
        // unlock screen orientation
        Utils.unlockScreenOrientation(getActivity());
        
        if (mService != null) {
            mService.stopRecording();
        }
        
        mMaxAmplitude = 0;
        mIdleCounter = 0;
    }
    
    private void toggleRecording() {
        if (mRecording) {
            stopRecording();
        } else {
            startRecording();
        }
    }
    
    @SuppressWarnings("deprecation")
    private void setIndicator(Float level) {
        LayerDrawable d = (LayerDrawable)getResources().getDrawable(R.drawable.ptt_background);
        
        int height = this.getView().getMeasuredHeight();
        
        d.setLayerInset(1, 0, 0, 0, Float.valueOf(height * (level * 1.2f)).intValue());
        mRecordButton.setBackgroundDrawable(d);
    }
    
    private Runnable mUpdateAmplitude = new Runnable() {
        @Override
        public void run() {
            if (mUpdateAmplitudeSwitch) {
                if (mService != null) {
                    int curamp = mService.getMaxAmplitude();
                    if (mMaxAmplitude < curamp) mMaxAmplitude = curamp;
                    
                    float level = 0.0f;
                            
                    if (mMaxAmplitude > 0) {
                        level = Float.valueOf(curamp) / mMaxAmplitude;
                    }
                    
                    if (level < 0.4f) {
                        mIdleCounter++;
                    } else {
                        mIdleCounter = 0;
                    }
                    
                    setIndicator(level);
                    
                    // if there is no sound for 2 seconds
                    if (mIdleCounter >= 20) {
                        stopRecording();
                    }
                }
                mHandler.postDelayed(mUpdateAmplitude, 100);
            }
        }
    };
}
