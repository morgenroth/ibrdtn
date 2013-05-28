package de.tubs.ibr.dtn.p2p.scheduler;

import java.text.*;
import java.util.*;

import android.app.*;
import android.content.*;
import android.util.*;
import de.tubs.ibr.dtn.p2p.*;
import de.tubs.ibr.dtn.p2p.service.*;

public class SchedulerService extends IntentService {

    private static final String TAG = "SchedulerService";

    public static final String STOP_SCHEDULER_ACTION = "de.tubs.ibr.dtn.p2p.action.STOP_SCHEDULER";

    public SchedulerService() {
        super("SchedulerService");
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        String action = intent.getAction();

        if (AlarmReceiver.ACTION.equals(action)) {
            SchedulerService.this.checkState();
        } else if (STOP_SCHEDULER_ACTION.equals(action)) {
            this.stopScheduler();
        }
    }

    private void stopScheduler() {
        Intent i = new Intent(this, AlarmReceiver.class);
        PendingIntent sender = PendingIntent.getBroadcast(this,
                AlarmReceiver.REQUEST_CODE, i,
                PendingIntent.FLAG_CANCEL_CURRENT);
        AlarmManager am = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
        am.cancel(sender);
        // if (Log.isLoggable(TAG, Log.DEBUG)) {
        Log.d(TAG, "Scheduler stopped");
        // }
    }

    private void checkState() {
        if (!SettingsUtil.isPeerDiscoveryEnabled(this)) {
            return;
        }
        Strategy s = SettingsUtil.getStrategy(this);
        String schedulerInfo = SettingsUtil.getSchedulerInfo(this);
        long nextCheck = SettingsUtil.getNextScheduledCheck(this);
        if (nextCheck < System.currentTimeMillis()) {
            Slot slot = s.getNextSlot(schedulerInfo);
            nextCheck = System.currentTimeMillis() + slot.getDuration() * 1000;
            Intent i = new Intent(this, WiFiP2P4IbrDtnService.class);
            i.setAction(slot.getS() == State.ON ? WiFiP2P4IbrDtnService.START_DISCOVERY_ACTION
                    : WiFiP2P4IbrDtnService.STOP_DISCOVERY_ACTION);
            startService(i);
            SettingsUtil.setNextScheduledCheck(this, nextCheck);
            SettingsUtil.setSchedulerInfo(this, slot.getInfo());
            SchedulerService.setNextScheduledCheck(this, nextCheck);
            Log.d(TAG, "State: " + slot.getS() + " Next Check:"
                    + DateFormat.getTimeInstance().format(new Date(nextCheck)));
        } else {
            Intent i = new Intent(this, WiFiP2P4IbrDtnService.class);
            i.setAction(s.getCurrentState(schedulerInfo) == State.ON ? WiFiP2P4IbrDtnService.START_DISCOVERY_ACTION
                    : WiFiP2P4IbrDtnService.STOP_DISCOVERY_ACTION);
            startService(i);
            SchedulerService.setNextScheduledCheck(this, nextCheck);
            Log.d(TAG,
                    "State: "
                            + s.getCurrentState(schedulerInfo)
                            + " Next Check:"
                            + DateFormat.getTimeInstance().format(
                                    new Date(nextCheck)));
        }
    }

    public static void setNextScheduledCheck(Context context, long nextCheck) {
        Intent intent = new Intent(context, AlarmReceiver.class);
        PendingIntent sender = PendingIntent.getBroadcast(context,
                AlarmReceiver.REQUEST_CODE, intent,
                PendingIntent.FLAG_CANCEL_CURRENT);
        AlarmManager am = (AlarmManager) context
                .getSystemService(Context.ALARM_SERVICE);
        am.set(AlarmManager.RTC_WAKEUP, nextCheck, sender);
    }

}
