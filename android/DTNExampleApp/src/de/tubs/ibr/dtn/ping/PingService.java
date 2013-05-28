package de.tubs.ibr.dtn.ping;

import android.app.IntentService;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import android.util.Log;
import de.tubs.ibr.dtn.api.Block;
import de.tubs.ibr.dtn.api.Bundle;
import de.tubs.ibr.dtn.api.Bundle.ProcFlags;
import de.tubs.ibr.dtn.api.BundleID;
import de.tubs.ibr.dtn.api.DTNClient;
import de.tubs.ibr.dtn.api.DTNClient.Session;
import de.tubs.ibr.dtn.api.DataHandler;
import de.tubs.ibr.dtn.api.GroupEndpoint;
import de.tubs.ibr.dtn.api.Registration;
import de.tubs.ibr.dtn.api.ServiceNotAvailableException;
import de.tubs.ibr.dtn.api.SessionDestroyedException;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.api.TransferMode;

public class PingService extends IntentService {
    
    // This TAG is used to identify this class (e.g. for debugging)
    private static final String TAG = "PingService";
    
    // mark a specific bundle as delivered
    public static final String MARK_DELIVERED_INTENT = "de.tubs.ibr.dtn.example.MARK_DELIVERED";
    
    // process a status report
    public static final String REPORT_DELIVERED_INTENT = "de.tubs.ibr.dtn.example.REPORT_DELIVERED";
    
    // this intent send out a PING message
    public static final String PING_INTENT = "de.tubs.ibr.dtn.example.PING";

    // indicates updated data to other components
    public static final String DATA_UPDATED = "de.tubs.ibr.dtn.example.DATA_UPDATED";
    
    // group EID of this app
    public static final GroupEndpoint PING_GROUP_EID = new GroupEndpoint("dtn://broadcast.dtn/ping");
    
    // This is the object that receives interactions from clients.  See
    // RemoteService for a more complete example.
    private final IBinder mBinder = new LocalBinder();
    
    // The communication with the DTN service is done using the DTNClient
    private DTNClient mClient = null;
    
    // Hold the last ping result
    private Double mLastMeasurement = 0.0;
    
    // Hold the start time of the last ping
    private Long mStart = 0L;
    
    public PingService() {
        super(TAG);
    }
    
    /**
     * Class for clients to access.  Because we know this service always
     * runs in the same process as its clients, we don't need to deal with
     * IPC.
     */
    public class LocalBinder extends Binder {
        public PingService getService() {
            return PingService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }
    
    public Double getLastMeasurement() {
        return mLastMeasurement;
    }
    
    public String getLocalEndpoint() {
        try {
            if (mClient != null) {
                return mClient.getDTNService().getEndpoint();
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        
        return null;
    }
    
    private void doPing(SingletonEndpoint destination) {
        // create a new bundle
        Bundle b = new Bundle();
        
        // set the destination of the bundle
        b.setDestination(destination);
        
        // limit the lifetime of the bundle to 60 seconds
        b.setLifetime(60L);
        
        // set status report requests for bundle reception
        b.set(ProcFlags.REQUEST_REPORT_OF_BUNDLE_RECEPTION, true);
        
        // set destination for status reports
        b.setReportto(SingletonEndpoint.ME);
        
        // generate some payload
        String payload = "Hello World";

        try {
            // get the DTN session
            Session s = mClient.getSession();
            
            // store the current time
            mStart = System.nanoTime();
            
            // send the bundle
            BundleID ret = s.send(b, payload.getBytes());
            
            if (ret == null)
            {
                Log.e(TAG, "could not send the message");
            }
            else
            {
                Log.d(TAG, "Bundle sent, BundleID: " + ret.toString());
            }
        } catch (SessionDestroyedException e) {
            Log.e(TAG, "could not send the message", e);
        } catch (InterruptedException e) {
            Log.e(TAG, "could not send the message", e);
        }
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        String action = intent.getAction();
        
        if (de.tubs.ibr.dtn.Intent.RECEIVE.equals(action))
        {
            // Received bundles from the DTN service here
            try {
                // We loop here until no more bundles are available
                // (queryNext() returns false)
                while (mClient.getSession().queryNext());
            } catch (SessionDestroyedException e) {
                Log.e(TAG, "Can not query for bundle", e);
            } catch (InterruptedException e) {
                Log.e(TAG, "Can not query for bundle", e);
            }
        }
        else if (MARK_DELIVERED_INTENT.equals(action))
        {
            // retrieve the bundle ID of the intent
            BundleID bundleid = intent.getParcelableExtra("bundleid");
            
            try {
                // mark the bundle ID as delivered
                mClient.getSession().delivered(bundleid);
            } catch (Exception e) {
                Log.e(TAG, "Can not mark bundle as delivered.", e);
            }
        }
        else if (REPORT_DELIVERED_INTENT.equals(action))
        {
            // retrieve the source of the status report
            SingletonEndpoint source = intent.getParcelableExtra("source");
            
            // retrieve the bundle ID of the intent
            BundleID bundleid = intent.getParcelableExtra("bundleid");
            
            Log.d(TAG, "Status report received for " + bundleid.toString() + " from " + source.toString());
        }
        else if (PING_INTENT.equals(action))
        {
            // retrieve the ping destination
            SingletonEndpoint destination = new SingletonEndpoint(intent.getStringExtra("destination"));
            
            // send out the ping
            doPing(destination);
        }
    }
    
    @Override
    public void onCreate() {
        super.onCreate();
        
        // create a new DTN client
        mClient = new DTNClient();
        
        // create registration with "example-app" as endpoint
        // if the EID of this device is "dtn://device" then the
        // address of this app will be "dtn://device/example-app"
        Registration registration = new Registration("example-app");
        
        // additionally join a group
        registration.add(PING_GROUP_EID);
        
        // register own data handler for incoming bundles
        mClient.setDataHandler(mDataHandler);
        
        try {
            // initialize the connection to the DTN service
            mClient.initialize(this, registration);
            Log.d(TAG, "Connection to DTN service established.");
        } catch (ServiceNotAvailableException e) {
            // The DTN service has not been found
            Log.e(TAG, "DTN service unavailable. Is IBR-DTN installed?", e);
        } catch (SecurityException e) {
            // The service has not been found
            Log.e(TAG, "The app has no permission to access the DTN service. It is important to install the DTN service first and then the app.", e);
        }
    }

    @Override
    public void onDestroy() {
        // terminate the DTN service
        mClient.terminate();
        mClient = null;
        
        super.onDestroy();
    }

    /**
     * This data handler is used to process incoming bundles
     */
    private DataHandler mDataHandler = new DataHandler() {

        private Bundle mBundle = null;

        @Override
        public void startBundle(Bundle bundle) {
            // store the bundle header locally
            mBundle = bundle;
        }

        @Override
        public void endBundle() {
            // complete bundle received
            BundleID received = new BundleID(mBundle);
            
            // stop measurement and store result
            long diffTime = System.nanoTime() - mStart;
            mLastMeasurement = Double.valueOf(diffTime) / 1000000.0;
            
            // mark the bundle as delivered
            Intent i = new Intent(PingService.this, PingService.class);
            i.setAction(MARK_DELIVERED_INTENT);
            i.putExtra("bundleid", received);
            startService(i);
            
            // free the bundle header
            mBundle = null;
            
            // notify other components of the updated value
            Intent updatedIntent = new Intent(DATA_UPDATED);
            updatedIntent.putExtra("bundleid", received);
            sendBroadcast(updatedIntent);
        }
        
        @Override
        public TransferMode startBlock(Block block) {
            // we are only interested in payload blocks (type = 1)
            if (block.type == 1) {
                // return SIMPLE mode to received the payload as "payload()" calls
                return TransferMode.SIMPLE;
            } else {
                // return NULL to discard the payload of this block
                return TransferMode.NULL;
            }
        }

        @Override
        public void endBlock() {
            // nothing to do here.
        }

        @Override
        public ParcelFileDescriptor fd() {
            // This method is used to hand-over a file descriptor to the
            // DTN service. We do not need the method here and always return
            // null.
            return null;
        }

        @Override
        public void payload(byte[] data) {
            // payload is received here
            Log.d(TAG, "payload received: " + data);
        }

        @Override
        public void progress(long offset, long length) {
            // if payload is written to a file descriptor, the progress
            // will be announced here
            Log.d(TAG, offset + " of " + length + " bytes received");
        }
    };
}
