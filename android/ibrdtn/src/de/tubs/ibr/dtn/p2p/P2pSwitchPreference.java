package de.tubs.ibr.dtn.p2p;

import android.annotation.TargetApi;
import android.content.Context;
import android.content.Intent;
import android.preference.SwitchPreference;
import android.util.AttributeSet;

@TargetApi(14)
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
    protected void onClick() {
        Intent intent = new Intent(getContext(), P2pSettingsActivity.class);
        getContext().startActivity(intent);
    }
    
    

}
