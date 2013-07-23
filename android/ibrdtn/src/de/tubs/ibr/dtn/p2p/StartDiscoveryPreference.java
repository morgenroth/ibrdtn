package de.tubs.ibr.dtn.p2p;

import de.tubs.ibr.dtn.p2p.scheduler.*;
import android.content.*;
import android.preference.*;
import android.util.*;

public class StartDiscoveryPreference extends Preference {

    public StartDiscoveryPreference(Context context) {
        super(context);
    }

    public StartDiscoveryPreference(Context context, AttributeSet attrs,
            int defStyle) {
        super(context, attrs, defStyle);
    }

    public StartDiscoveryPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void onClick() {
        Intent i = new Intent(getContext(), SchedulerService.class);
        i.setAction(AlarmReceiver.ACTION);
        getContext().startService(i);
        super.onClick();
    }

}
