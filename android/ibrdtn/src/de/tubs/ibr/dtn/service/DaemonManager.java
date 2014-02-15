package de.tubs.ibr.dtn.service;

import android.app.Service;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.IBinder;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import de.tubs.ibr.dtn.DtnManager;
import de.tubs.ibr.dtn.daemon.Preferences;

public class DaemonManager extends Service {
    
    private final DtnManager.Stub mBinder = new DtnManager.Stub() {
        @Override
        public void setDtnEnabled(boolean state) throws RemoteException {
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(DaemonManager.this);
            prefs.edit().putBoolean(Preferences.KEY_ENABLED, state).commit();
            
            if (state) {
                // start the DTN service
                Intent is = new Intent(DaemonManager.this, DaemonService.class);
                is.setAction(DaemonService.ACTION_STARTUP);
                startService(is);
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
        return mBinder;
    }
}
