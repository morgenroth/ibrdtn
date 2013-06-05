package de.tubs.ibr.dtn.sharebox;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.LinkedList;

import android.app.IntentService;
import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
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
import de.tubs.ibr.dtn.sharebox.data.Download;
import de.tubs.ibr.dtn.sharebox.data.Utils;
import de.tubs.ibr.dtn.sharebox.data.Download.State;

public class DtnService extends IntentService {

    // This TAG is used to identify this class (e.g. for debugging)
    private static final String TAG = "DtnService";
    
    // mark a specific bundle as delivered
    public static final String MARK_DELIVERED_INTENT = "de.tubs.ibr.dtn.sharebox.MARK_DELIVERED";
    
    // process a status report
    public static final String REPORT_DELIVERED_INTENT = "de.tubs.ibr.dtn.sharebox.REPORT_DELIVERED";
    
    // download or rejet a bundle
    public static final String ACCEPT_DOWNLOAD_INTENT = "de.tubs.ibr.dtn.sharebox.ACCEPT_DOWNLOAD";
    public static final String REJECT_DOWNLOAD_INTENT = "de.tubs.ibr.dtn.sharebox.REJECT_DOWNLOAD";
    
    // pending download intent
    public static final String PENDING_DOWNLOAD_INTENT = "de.tubs.ibr.dtn.sharebox.PENDING_DOWNLOAD";
    
    // local endpoint
    public static final String SHAREBOX_APP_ENDPOINT = "sharebox";
    
    // group EID of this app
    public static final GroupEndpoint SHAREBOX_GROUP_EID = new GroupEndpoint("dtn://broadcast.dtn/sharebox");
    
    public static final String PARCEL_KEY_BUNDLE_ID = "bundleid";
    public static final String PARCEL_KEY_SOURCE = "source";

    // This is the object that receives interactions from clients.  See
    // RemoteService for a more complete example.
    private final IBinder mBinder = new LocalBinder();
    
    // The communication with the DTN service is done using the DTNClient
    private DTNClient mClient = null;
    
    // should be set to true if a download is requested 
    private Boolean mIsDownloading = false;
    
    // handle of the notification manager
    NotificationFactory mNotificationFactory = null;
    
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
            BundleID bundleid = intent.getParcelableExtra(PARCEL_KEY_BUNDLE_ID);
            
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
            SingletonEndpoint source = intent.getParcelableExtra(PARCEL_KEY_SOURCE);
            
            // retrieve the bundle ID of the intent
            BundleID bundleid = intent.getParcelableExtra(PARCEL_KEY_BUNDLE_ID);
            
            Log.d(TAG, "Status report received for " + bundleid.toString() + " from " + source.toString());
        }
        else if (REJECT_DOWNLOAD_INTENT.equals(action))
        {           
            // retrieve the bundle ID of the intent
            BundleID bundleid = intent.getParcelableExtra(PARCEL_KEY_BUNDLE_ID);
            
            // dismiss notification if there are no more pending downloads
            if (mDatabase.getPending() <= 1) {
                mNotificationFactory.cancelPendingDownload(bundleid);
            }
            
            // mark the bundle as delivered
            Intent i = new Intent(DtnService.this, DtnService.class);
            i.setAction(MARK_DELIVERED_INTENT);
            i.putExtra(PARCEL_KEY_BUNDLE_ID, bundleid);
            startService(i);
            
            // delete the pending bundle
            mDatabase.remove(bundleid);
        }
        else if (ACCEPT_DOWNLOAD_INTENT.equals(action))
        {
            // retrieve the bundle ID of the intent
            BundleID bundleid = intent.getParcelableExtra(PARCEL_KEY_BUNDLE_ID);
            
            // dismiss notification if there are no more pending downloads
            if (mDatabase.getPending() <= 1) {
                mNotificationFactory.cancelPendingDownload(bundleid);
            }
            
            Log.d(TAG, "Download request for " + bundleid.toString());
            
            // mark the download as accepted
            Download d = mDatabase.get(bundleid);
            mDatabase.setState(d.getId(), 1);

            try {
                mIsDownloading = true;
                mClient.getSession().query(bundleid);
            } catch (SessionDestroyedException e) {
                Log.e(TAG, "Can not query for bundle", e);
                
                mNotificationFactory.showDownloadAborted(bundleid);
            } catch (InterruptedException e) {
                Log.e(TAG, "Can not query for bundle", e);
                
                mNotificationFactory.showDownloadAborted(bundleid);
            }
        } else if (PENDING_DOWNLOAD_INTENT.equals(action)) {
            // retrieve the bundle ID of the intent
            BundleID bundleid = intent.getParcelableExtra(PARCEL_KEY_BUNDLE_ID);

            // get the number of pending downloads
            Integer pendingCount = mDatabase.getPending();
            
            if (pendingCount > 0) {
            	mNotificationFactory.showPendingDownload(bundleid, pendingCount);
            }
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
        
        // create notification factory
        mNotificationFactory = new NotificationFactory( this, (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE) );
        
        // create message database
        mDatabase = new Database(this);
        
        // create a new DTN client
        mClient = new DTNClient(mSession);
        
        // create registration with "sharebox" as endpoint
        // if the EID of this device is "dtn://device" then the
        // address of this app will be "dtn://device/sharebox"
        Registration registration = new Registration(SHAREBOX_APP_ENDPOINT);
        
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
        private BundleID mBundleId = null;
        private LinkedList<Block> mBlocks = null;
        
        private ParcelFileDescriptor mWriteFd = null;
        private ParcelFileDescriptor mReadFd = null;
        
        private Object mExtractorLock = new Object();
        private TarExtractor mExtractor = null;
        private Thread mExtractorThread = null;
        
        private int mLastProgressValue = 0;

        @Override
        public void startBundle(Bundle bundle) {
            // store the bundle header locally
            mBundle = bundle;
            mBundleId = new BundleID(bundle);
            
            // show ongoing download notification
            mNotificationFactory.showOngoingDownload(mBundleId);
        }

        @Override
        public void endBundle() {
            if (mIsDownloading) {
                // clear notification with progressbar
                mNotificationFactory.cancelOngoingDownload(mBundleId);
                mNotificationFactory.showDownloadCompleted(mBundleId);
                
                // mark the bundle as delivered if this was a complete download
                Intent i = new Intent(DtnService.this, DtnService.class);
                i.setAction(MARK_DELIVERED_INTENT);
                i.putExtra(PARCEL_KEY_BUNDLE_ID, mBundleId);
                startService(i);

                // set state to completed
                Download d = mDatabase.get(mBundleId);
                mDatabase.setState(d.getId(), 2);
            } else {
                // create new download object
                Download download_request = new Download(mBundle);
                
                // get payload length
                long len = 0L;
                
                if (mBlocks != null) {
                    for (Block b : mBlocks) {
                        if (b.type == 1) {
                            len += b.length;
                            break;
                        }
                    }
                }
                
                // set payload length
                download_request.setLength(len);
                
                // set request to pending
                download_request.setState(State.PENDING);
                
                // put download object into the database
                mDatabase.put(download_request);
                
                // announce pending bundle
                Intent i = new Intent(DtnService.this, DtnService.class);
                i.setAction(PENDING_DOWNLOAD_INTENT);
                i.putExtra(PARCEL_KEY_BUNDLE_ID, download_request.getBundleId());
                startService(i);
            }
            
            // free the bundle header
            mBundle = null;
            mBundleId = null;
            mBlocks = null;
        }
        
        @Override
        public TransferMode startBlock(Block block) {
            // we are only interested in payload blocks (type = 1)
            if (block.type == 1) {
                // retrieve payload when downloading
                if (mIsDownloading) {
                    File folder = Utils.getStoragePath();
                    
                    // do not store any data if there is no space to store
                    if (folder == null)
                        return TransferMode.NULL;
                    
                    // create new filedescriptor
                    try {
                        ParcelFileDescriptor[] p = ParcelFileDescriptor.createPipe();
                        mReadFd = p[0];
                        mWriteFd = p[1];
                        
                        synchronized(mExtractorLock) {
                            while (mExtractor != null) {
                                mExtractorLock.wait();
                            }
                            // create a new tar extractor
                            mExtractor = new TarExtractor(new FileInputStream(mReadFd.getFileDescriptor()), folder);
                            mExtractor.setOnStateChangeListener(mExtractorListener);
                            mExtractorThread = new Thread(mExtractor);
                            mExtractorThread.start();
                        }
                        
                        // return FILEDESCRIPTOR mode to received the payload using fd()
                        return TransferMode.FILEDESCRIPTOR;
                    } catch (FileNotFoundException e) {
                        Log.e(TAG, "Can not create a filedescriptor.", e);
                    } catch (IOException e) {
                        Log.e(TAG, "Can not create a filedescriptor.", e);
                    } catch (InterruptedException e) {
                        Log.e(TAG, "Interrupted while waiting for extractor clearance.", e);
                    }
                        
                    return TransferMode.NULL;
                }
                
                if (mBlocks == null) {
                    mBlocks = new LinkedList<Block>();
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
            // reset progress
            mLastProgressValue = 0;
            
            if (mWriteFd != null)
            {
                // close filedescriptor
                try {
                    mWriteFd.close();
                    mWriteFd = null;
                } catch (IOException e) {
                    Log.e(TAG, "Can not close filedescriptor.", e);
                }
            }
        }

        @Override
        public ParcelFileDescriptor fd() {
            return mWriteFd;
        }

        @Override
        public void payload(byte[] data) {
            // nothing to do here. 
        }

        @Override
        public void progress(long offset, long length) {
            // if payload is written to a file descriptor, the progress
            // will be announced here
            
            // scale the download progress to 0..100
            Double pos = Double.valueOf(offset) / Double.valueOf(length) * 100.0;
            
            if (mIsDownloading && (mLastProgressValue < pos.intValue())) {
                mLastProgressValue = pos.intValue();
                mNotificationFactory.updateOngoingDownload(mBundleId, mLastProgressValue, 100);
            }
        }
        
        private TarExtractor.OnStateChangeListener mExtractorListener = new TarExtractor.OnStateChangeListener() {

            @Override
            public void onStateChanged(int state) {
                synchronized(mExtractorLock) {
                    switch (state) {
                        case 1:
                            // successful
                            try {
                                mReadFd.close();
                            } catch (IOException e) {
                                Log.e(TAG, "Can not close filedescriptor.", e);
                            }
                            
                            mReadFd = null;
                            
                            if (mExtractor != null) {
                                // put files into the database
                                for (File f : mExtractor.getFiles()) {
                                    Log.d(TAG, "Extracted file: " + f.getAbsolutePath());
                                    mDatabase.put(mBundleId, f);
                                    
                                    // add new file to the media library
                                    sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, Uri.fromFile(f)));
                                }
                            }
                            
                            mExtractor = null;
                            mExtractorThread = null;
                            mExtractorLock.notifyAll();
                            break;
                            
                        case -1:
                            // error
                            try {
                                mReadFd.close();
                            } catch (IOException e) {
                                Log.e(TAG, "Can not close filedescriptor.", e);
                            }
                            
                            mReadFd = null;
                            mExtractor = null;
                            mExtractorThread = null;
                            mExtractorLock.notifyAll();
                            break;
                    }
                }
            }
            
        };
    };
}
