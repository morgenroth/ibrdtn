package de.tubs.ibr.dtn.p2p;

import android.annotation.TargetApi;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.preference.SwitchPreference;
import android.util.AttributeSet;

@TargetApi(16)
public class P2pSwitchPreference extends SwitchPreference {

    public P2pSwitchPreference(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    public P2pSwitchPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public P2pSwitchPreference(Context context) {
        super(context);
    }

    @Override
    protected void onAttachedToHierarchy(PreferenceManager preferenceManager) {
        super.onAttachedToHierarchy(preferenceManager);
     
        // register as preference change listener
        PreferenceManager.getDefaultSharedPreferences(getContext()).registerOnSharedPreferenceChangeListener(mPrefChangeListener);
    }

    @Override
    protected void onPrepareForRemoval() {
        // un-register as preference change listener
        PreferenceManager.getDefaultSharedPreferences(getContext()).unregisterOnSharedPreferenceChangeListener(mPrefChangeListener);
        
        super.onPrepareForRemoval();
    }

    @Override
    protected void onClick() {
        Intent intent = new Intent(getContext(), P2pSettingsActivity.class);
        getContext().startActivity(intent);
    }
    
    private SharedPreferences.OnSharedPreferenceChangeListener mPrefChangeListener = new SharedPreferences.OnSharedPreferenceChangeListener() {
        @Override
        public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
            // adjust P2P switch
            if (SettingsUtil.KEY_P2P_ENABLED.equals(key)) {
                P2pSwitchPreference.this.setChecked(prefs.getBoolean(key, false));
            }
        }
    };
}
