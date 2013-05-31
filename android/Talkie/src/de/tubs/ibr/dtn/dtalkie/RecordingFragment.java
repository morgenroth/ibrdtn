package de.tubs.ibr.dtn.dtalkie;

import java.util.List;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.AudioManager;
import android.os.Bundle;
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
                    startRecording();
                    break;
                    
                case MotionEvent.ACTION_UP:
                    stopRecording();
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
    }

    @Override
    public void onDestroy() {
        if (mBound) {
            getActivity().unbindService(mConnection);
            mBound = false;
        }
        
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
        // lock screen orientation
        Utils.lockScreenOrientation(getActivity());
        
        mRecordButton.setPressed(true);
        
        if (mService != null) {
            mService.startRecording(RecorderService.TALKIE_GROUP_EID);
        }
    }
    
    private void stopRecording() {
        // unlock screen orientation
        Utils.unlockScreenOrientation(getActivity());
        
        mRecordButton.setPressed(false);
        
        if (mService != null) {
            mService.stopRecording();
        }
    }
}
