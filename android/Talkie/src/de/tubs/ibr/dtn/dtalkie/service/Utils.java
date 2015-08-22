package de.tubs.ibr.dtn.dtalkie.service;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.content.res.AssetFileDescriptor;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.provider.MediaStore;
import android.util.Log;
import de.tubs.ibr.dtn.api.EID;
import de.tubs.ibr.dtn.api.GroupEndpoint;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.dtalkie.R;

public class Utils {
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
	
	public static File getStoragePath(Context context)
	{
		File externalStorage = context.getFilesDir();

		// create working directory
		File sharefolder = new File(externalStorage.getPath() + File.separatorChar + "audio");
		if (!sharefolder.exists())
		{
			sharefolder.mkdir();
		}

		return sharefolder;
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

	public static void registerNotificationSound(Context context) throws IOException {
		// file path for ring-tones
		File filePath = context.getExternalFilesDir(Environment.DIRECTORY_NOTIFICATIONS);
		File soundFile = new File(filePath, "talkie_message.mp3");
		
		// copy the file to the external storage
		AssetFileDescriptor fd = context.getResources().openRawResourceFd(R.raw.talkie_message);
		long file_size = fd.getLength();
		
		// query for current entry
		Cursor c = context.getContentResolver().query(MediaStore.Audio.Media.INTERNAL_CONTENT_URI, new String[] { MediaStore.MediaColumns.SIZE }, MediaStore.MediaColumns.DATA + " = ?", new String[] { soundFile.getAbsolutePath() }, null);
		
		if (c != null && c.moveToNext()) {
			// check if file-size matches the current one
			if (c.getLong(c.getColumnIndex(MediaStore.MediaColumns.SIZE)) == file_size) return;
			
			// delete old Uri
			context.getContentResolver().delete(MediaStore.Audio.Media.INTERNAL_CONTENT_URI, MediaStore.MediaColumns.DATA + " = ?", new String[] { soundFile.getAbsolutePath() });
		}
		
		// delete previous file
		if (soundFile.exists()) soundFile.delete();
		
		FileInputStream fis = fd.createInputStream();
		FileOutputStream fos = new FileOutputStream(soundFile);
		byte[] readData = new byte[1024*500];
		int i = fis.read(readData);
		
		while (i != -1) {
			fos.write(readData, 0, i);
			i = fis.read(readData);
		}

		fos.close();
		
		ContentValues values = new ContentValues();
		values.put(MediaStore.MediaColumns.DATA, soundFile.getAbsolutePath());
		values.put(MediaStore.MediaColumns.TITLE, context.getString(R.string.talkie_message_notification_sound_title));
		values.put(MediaStore.MediaColumns.MIME_TYPE, "audio/mp3");
		values.put(MediaStore.Audio.Media.ARTIST, "Talkie");
		values.put(MediaStore.MediaColumns.SIZE, file_size);
		values.put(MediaStore.Audio.Media.IS_RINGTONE, false);
		values.put(MediaStore.Audio.Media.IS_NOTIFICATION, true);
		values.put(MediaStore.Audio.Media.IS_ALARM, false);
		values.put(MediaStore.Audio.Media.IS_MUSIC, false);
		
		// add new Uri
		Uri uri = context.getContentResolver().insert(MediaStore.Audio.Media.INTERNAL_CONTENT_URI, values);
		
		// set default ring-tone for this application
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		prefs.edit().putString("ringtoneOnMessage", uri.toString()).commit();
		
		Log.i("Utils", "Notification sound installed");
	}
}
