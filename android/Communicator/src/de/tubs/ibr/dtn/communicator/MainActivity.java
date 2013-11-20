
package de.tubs.ibr.dtn.communicator;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ToggleButton;

public class MainActivity extends Activity {
    
    private static final String TAG = "MainActivity";
    private ToggleButton mToggle = null;
    
    private ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG, "service connected");
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "service disconnected");
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        mToggle = (ToggleButton)findViewById(R.id.toggleButton);
        mToggle.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mToggle.isChecked()) {
                    Log.d(TAG, "start recording");
                    Intent i = new Intent(MainActivity.this, CommService.class);
                    i.setAction(CommService.OPEN_COMM_CHANNEL);
                    startService(i);
                } else {
                    Log.d(TAG, "stop recording");
                    Intent i = new Intent(MainActivity.this, CommService.class);
                    i.setAction(CommService.CLOSE_COMM_CHANNEL);
                    startService(i);
                }
            }
        });
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    protected void onStart() {
        super.onStart();
        bindService(new Intent(this, CommService.class), mServiceConnection, Context.BIND_AUTO_CREATE);
    }

    @Override
    protected void onStop() {
        unbindService(mServiceConnection);
        super.onStop();
    }
}
