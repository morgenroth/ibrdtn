package de.tubs.ibr.dtn.keyexchange;

import android.app.IntentService;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Parcelable;
import android.support.v4.app.NotificationCompat;
import android.util.Log;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.SingletonEndpoint;

public class KeyExchangeService extends IntentService {

	private static final String TAG = "KeyExchangeService";
	
	public static final String INTENT_KEY_EXCHANGE = "de.tubs.ibr.dtn.Intent.KEY_EXCHANGE";
	public static final String INTENT_NFC_EXCHANGE = "de.tubs.ibr.dtn.Intent.NFC_EXCHANGE";
	
	public static final String EXTRA_ACTION = "action";
	public static final String EXTRA_SESSION = "session";
	public static final String EXTRA_PROTOCOL = "protocol";
	public static final String EXTRA_NAME = "name";
	public static final String EXTRA_ENDPOINT = "EID";
	public static final String EXTRA_DATA = "data";
	public static final String EXTRA_PASSWORD = "password";
	public static final String EXTRA_FLAGS = "flags";
	
	public static final String ACTION_COMPLETE = "COMPLETE";
	public static final String ACTION_PASSWORD_REQUEST = "PASSWORDREQUEST";
	public static final String ACTION_PASSWORD_WRONG = "WRONGPASSWORD";
	public static final String ACTION_HASH_COMMIT = "HASHCOMMIT";
	public static final String ACTION_NEW_KEY = "NEWKEY";
	public static final String ACTION_ERROR = "ERROR";
	public static final String ACTION_KEY_UPDATED = "KEYUPDATED";
	
	public KeyExchangeService() {
		super(TAG);
	}
	
	@Override
	protected void onHandleIntent(Intent intent) {
		// get the intent action
		String action = intent.getAction();
		
		// get the endpoint
		SingletonEndpoint endpoint = new SingletonEndpoint(intent.getStringExtra(EXTRA_ENDPOINT));
		
		// get the session ID
		int session = Integer.valueOf(intent.hasExtra(KeyExchangeService.EXTRA_SESSION) ? intent.getStringExtra(KeyExchangeService.EXTRA_SESSION) : "-1");
		
		// get the protocol ID
		int protocol = Integer.valueOf(intent.hasExtra(KeyExchangeService.EXTRA_PROTOCOL) ? intent.getStringExtra(KeyExchangeService.EXTRA_PROTOCOL) : "-1");
		
		// prepare notification builder
		NotificationCompat.Builder builder = new NotificationCompat.Builder(this);

		// set default small icon
		builder.setSmallIcon(R.drawable.ic_stat_security);
		
		// set identicon as large icon
		Bitmap identicon = Utils.createIdenticon(endpoint.toString());
		int icon_size = getResources().getDimensionPixelSize(R.dimen.notification_large_icon_size);
		builder.setLargeIcon(Bitmap.createScaledBitmap(identicon, icon_size, icon_size, false));
		
		// enable auto-cancel feature
		builder.setAutoCancel(true);
		
		// get notification manager
		NotificationManager notificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
		
		Log.d(TAG, "Handle intent " + action + " for session " + session + "[" + endpoint + "]");

		if (ACTION_COMPLETE.equals(action))
		{
			builder.setContentTitle(getString(R.string.notification_success));
			builder.setContentText(getString(R.string.notification_success_text));
			
			Intent i = new Intent(this, KeyInformationActivity.class);
			i.setAction(intent.getAction());
			
			i.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)endpoint);
			i.putExtra(EXTRA_SESSION, session);
			i.putExtra(EXTRA_PROTOCOL, protocol);
			
			i.setData(Uri.parse(endpoint.toString()));
			builder.setContentIntent(PendingIntent.getActivity(this, 0, i, PendingIntent.FLAG_UPDATE_CURRENT));
			
			notificationManager.notify(session, builder.build());
		}
		else if (ACTION_PASSWORD_REQUEST.equals(action))
		{
			builder.setContentTitle(getString(R.string.notification_password_request));
			builder.setContentText(getString(R.string.notification_password_request_text));

			Intent i = new Intent(this, KeyInformationActivity.class);
			i.setAction(action);
			
			i.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)endpoint);
			i.putExtra(EXTRA_SESSION, session);
			i.putExtra(EXTRA_PROTOCOL, EnumProtocol.JPAKE.getValue());
			
			i.setData(Uri.parse(endpoint.toString()));
			builder.setContentIntent(PendingIntent.getActivity(this, 0, i, PendingIntent.FLAG_UPDATE_CURRENT));
			
			notificationManager.notify(session, builder.build());
		}
		else if (ACTION_PASSWORD_WRONG.equals(action))
		{
			builder.setContentTitle(getString(R.string.notification_password_wrong));
			builder.setContentText(getString(R.string.notification_password_wrong_text));

			Intent i = new Intent(this, KeyInformationActivity.class);
			i.setAction(action);
			
			i.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)endpoint);
			i.putExtra(EXTRA_SESSION, session);
			i.putExtra(EXTRA_PROTOCOL, EnumProtocol.JPAKE.getValue());
			
			i.setData(Uri.parse(endpoint.toString()));
			builder.setContentIntent(PendingIntent.getActivity(this, 0, i, PendingIntent.FLAG_UPDATE_CURRENT));
			
			notificationManager.notify(session, builder.build());
		}
		else if (ACTION_HASH_COMMIT.equals(action))
		{
			builder.setContentTitle(getString(R.string.notification_hash_compare));
			builder.setContentText(getString(R.string.notification_hash_compare_text));

			Intent i = new Intent(this, KeyInformationActivity.class);
			i.setAction(action);
			
			i.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)endpoint);
			i.putExtra(EXTRA_SESSION, session);
			i.putExtra(EXTRA_PROTOCOL, EnumProtocol.HASH.getValue());
			i.putExtra(EXTRA_DATA, intent.getStringExtra(EXTRA_DATA));
			
			i.setData(Uri.parse(endpoint.toString()));
			builder.setContentIntent(PendingIntent.getActivity(this, 0, i, PendingIntent.FLAG_UPDATE_CURRENT));
			
			notificationManager.notify(session, builder.build());
		}
		else if (ACTION_NEW_KEY.equals(action))
		{
			builder.setContentTitle(getString(R.string.notification_not_equal));
			builder.setContentText(getString(R.string.notification_not_equal_text));

			Intent i = new Intent(this, KeyInformationActivity.class);
			i.setAction(action);
			
			i.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)endpoint);
			i.putExtra(EXTRA_SESSION, session);
			i.putExtra(EXTRA_PROTOCOL, protocol);
			i.putExtra(EXTRA_DATA, intent.getStringExtra(EXTRA_DATA));
			
			i.setData(Uri.parse(endpoint.toString()));
			builder.setContentIntent(PendingIntent.getActivity(this, 0, i, PendingIntent.FLAG_UPDATE_CURRENT));
			
			notificationManager.notify(session, builder.build());
		}
		else if (ACTION_ERROR.equals(action))
		{
			builder.setContentTitle(getString(R.string.notification_error));
			builder.setContentText(getString(R.string.notification_error_text));

			Intent i = new Intent(this, KeyInformationActivity.class);
			i.setAction(action);
			
			i.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)endpoint);
			i.putExtra(EXTRA_SESSION, session);
			i.putExtra(EXTRA_PROTOCOL, protocol);
			
			i.setData(Uri.parse(endpoint.toString()));
			builder.setContentIntent(PendingIntent.getActivity(this, 0, i, PendingIntent.FLAG_UPDATE_CURRENT));
			
			notificationManager.notify(session, builder.build());
		}
	}
}
