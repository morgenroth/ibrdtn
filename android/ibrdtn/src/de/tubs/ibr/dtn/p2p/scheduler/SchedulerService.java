package de.tubs.ibr.dtn.p2p.scheduler;

import java.text.DateFormat;
import java.util.Date;

import android.app.AlarmManager;
import android.app.IntentService;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import de.tubs.ibr.dtn.p2p.P2pManager;
import de.tubs.ibr.dtn.p2p.SettingsUtil;

public class SchedulerService extends IntentService {

    private static final String TAG = "SchedulerService";

    public final static Integer RC_CHECK_STATE = 9725;

    public final static String ACTION_CHECK_STATE = "de.tubs.ibr.dtn.p2p.action.CHECK_STATE";
    
    public static final String ACTION_ACTIVATE_SCHEDULER = "de.tubs.ibr.dtn.p2p.action.ACTIVATE_SCHEDULER";
    public static final String ACTION_DEACTIVATE_SCHEDULER = "de.tubs.ibr.dtn.p2p.action.DEACTIVATE_SCHEDULER";

    public SchedulerService() {
        super("SchedulerService");
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        String action = intent.getAction();

        if (ACTION_CHECK_STATE.equals(action)) {
            checkState();
        } else if (ACTION_ACTIVATE_SCHEDULER.equals(action)) {
            checkState();
        } else if (ACTION_DEACTIVATE_SCHEDULER.equals(action)) {
            stopScheduler();
        }
    }

    private void stopScheduler() {
        // get the alarm manager
        AlarmManager am = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
        
        // create an intent to check the state within the scheduler service
        Intent i = new Intent(this, SchedulerService.class);
        i.setAction(ACTION_CHECK_STATE);
        
        // create or get a pending intent
        PendingIntent pi = PendingIntent.getService(this, RC_CHECK_STATE, i, PendingIntent.FLAG_CANCEL_CURRENT);
        
        // cancel the pending intent
        am.cancel(pi);
        pi.cancel();
        
        // clear the scheduler info
        SettingsUtil.setSchedulerInfo(this, "");
        SettingsUtil.setNextScheduledCheck(this, 0);
        
        // debug
        Log.d(TAG, "Scheduler stopped");
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
            Intent i = new Intent(slot.getS() == State.ON ? P2pManager.START_DISCOVERY_ACTION : P2pManager.STOP_DISCOVERY_ACTION);
            sendBroadcast(i);
            SettingsUtil.setNextScheduledCheck(this, nextCheck);
            SettingsUtil.setSchedulerInfo(this, slot.getInfo());
            SchedulerService.setNextScheduledCheck(this, nextCheck);
            Log.d(TAG, "State: " + slot.getS() + " Next Check:"
                    + DateFormat.getTimeInstance().format(new Date(nextCheck)));
        } else {
            Intent i = new Intent(s.getCurrentState(schedulerInfo) == State.ON ? P2pManager.START_DISCOVERY_ACTION : P2pManager.STOP_DISCOVERY_ACTION);
            sendBroadcast(i);
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
        // get the alarm manager
        AlarmManager am = (AlarmManager)context.getSystemService(Context.ALARM_SERVICE);
        
        // create an intent to check the state within the scheduler service
        Intent i = new Intent(context, SchedulerService.class);
        i.setAction(ACTION_CHECK_STATE);
        
        // create or get a pending intent
        PendingIntent pi = PendingIntent.getService(context, RC_CHECK_STATE, i, PendingIntent.FLAG_CANCEL_CURRENT);
        
        // schedule the intent for later execution
        am.set(AlarmManager.RTC_WAKEUP, nextCheck, pi);
    }

}
