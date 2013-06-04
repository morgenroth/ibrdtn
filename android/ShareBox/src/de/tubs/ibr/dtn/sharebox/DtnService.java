package de.tubs.ibr.dtn.sharebox;

import java.util.LinkedList;

import android.app.IntentService;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.util.Log;
import de.tubs.ibr.dtn.api.Block;
import de.tubs.ibr.dtn.api.Bundle;
import de.tubs.ibr.dtn.api.BundleID;
import de.tubs.ibr.dtn.api.DTNClient;
import de.tubs.ibr.dtn.api.DTNClient.Session;
import de.tubs.ibr.dtn.api.DataHandler;
import de.tubs.ibr.dtn.api.GroupEndpoint;
import de.tubs.ibr.dtn.api.Registration;
import de.tubs.ibr.dtn.api.ServiceNotAvailableException;
import de.tubs.ibr.dtn.api.SessionConnection;
import de.tubs.ibr.dtn.api.SessionDestroyedException;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.api.TransferMode;
import de.tubs.ibr.dtn.sharebox.data.Database;

public class DtnService extends IntentService {

    // This TAG is used to identify this class (e.g. for debugging)
    private static final String TAG = "DtnService";
    
    // mark a specific bundle as delivered
    public static final String MARK_DELIVERED_INTENT = "de.tubs.ibr.dtn.sharebox.MARK_DELIVERED";
    
    // process a status report
    public static final String REPORT_DELIVERED_INTENT = "de.tubs.ibr.dtn.sharebox.REPORT_DELIVERED";
    
    // download a bundle
    public static final String DOWNLOAD_INTENT = "de.tubs.ibr.dtn.sharebox.DOWNLOAD";
    
    // group EID of this app
    public static final GroupEndpoint SHAREBOX_GROUP_EID = new GroupEndpoint("dtn://broadcast.dtn/sharebox");

    // This is the object that receives interactions from clients.  See
    // RemoteService for a more complete example.
    private final IBinder mBinder = new LocalBinder();
    
    // The communication with the DTN service is done using the DTNClient
    private DTNClient mClient = null;
    
    // should be set to true if a download is requested 
    private Boolean mIsDownloading = false;
    
    private ServiceError mServiceError = ServiceError.NO_ERROR;
    
    // the database
    private Database mDatabase = null;
    
    public DtnService() {
        super(TAG);
    }
    
    /**
     * Class for clients to access.  Because we know this service always
     * runs in the same process as its clients, we don't need to deal with
     * IPC.
     */
    public class LocalBinder extends Binder {
        public DtnService getService() {
            return DtnService.this;
        }
    }
    
    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }
    
    public ServiceError getServiceError() {
        return mServiceError;
    }
    
    public Database getDatabase() {
        return mDatabase;
    }
    
    @Override
    protected void onHandleIntent(Intent intent) {
        String action = intent.getAction();
        
        if (de.tubs.ibr.dtn.Intent.RECEIVE.equals(action))
        {
            // Receive bundle info from the DTN service here
            try {
                // We loop here until no more bundles are available
                // (queryNext() returns false)
                mIsDownloading = false;
                while (mClient.getSession().queryInfoNext());
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
        else if (DOWNLOAD_INTENT.equals(action))
        {
            // retrieve the bundle ID of the intent
            BundleID bundleid = intent.getParcelableExtra("bundleid");
            
            Log.d(TAG, "Download request for " + bundleid.toString());
            
            // TODO: create notification with progressbar

            try {
                mIsDownloading = true;
                mClient.getSession().query(bundleid);
                
                // TODO: show download completed notification
            } catch (SessionDestroyedException e) {
                Log.e(TAG, "Can not query for bundle", e);
                
                // TODO: show download aborted notification
            } catch (InterruptedException e) {
                Log.e(TAG, "Can not query for bundle", e);
                
                // TODO: show download aborted notification
            }
            
            // TODO: clear notification with progressbar
        }
    }
    
    SessionConnection mSession = new SessionConnection() {

        @Override
        public void onSessionConnected(Session session) {
            Log.d(TAG, "Session connected");
        }

        @Override
        public void onSessionDisconnected() {
            Log.d(TAG, "Session disconnected");
        }
        
    };

    @Override
    public void onCreate() {
        super.onCreate();
        
        // create message database
        mDatabase = new Database(this);
        
        // create a new DTN client
        mClient = new DTNClient(mSession);
        
        // create registration with "sharebox" as endpoint
        // if the EID of this device is "dtn://device" then the
        // address of this app will be "dtn://device/sharebox"
        Registration registration = new Registration("sharebox");
        
        // additionally join a group
        registration.add(SHAREBOX_GROUP_EID);
        
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
        
        // close the database
        mDatabase.close();
        
        super.onDestroy();
    }
    
    /**
     * This data handler is used to process incoming bundles
     */
    private DataHandler mDataHandler = new DataHandler() {

        private Bundle mBundle = null;
        private LinkedList<Block> mBlocks = null;

        @Override
        public void startBundle(Bundle bundle) {
            // store the bundle header locally
            mBundle = bundle;
            mBlocks = new LinkedList<Block>();
        }

        @Override
        public void endBundle() {
            // complete bundle received
            BundleID received = new BundleID(mBundle);
            
            if (mIsDownloading) {
                // mark the bundle as delivered if this was a complete download
                Intent i = new Intent(DtnService.this, DtnService.class);
                i.setAction(MARK_DELIVERED_INTENT);
                i.putExtra("bundleid", received);
                startService(i);
            } else {
                // TODO: put download into the database
            }
            
            // free the bundle header
            mBundle = null;
        }
        
        @Override
        public TransferMode startBlock(Block block) {
            // we are only interested in payload blocks (type = 1)
            if (block.type == 1) {
                // retrieve payload when downloading
                if (mIsDownloading) {
                    // return FILEDESCRIPTOR mode to received the payload using fd()
                    return TransferMode.FILEDESCRIPTOR;
                }
                
                // store the block data
                mBlocks.push(block);

                return TransferMode.NULL;
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
            // TODO: generate a new parcel file descriptor to recieve the content
            // of the bundle
            return null;
        }

        @Override
        public void payload(byte[] data) {
            // nothing to do here. 
        }

        @Override
        public void progress(long offset, long length) {
            // if payload is written to a file descriptor, the progress
            // will be announced here
            Log.d(TAG, offset + " of " + length + " bytes received");
            
            if (mIsDownloading) {
                // TODO: update notification
            }
        }
    };
}
