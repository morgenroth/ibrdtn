package de.tubs.ibr.dtn.dtalkie.service;

import java.io.File;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.net.Uri;
import android.os.Environment;
import de.tubs.ibr.dtn.dtalkie.R;

public class Utils {
//	public static int ANDROID_API_ACTIONBAR = 16;
//	public static int ANDROID_API_ACTIONBAR_SETICON = 17;
	
	public static int ANDROID_API_ACTIONBAR = 11;
	public static int ANDROID_API_ACTIONBAR_SETICON = 14;
	
	public static void showInstallServiceDialog(final Activity activity) {
		DialogInterface.OnClickListener dialogClickListener = new DialogInterface.OnClickListener() {
		    public void onClick(DialogInterface dialog, int which) {
		        switch (which){
		        case DialogInterface.BUTTON_POSITIVE:
					final Intent marketIntent = new Intent(Intent.ACTION_VIEW);
					marketIntent.setData(Uri.parse("market://details?id=de.tubs.ibr.dtn"));
					activity.startActivity(marketIntent);
		            break;

		        case DialogInterface.BUTTON_NEGATIVE:
		            break;
		        }
		        activity.finish();
		    }
		};

		AlertDialog.Builder builder = new AlertDialog.Builder(activity);
		builder.setMessage(activity.getResources().getString(R.string.alert_missing_daemon));
		builder.setPositiveButton(activity.getResources().getString(R.string.alert_yes), dialogClickListener);
		builder.setNegativeButton(activity.getResources().getString(R.string.alert_no), dialogClickListener);
		builder.show();
	}
	
	public static void showReinstallDialog(final Activity activity) {
		DialogInterface.OnClickListener dialogClickListener = new DialogInterface.OnClickListener() {
		    public void onClick(DialogInterface dialog, int which) {
		        switch (which){
		        case DialogInterface.BUTTON_POSITIVE:
					final Intent marketIntent = new Intent(Intent.ACTION_VIEW);
					marketIntent.setData(Uri.parse("market://details?id=" + activity.getApplication().getPackageName()));
					activity.startActivity(marketIntent);
		            break;

		        case DialogInterface.BUTTON_NEGATIVE:
		            break;
		        }
		        activity.finish();
		    }
		};

		AlertDialog.Builder builder = new AlertDialog.Builder(activity);
		builder.setMessage(activity.getResources().getString(R.string.alert_reinstall_app));
		builder.setPositiveButton(activity.getResources().getString(R.string.alert_yes), dialogClickListener);
		builder.setNegativeButton(activity.getResources().getString(R.string.alert_no), dialogClickListener);
		builder.show();
	}
	
    public static File getStoragePath()
    {
        if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED))
        {
            File externalStorage = Environment.getExternalStorageDirectory();
            
            // create working directory
            File sharefolder = new File(externalStorage.getPath() + File.separatorChar + "dtalkie");
            if (!sharefolder.exists())
            {
                    sharefolder.mkdir();
            }
            
            return sharefolder;
        }
        
        return null;
    }
    
    public static void lockScreenOrientation(Activity activity) {
        int orientation = activity.getResources().getConfiguration().orientation;
        activity.setRequestedOrientation(orientation);
    }

    public static void unlockScreenOrientation(Activity activity) {
        activity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_USER);
    }
}
