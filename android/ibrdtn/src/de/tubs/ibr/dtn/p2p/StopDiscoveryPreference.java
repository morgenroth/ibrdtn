package de.tubs.ibr.dtn.p2p;

import android.content.Context;
import android.content.Intent;
import android.preference.Preference;
import android.util.AttributeSet;
import de.tubs.ibr.dtn.p2p.scheduler.SchedulerService;

public class StopDiscoveryPreference extends Preference {

    public StopDiscoveryPreference(Context context, AttributeSet attrs,
            int defStyle) {
        super(context, attrs, defStyle);
    }

    public StopDiscoveryPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public StopDiscoveryPreference(Context context) {
        super(context);
    }

    @Override
    protected void onClick() {
        Intent i = new Intent(getContext(), SchedulerService.class);
        i.setAction(SchedulerService.STOP_SCHEDULER_ACTION);
        getContext().startService(i);
        super.onClick();
    }

}
