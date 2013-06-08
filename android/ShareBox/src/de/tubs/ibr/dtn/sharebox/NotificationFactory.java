package de.tubs.ibr.dtn.sharebox;

import java.util.Date;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.preference.PreferenceManager;
import android.support.v4.app.NotificationCompat;
import android.text.Html;
import de.tubs.ibr.dtn.api.BundleID;
import de.tubs.ibr.dtn.api.EID;
import de.tubs.ibr.dtn.sharebox.data.Download;
import de.tubs.ibr.dtn.sharebox.data.Utils;
import de.tubs.ibr.dtn.sharebox.ui.PackageFileActivity;
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
    
    // while showing a notification for ongoing uploads we use
    // this to store the notification builder
    NotificationCompat.Builder mUploadBuilder = null;
    Long mUploadTimestamp = null;
    
    public NotificationFactory(Context context, NotificationManager manager) {
    	mContext = context;
    	mManager = manager;
    }
    
    /**
     * Notification methods for ongoing downloads
     */
    
    public void updateDownload(BundleID bundleid, int pos, int max) {
        // update notification
        mDownloadBuilder.setProgress(max, pos, false);
        
        // display the progress
        mManager.notify(bundleid.toString(), ONGOING_DOWNLOAD, mDownloadBuilder.build());
    }
    
    public void showDownload(Download d) {
        String bytesText = Utils.humanReadableByteCount(d.getLength(), true);
        
        // create notification with progressbar
        mDownloadBuilder = new NotificationCompat.Builder(mContext);
        mDownloadBuilder.setContentTitle(mContext.getString(R.string.notification_ongoing_download_title));
        mDownloadBuilder.setContentText(String.format(mContext.getString(R.string.notification_ongoing_download_text), bytesText));
        mDownloadBuilder.setSmallIcon(R.drawable.ic_stat_download);
        mDownloadBuilder.setProgress(0, 0, true);
        mDownloadBuilder.setOngoing(true);
        
        // display the progress
        mManager.notify(d.getBundleId().toString(), ONGOING_DOWNLOAD, mDownloadBuilder.build());
    }
    
    public void showDownloadCompleted(Download d) {
        String bytesText = Utils.humanReadableByteCount(d.getLength(), true);
        
        Uri downloadUri = Uri.fromParts("download", d.getId().toString(), "");
        
        // update notification
        mDownloadBuilder.setContentTitle(mContext.getString(R.string.notification_completed_download_title));
        mDownloadBuilder.setContentText(String.format(mContext.getString(R.string.notification_completed_download_text), bytesText, d.getBundleId().getSource()));
        mDownloadBuilder.setProgress(0, 0, false);
        mDownloadBuilder.setOngoing(false);
        mDownloadBuilder.setAutoCancel(true);
        
        // add direct intent to download
        Intent summaryIntent = new Intent(mContext, PackageFileActivity.class);
        summaryIntent.putExtra("download_id", d.getId());
        summaryIntent.setData(downloadUri);
        
        // create the pending intent
        PendingIntent contentIntent = PendingIntent.getActivity(mContext, 0, summaryIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        mDownloadBuilder.setContentIntent(contentIntent);
        
        // display the progress
        mManager.notify(d.getBundleId().toString(), ONGOING_DOWNLOAD, mDownloadBuilder.build());
    }

    public void showDownloadAborted(Download d) {
        // update notification
        mDownloadBuilder.setContentTitle(mContext.getString(R.string.notification_aborted_download_title));
        mDownloadBuilder.setContentText(String.format(mContext.getString(R.string.notification_aborted_download_text), d.getBundleId().getSource()));
        mDownloadBuilder.setProgress(0, 0, false);
        mDownloadBuilder.setOngoing(false);
        mDownloadBuilder.setAutoCancel(true);
        
        Intent resultIntent = new Intent(mContext, TransferListActivity.class);
        
        // create the pending intent
        PendingIntent contentIntent = PendingIntent.getActivity(mContext, 0, resultIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        mDownloadBuilder.setContentIntent(contentIntent);
        
        // display the progress
        mManager.notify(d.getBundleId().toString(), ONGOING_DOWNLOAD, mDownloadBuilder.build());
    }
    
    public void cancelDownload(BundleID bundleid) {
        mManager.cancel(bundleid.toString(), ONGOING_DOWNLOAD);
        mDownloadBuilder = null;
    }
    
    /**
     * Notification methods for ongoing uploads
     */
    
    public void updateUpload(int current, int max) {
        // update notification
        mUploadBuilder.setProgress(max, current, false);
        
        // display the progress
        mManager.notify(mUploadTimestamp.toString(), ONGOING_UPLOAD, mUploadBuilder.build());
    }
    
    public void updateUpload(String currentFile, int currentFileNum, int maxFiles) {
        // update notification
        mUploadBuilder.setProgress(1, 0, false);
        
        // status text
        String content = mContext.getString(R.string.notification_ongoing_upload_progress_text);
        
        // write current file into the description
        mUploadBuilder.setContentText(String.format(content, currentFile));

//        // progress text
//        String subtext = mContext.getString(R.string.notification_ongoing_upload_progress_subtext);
//        
//        // write the progress into the description
//        mUploadBuilder.setSubText(String.format(subtext, currentFileNum, maxFiles));
        
        // display the progress
        mManager.notify(mUploadTimestamp.toString(), ONGOING_UPLOAD, mUploadBuilder.build());
    }
    
    public void showUpload(EID destination, int maxFiles) {
        // create notification with progressbar
        mUploadBuilder = new NotificationCompat.Builder(mContext);
        mUploadTimestamp = (new Date()).getTime();
        mUploadBuilder.setContentTitle(mContext.getString(R.string.notification_ongoing_upload_title));
        mUploadBuilder.setContentText(mContext.getResources().getQuantityString(R.plurals.notification_ongoing_upload_text, maxFiles, maxFiles));
        mUploadBuilder.setSmallIcon(R.drawable.ic_stat_upload);
        mUploadBuilder.setProgress(0, 0, true);
        mUploadBuilder.setOngoing(true);
        
        // display the progress
        mManager.notify(mUploadTimestamp.toString(), ONGOING_UPLOAD, mUploadBuilder.build());
    }
    
    public void showUploadCompleted(int maxFiles) {
        // update notification
        mUploadBuilder.setContentTitle(mContext.getString(R.string.notification_completed_upload_title));
        mUploadBuilder.setContentText(mContext.getResources().getQuantityString(R.plurals.notification_completed_upload_text, maxFiles, maxFiles));
        mUploadBuilder.setProgress(0, 0, false);
        mUploadBuilder.setOngoing(false);
        mUploadBuilder.setAutoCancel(true);
        
        // add intent for transfer list
        Intent resultIntent = new Intent(mContext, TransferListActivity.class);
        
        // create the pending intent
        PendingIntent contentIntent = PendingIntent.getActivity(mContext, 0, resultIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        mUploadBuilder.setContentIntent(contentIntent);
        
        // display the progress
        mManager.notify(mUploadTimestamp.toString(), ONGOING_UPLOAD, mUploadBuilder.build());
    }

    public void showUploadAborted(EID destination) {
        // update notification
        mUploadBuilder.setContentTitle(mContext.getString(R.string.notification_aborted_upload_title));
        mUploadBuilder.setContentText(String.format(mContext.getString(R.string.notification_aborted_upload_text), destination.toString()));
        mUploadBuilder.setProgress(0, 0, false);
        mUploadBuilder.setOngoing(false);
        mUploadBuilder.setAutoCancel(true);
        
        // add intent for transfer list
        Intent resultIntent = new Intent(mContext, TransferListActivity.class);
        
        // create the pending intent
        PendingIntent contentIntent = PendingIntent.getActivity(mContext, 0, resultIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        mUploadBuilder.setContentIntent(contentIntent);
        
        // display the progress
        mManager.notify(mUploadTimestamp.toString(), ONGOING_UPLOAD, mUploadBuilder.build());
    }
    
    public void cancelUpload() {
    	if (mUploadTimestamp != null) {
	        mManager.cancel(mUploadTimestamp.toString(), ONGOING_UPLOAD);
	        mUploadBuilder = null;
	        mUploadTimestamp = null;
    	}
    }
    
    /**
     * Notification methods for pending downloads
     */
    
    public void showPendingDownload(Download d, int pendingCount) {
        NotificationCompat.Builder builder = new NotificationCompat.Builder(mContext);
        setNotificationSettings(builder);
        
        builder.setContentTitle(mContext.getResources().getQuantityString(R.plurals.notification_pending_download_title, pendingCount));
        builder.setSmallIcon(R.drawable.ic_stat_download);
        builder.setWhen( System.currentTimeMillis() );
        builder.setOnlyAlertOnce(true);
        
        Uri downloadUri = Uri.fromParts("download", d.getId().toString(), "");
        
        if (pendingCount == 1) {
            Intent dismissIntent = new Intent(mContext, DtnService.class);
            dismissIntent.setAction(DtnService.REJECT_DOWNLOAD_INTENT);
            dismissIntent.putExtra(DtnService.EXTRA_KEY_BUNDLE_ID, d.getBundleId());
            dismissIntent.setData(downloadUri);
            PendingIntent piDismiss = PendingIntent.getService(mContext, 0, dismissIntent, PendingIntent.FLAG_UPDATE_CURRENT);
            
            Intent acceptIntent = new Intent(mContext, DtnService.class);
            acceptIntent.setAction(DtnService.ACCEPT_DOWNLOAD_INTENT);
            acceptIntent.putExtra(DtnService.EXTRA_KEY_BUNDLE_ID, d.getBundleId());
            acceptIntent.setData(downloadUri);
            PendingIntent piAccept = PendingIntent.getService(mContext, 0, acceptIntent, PendingIntent.FLAG_UPDATE_CURRENT);
            
            builder.addAction(R.drawable.ic_stat_accept, mContext.getResources().getString(R.string.notification_accept_text), piAccept);
            builder.addAction(R.drawable.ic_stat_reject, mContext.getResources().getString(R.string.notification_reject_text), piDismiss);
            
            Intent dialogIntent = new Intent(mContext, PendingDialog.class);
            dialogIntent.putExtra(DtnService.EXTRA_KEY_BUNDLE_ID, d.getBundleId());
            dialogIntent.putExtra(DtnService.EXTRA_KEY_LENGTH, d.getLength());
            dialogIntent.setData(downloadUri);

            builder.setContentText(String.format(mContext.getString(R.string.notification_pending_download_from), d.getBundleId().toString()));
            builder.setTicker(mContext.getResources().getQuantityString(R.plurals.notification_pending_download_text, pendingCount, pendingCount));
            
            String htmlMessage = 
                    String.format(mContext.getString(R.string.transmission_summary_text),
                    		d.getBundleId().getSource().toString(), Utils.humanReadableByteCount(d.getLength(), true));
            
            builder.setStyle(new NotificationCompat.BigTextStyle().bigText(Html.fromHtml(htmlMessage)));
            
            // create the pending intent
            PendingIntent contentIntent = PendingIntent.getActivity(mContext, 0, dialogIntent, PendingIntent.FLAG_UPDATE_CURRENT);
            builder.setContentIntent(contentIntent);
        } else {
            builder.setContentText(mContext.getResources().getQuantityString(R.plurals.notification_pending_download_text, pendingCount, pendingCount));
            builder.setTicker(mContext.getResources().getQuantityString(R.plurals.notification_pending_download_text, pendingCount, pendingCount));
            
            Intent resultIntent = new Intent(mContext, TransferListActivity.class);
            
            // create the pending intent
            PendingIntent contentIntent = PendingIntent.getActivity(mContext, 0, resultIntent, PendingIntent.FLAG_UPDATE_CURRENT);
            builder.setContentIntent(contentIntent);
        }
        
        mManager.notify(PENDING_DOWNLOAD, builder.build());
    }
    
	public void cancelPending() {
		mManager.cancel(PENDING_DOWNLOAD);
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
