package de.tubs.ibr.dtn.dtalkie.service;

import java.io.File;
import java.io.IOException;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import de.tubs.ibr.dtn.api.EID;
import de.tubs.ibr.dtn.api.GroupEndpoint;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.dtalkie.R;

public class Utils {
//	public static int ANDROID_API_ACTIONBAR = 16;
//	public static int ANDROID_API_ACTIONBAR_SETICON = 17;
	
	public static int ANDROID_API_ACTIONBAR = 11;
	public static int ANDROID_API_ACTIONBAR_SETICON = 14;
	
	private static final String TAG = "Utils";
	
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
            
            // add gallery hide file
            File hideFile = new File(sharefolder.getPath() + File.separatorChar + ".nomedia");
            if (!hideFile.exists()) {
                try {
                    hideFile.createNewFile();
                } catch (IOException e) {
                    Log.e(TAG, null, e);
                }
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
    
    public static EID getEndpoint(Bundle extras, String eid_key, String singleton_key, EID default_value) {
        EID destination = default_value;
        Boolean singleton = true;
        
        if (extras == null)
        	return destination;
    	
        if (extras.containsKey(eid_key)) {
            String dstr = extras.getString(eid_key);
            if (extras.containsKey(singleton_key)) {
                singleton = extras.getBoolean(singleton_key);
            }
            if (singleton) {
                destination = new SingletonEndpoint(dstr);
            } else {
                destination = new GroupEndpoint(dstr);
            }
        }
        
        return destination;
    }
    
    public static void putEndpoint(Intent intent, EID endpoint, String eid_key, String singleton_key) {
    	if (endpoint instanceof SingletonEndpoint) {
    		intent.putExtra(eid_key, endpoint.toString());
    		intent.putExtra(singleton_key, true);
    	}
    	else {
    		intent.putExtra(eid_key, endpoint.toString());
    		intent.putExtra(singleton_key, false);
    	}
    }
}
