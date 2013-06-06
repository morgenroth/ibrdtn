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
import de.tubs.ibr.dtn.sharebox.ui.PendingDialog;
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
    NotificationCompat.Builder mDownloadBuilder = null;
    
    public NotificationFactory(Context context, NotificationManager manager) {
    	mContext = context;
    	mManager = manager;
    }
    
    public void updateDownload(BundleID bundleid, int pos, int max) {
        // update notification
        mDownloadBuilder.setProgress(max, pos, false);
        
        // display the progress
        mManager.notify(bundleid.toString(), ONGOING_DOWNLOAD, mDownloadBuilder.build());
    }
    
    public void showDownload(BundleID bundleid) {
        // create notification with progressbar
        mDownloadBuilder = new NotificationCompat.Builder(mContext);
        mDownloadBuilder.setContentTitle(mContext.getResources().getString(R.string.notification_ongoing_download_title));
        mDownloadBuilder.setContentText(mContext.getResources().getString(R.string.notification_ongoing_download_text));
        mDownloadBuilder.setSmallIcon(R.drawable.ic_stat_download);
        mDownloadBuilder.setProgress(0, 0, true);
        
        // display the progress
        mManager.notify(bundleid.toString(), ONGOING_DOWNLOAD, mDownloadBuilder.build());
    }
    
    public void showPendingDownload(BundleID bundleid, long length, int pendingCount) {
        NotificationCompat.Builder builder = new NotificationCompat.Builder(mContext);
        setNotificationSettings(builder);
        
        builder.setContentTitle(mContext.getResources().getQuantityString(R.plurals.notification_pending_download_title, pendingCount));
        builder.setContentText(mContext.getResources().getQuantityString(R.plurals.notification_pending_download_text, pendingCount));
        builder.setSmallIcon(R.drawable.ic_stat_download);
        builder.setWhen( System.currentTimeMillis() );
        builder.setOnlyAlertOnce(true);
        
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
            
            Intent resultIntent = new Intent(mContext, PendingDialog.class);
            resultIntent.putExtra(DtnService.PARCEL_KEY_BUNDLE_ID, bundleid);
            resultIntent.putExtra(DtnService.PARCEL_KEY_LENGTH, length);
            
            // create the pending intent
            PendingIntent contentIntent = PendingIntent.getActivity(mContext, 0, resultIntent, PendingIntent.FLAG_UPDATE_CURRENT);
            builder.setContentIntent(contentIntent);
        } else {
            Intent resultIntent = new Intent(mContext, TransferListActivity.class);
            
            // create the pending intent
            PendingIntent contentIntent = PendingIntent.getActivity(mContext, 0, resultIntent, PendingIntent.FLAG_UPDATE_CURRENT);
            builder.setContentIntent(contentIntent);
        }
        
        mManager.notify(PENDING_DOWNLOAD, builder.build());
    }
    
    public void showDownloadCompleted(BundleID bundleid) {
        // update notification
        mDownloadBuilder.setContentTitle(mContext.getResources().getString(R.string.notification_completed_download_title));
        mDownloadBuilder.setContentText(mContext.getResources().getString(R.string.notification_completed_download_text));
        mDownloadBuilder.setProgress(0, 0, false);
        
        // display the progress
        mManager.notify(bundleid.toString(), ONGOING_DOWNLOAD, mDownloadBuilder.build());
    }

	public void showDownloadAborted(BundleID bundleid) {
        // update notification
        mDownloadBuilder.setContentTitle(mContext.getResources().getString(R.string.notification_aborted_download_title));
        mDownloadBuilder.setContentText(mContext.getResources().getString(R.string.notification_aborted_download_text));
        mDownloadBuilder.setProgress(0, 0, false);
        
        // display the progress
        mManager.notify(bundleid.toString(), ONGOING_DOWNLOAD, mDownloadBuilder.build());
	}
	
	public void cancelPending() {
		mManager.cancel(PENDING_DOWNLOAD);
	}
	
	public void cancelDownload(BundleID bundleid) {
		mManager.cancel(bundleid.toString(), ONGOING_DOWNLOAD);
		mDownloadBuilder = null;
	}
	
    private void setNotificationSettings(NotificationCompat.Builder builder)
    {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
        
        // enable auto-cancel
        builder.setAutoCancel(true);
        
        int defaults = 0;
        
        if (prefs.getBoolean("notifications", true)) {
            if (prefs.getBoolean("notifications_pending_download_vibrate", true)) {
                defaults |= Notification.DEFAULT_VIBRATE;
            }
            
            builder.setDefaults(defaults);
            builder.setLights(0xff0080ff, 300, 1000);
            builder.setSound( Uri.parse( prefs.getString("notifications_pending_download_ringtone", "content://settings/system/notification_sound") ) );
        }
    }
}
