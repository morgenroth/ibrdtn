package de.tubs.ibr.dtn.sharebox;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.preference.PreferenceManager;
import android.support.v4.app.NotificationCompat;
import de.tubs.ibr.dtn.api.BundleID;
import de.tubs.ibr.dtn.sharebox.ui.TransferListActivity;

public class NotificationFactory {
	
    // ids for download notifications
	public static final int ONGOING_DOWNLOAD   = 1;
	public static final int PENDING_DOWNLOAD   = 2;
	public static final int COMPLETED_DOWNLOAD = 3;
    public static final int ABORTED_DOWNLOAD   = 4;
    
    // ids for upload notifications
    public static final int ONGOING_UPLOAD     = 5;
    public static final int COMPLETED_UPLOAD   = 6;
    public static final int ABORTED_UPLOAD     = 7;
    
    private Context mContext = null;
    private NotificationManager mManager = null;
    
    // while showing a notification for ongoing downloads we use
    // this to store the notification builder
    NotificationCompat.Builder mOngoingDownloadBuilder = null;
    
    public NotificationFactory(Context context, NotificationManager manager) {
    	mContext = context;
    	mManager = manager;
    }
    
    public void updateOngoingDownload(BundleID bundleid, int pos, int max) {
        // update notification
        mOngoingDownloadBuilder.setProgress(max, pos, false);
        
        // display the progress
        mManager.notify(ONGOING_DOWNLOAD, mOngoingDownloadBuilder.build());
    }
    
    public void showOngoingDownload(BundleID bundleid) {
        // create notification with progressbar
        mOngoingDownloadBuilder = new NotificationCompat.Builder(mContext);
        mOngoingDownloadBuilder.setContentTitle(mContext.getResources().getString(R.string.notification_ongoing_download_title));
        mOngoingDownloadBuilder.setContentText(mContext.getResources().getString(R.string.notification_ongoing_download_text));
        mOngoingDownloadBuilder.setSmallIcon(R.drawable.ic_stat_download);
        
        // display the progress
        mManager.notify(ONGOING_DOWNLOAD, mOngoingDownloadBuilder.build());
    }
    
    public void showPendingDownload(BundleID bundleid, int pendingCount) {
        NotificationCompat.Builder builder = new NotificationCompat.Builder(mContext);
        setNotificationSettings(builder);
        
        Intent resultIntent = new Intent(mContext, TransferListActivity.class);
        
        builder.setContentTitle(mContext.getResources().getQuantityString(R.plurals.notification_pending_download_title, pendingCount));
        builder.setContentText(mContext.getResources().getQuantityString(R.plurals.notification_pending_download_text, pendingCount));
        builder.setSmallIcon(R.drawable.ic_stat_download);
        
        if (pendingCount == 1) {
            Intent dismissIntent = new Intent(mContext, DtnService.class);
            dismissIntent.setAction(DtnService.REJECT_DOWNLOAD_INTENT);
            dismissIntent.putExtra(DtnService.PARCEL_KEY_BUNDLE_ID, bundleid);
            PendingIntent piDismiss = PendingIntent.getService(mContext, 0, dismissIntent, PendingIntent.FLAG_UPDATE_CURRENT);
            
            Intent acceptIntent = new Intent(mContext, DtnService.class);
            acceptIntent.setAction(DtnService.ACCEPT_DOWNLOAD_INTENT);
            acceptIntent.putExtra(DtnService.PARCEL_KEY_BUNDLE_ID, bundleid);
            PendingIntent piAccept = PendingIntent.getService(mContext, 0, acceptIntent, PendingIntent.FLAG_UPDATE_CURRENT);
            
            builder.addAction(R.drawable.ic_stat_accept, mContext.getResources().getString(R.string.notification_accept_text), piAccept);
            builder.addAction(R.drawable.ic_stat_reject, mContext.getResources().getString(R.string.notification_reject_text), piDismiss);
        }
        
        // create the pending intent
        PendingIntent contentIntent = PendingIntent.getActivity(mContext, 0, resultIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        
        builder.setWhen( System.currentTimeMillis() );
        builder.setContentIntent(contentIntent);
        
        mManager.notify(PENDING_DOWNLOAD, builder.build());
    }
    
    public void showDownloadCompleted(BundleID bundleid) {
        NotificationCompat.Builder builder = new NotificationCompat.Builder(mContext);
        
        Intent resultIntent = new Intent(mContext, TransferListActivity.class);
        resultIntent.putExtra(DtnService.PARCEL_KEY_BUNDLE_ID, bundleid);
        
        builder.setContentTitle(mContext.getResources().getString(R.string.notification_completed_download_title));
        builder.setContentText(mContext.getResources().getString(R.string.notification_completed_download_text));
        builder.setSmallIcon(R.drawable.ic_stat_download);
        
        // create the pending intent
        PendingIntent contentIntent = PendingIntent.getActivity(mContext, 0, resultIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        builder.setContentIntent(contentIntent);
        
        builder.setWhen( System.currentTimeMillis() );

        // show download completed notification
        mManager.notify(bundleid.toString(), COMPLETED_DOWNLOAD, builder.build());
    }

	public void showDownloadAborted(BundleID bundleid) {
        NotificationCompat.Builder builder = new NotificationCompat.Builder(mContext);
        
        Intent resultIntent = new Intent(mContext, TransferListActivity.class);
        resultIntent.putExtra(DtnService.PARCEL_KEY_BUNDLE_ID, bundleid);
        
        builder.setContentTitle(mContext.getResources().getString(R.string.notification_aborted_download_title));
        builder.setContentText(mContext.getResources().getString(R.string.notification_aborted_download_text));
        builder.setSmallIcon(R.drawable.ic_stat_download);
        
        // create the pending intent
        PendingIntent contentIntent = PendingIntent.getActivity(mContext, 0, resultIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        builder.setContentIntent(contentIntent);
        
        builder.setWhen( System.currentTimeMillis() );

        // show download aborted notification
        mManager.notify(bundleid.toString(), ABORTED_DOWNLOAD, builder.build());
	}
	
	public void cancelPendingDownload(BundleID bundleid) {
		mManager.cancel(bundleid.toString(), PENDING_DOWNLOAD);
	}
	
	public void cancelOngoingDownload(BundleID bundleid) {
		mManager.cancel(bundleid.toString(), ONGOING_DOWNLOAD);
		mOngoingDownloadBuilder = null;
	}
	
    private void setNotificationSettings(NotificationCompat.Builder builder)
    {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
        
        // enable auto-cancel
        builder.setAutoCancel(true);
        
        int defaults = 0;
        
        if (prefs.getBoolean("vibrateOnMessage", true)) {
            defaults |= Notification.DEFAULT_VIBRATE;
        }
        
        builder.setDefaults(defaults);
        builder.setLights(0xff0080ff, 300, 1000);
        builder.setSound( Uri.parse( prefs.getString("ringtoneOnMessage", "content://settings/system/notification_sound") ) );
    }
}
