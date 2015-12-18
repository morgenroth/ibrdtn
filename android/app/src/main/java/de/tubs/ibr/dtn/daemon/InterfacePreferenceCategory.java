package de.tubs.ibr.dtn.daemon;

import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Collections;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.CheckBoxPreference;
import android.preference.PreferenceCategory;
import android.preference.PreferenceManager;
import android.util.AttributeSet;
import android.util.Log;

public class InterfacePreferenceCategory extends PreferenceCategory {
    
    private static final String TAG = "IfPreferenceCategory";
    
    public InterfacePreferenceCategory(Context context) {
        super(context);
    }
    
    public InterfacePreferenceCategory(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public InterfacePreferenceCategory(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    public void updateInterfaceList() {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getContext());
        
        try {
            // clear all preferences
            removeAll();
            
            ArrayList<NetworkInterface> ilist = Collections.list( NetworkInterface.getNetworkInterfaces() );

            // scan for known network devices
            for (NetworkInterface i : ilist)
            {
                String iface = i.getDisplayName();
                String iface_key = "interface_" + iface;
    
                // skip virtual interfaces
                if (i.isVirtual()) continue;
                
                // do not work on non-multicast interfaces
                if (!i.supportsMulticast()) continue;
                
                // skip loopback device
                if (i.isLoopback()) continue;
                
                // skip rmnet
                if (i.getDisplayName().startsWith("rmnet")) continue;
                if (i.getDisplayName().startsWith("rev_rmnet")) continue;
                
                // p2p groups
                if (i.getDisplayName().equals("p2p0")) continue;
                if (i.getDisplayName().startsWith("p2p-")) continue;
                
                if (    iface.contains("wlan") ||
                        iface.contains("wifi") ||
                        iface.contains("eth")
                    )
                {
                    // add default selection
                    if (!prefs.contains(iface_key)) {
                        prefs.edit().putBoolean(iface_key, true).commit();
                    }
                }
                
                // create a new checkbox preference
                CheckBoxPreference cb_i = new CheckBoxPreference(getContext());
                
                // set the preference title
                cb_i.setTitle(iface);
                
                // add hint if point-to-point
                if (i.isPointToPoint())
                {
                    cb_i.setSummary("Point-to-Point");
                }
                
                // set preference key
                cb_i.setKey("interface_" + iface);
                
                // add preference
                addPreference(cb_i);
            }
        } catch (SocketException e) {
            Log.e(TAG, null, e);
        }
    }
}
