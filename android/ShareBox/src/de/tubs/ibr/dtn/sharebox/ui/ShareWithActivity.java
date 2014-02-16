package de.tubs.ibr.dtn.sharebox.ui;

import java.io.Serializable;
import java.util.ArrayList;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.net.Uri;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import de.tubs.ibr.dtn.api.Node;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.sharebox.DtnService;

public class ShareWithActivity extends FragmentActivity {
    
    private static final String TAG = "ShareWithActivity";
    private static final int SELECT_NEIGHBOR = 1;
    
    private DtnService mService = null;
    private Boolean mBound = false;
    
    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, IBinder service) {
            mService = ((DtnService.LocalBinder)service).getService();
            
            Intent intent = mService.getSelectNeighborIntent();
            startActivityForResult(intent, SELECT_NEIGHBOR);
        }

        public void onServiceDisconnected(ComponentName name) {
            mService = null;
        }
    };
    
    private void onNeighborSelected(Node n) {
        String endpoint = n.endpoint.toString();
        
        if (endpoint.startsWith("ipn:")) {
            endpoint = endpoint + ".4066896964";
        } else {
            endpoint = endpoint + "/sharebox";
        }
        
        Log.d(TAG, "Neighbor selected: " + endpoint);
        SingletonEndpoint destination = new SingletonEndpoint(endpoint);
        
        // intent of the share request
        Intent intent = getIntent();
        
        // close the activity is there is no intent
        if (intent == null) return;
        
        // extract the action
        String action = intent.getAction();
        
        if (Intent.ACTION_SEND.equals(action)) {
            Uri imageUri = (Uri) intent.getParcelableExtra(Intent.EXTRA_STREAM);
            
            Intent dtnSendIntent = new Intent(this, DtnService.class);
            dtnSendIntent.setAction(de.tubs.ibr.dtn.Intent.SENDFILE);
            dtnSendIntent.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_KEY_STREAM, imageUri);
            dtnSendIntent.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_KEY_DESTINATION, (Serializable)destination);
            startService(dtnSendIntent);
            
        } else if (Intent.ACTION_SEND_MULTIPLE.equals(action)) {
            ArrayList<Uri> imageUris = intent.getParcelableArrayListExtra(Intent.EXTRA_STREAM);

            Intent dtnSendIntent = new Intent(this, DtnService.class);
            dtnSendIntent.setAction(de.tubs.ibr.dtn.Intent.SENDFILE_MULTIPLE);
            dtnSendIntent.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_KEY_STREAM, imageUris);
            dtnSendIntent.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_KEY_DESTINATION, (Serializable)destination);
            startService(dtnSendIntent);
        }
    }
    
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (SELECT_NEIGHBOR == requestCode) {
            if ((data != null) && data.hasExtra(de.tubs.ibr.dtn.Intent.NODE_KEY)) {
                Node n = data.getParcelableExtra(de.tubs.ibr.dtn.Intent.NODE_KEY);
                onNeighborSelected(n);
            }
        }
        finish();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mBound = false;
    }
    
    @Override
    public void onDestroy() {
        if (mBound) {
            unbindService(mConnection);
            mBound = false;
        }
        
        super.onDestroy();
    }
    
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
        
        if (!mBound) {
            bindService(new Intent(this, DtnService.class), mConnection, Context.BIND_AUTO_CREATE);
            mBound = true;
        }
    }
    
}
