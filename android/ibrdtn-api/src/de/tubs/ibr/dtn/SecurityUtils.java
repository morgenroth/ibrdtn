package de.tubs.ibr.dtn;

import android.app.PendingIntent;
import android.content.Intent;
import android.os.RemoteException;

public class SecurityUtils {
	/**
	 * Security related intents
	 */
	public static final String ACTION_INFO_ACTIVITY = "de.tubs.ibr.dtn.security.ACTION_INFO_ACTIVITY";
	public static final String ACTION_INFO_DATA = "de.tubs.ibr.dtn.security.ACTION_INFO_DATA";
	
	public static final String EXTRA_ACTION_ICON = "de.tubs.ibr.dtn.security.EXTRA_ACTION_ICON";
	public static final String EXTRA_TRUST_LEVEL = "trustlevel";
	
	public static PendingIntent getSecurityInfoIntent(SecurityService service, String endpoint) {
		try {
			if (service == null) return null;

			Intent i = new Intent(ACTION_INFO_ACTIVITY);
			
			// add singleton endpoint
			if (endpoint != null) {
				i.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, endpoint);
			}
			
			// execute the request
			android.os.Bundle ret = service.execute(i);
			
			// extract intent from result
			return ret.getParcelable(de.tubs.ibr.dtn.Intent.EXTRA_PENDING_INTENT);
		} catch (RemoteException e) {
			return null;
		}
	}
	
	public static android.os.Bundle getSecurityInfo(SecurityService service, String endpoint) {
		try {
			if (service == null) return null;
			
			Intent i = new Intent(ACTION_INFO_DATA);
			
			// add singleton endpoint
			if (endpoint != null) {
				i.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, endpoint);
			}
			
			// execute the request
			return service.execute(i);
		} catch (RemoteException e) {
			return null;
		}
	}
}
