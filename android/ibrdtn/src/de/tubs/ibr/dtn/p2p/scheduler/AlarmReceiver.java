package de.tubs.ibr.dtn.p2p.scheduler;

import android.content.*;

public class AlarmReceiver extends BroadcastReceiver {

    public final static Integer REQUEST_CODE = 9725;

    public final static String ACTION = "de.tubs.ibr.dtn.p2p.action.CHECK_STATE";

    @Override
    public void onReceive(Context context, Intent intent) {
        Intent i = new Intent(context, SchedulerService.class);
        i.setAction(ACTION);
        context.startService(i);
    }

}
