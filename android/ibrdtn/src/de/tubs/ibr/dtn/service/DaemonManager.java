
package de.tubs.ibr.dtn.service;

import android.app.Service;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.IBinder;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import de.tubs.ibr.dtn.DtnManager;
import de.tubs.ibr.dtn.Services;
import de.tubs.ibr.dtn.daemon.Preferences;

public class DaemonManager extends Service {

	private final DtnManager.Stub mBinder = new DtnManager.Stub() {
		@Override
		public void setDtnEnabled(boolean state) throws RemoteException {
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(DaemonManager.this);
			prefs.edit().putBoolean(Preferences.KEY_ENABLED, state).commit();

			/**
			 * Forward preference change to the DTN service
			 */
			final Intent prefChangedIntent = new Intent(DaemonManager.this, DaemonService.class);
			prefChangedIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_PREFERENCE_CHANGED);
			prefChangedIntent.putExtra("prefkey", Preferences.KEY_ENABLED);
			prefChangedIntent.putExtra(Preferences.KEY_ENABLED, prefs.getBoolean(Preferences.KEY_ENABLED, false));
			startService(prefChangedIntent);

			if (state) {
				// startup the DTN service
				final Intent intent = new Intent(DaemonManager.this, DaemonService.class);
				intent.setAction(DaemonService.ACTION_STARTUP);
				startService(intent);
			} else {
				// shutdown the DTN service
				final Intent intent = new Intent(DaemonManager.this, DaemonService.class);
				intent.setAction(DaemonService.ACTION_SHUTDOWN);
				startService(intent);
			}
		}

		@Override
		public boolean isDtnEnabled() throws RemoteException {
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(DaemonManager.this);
			return prefs.getBoolean(Preferences.KEY_ENABLED, true);
		}
	};

	@Override
	public IBinder onBind(Intent intent) {
		if (Services.SERVICE_MANAGER.match(intent)) {
			return mBinder;
		} else {
			return null;
		}
	}
}
