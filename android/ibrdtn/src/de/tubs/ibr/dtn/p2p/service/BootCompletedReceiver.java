package de.tubs.ibr.dtn.p2p.service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import de.tubs.ibr.dtn.p2p.scheduler.AlarmReceiver;

public class BootCompletedReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        Intent i = new Intent(context, AlarmReceiver.class);
        i.setAction(AlarmReceiver.ACTION);
//        context.startService(i);
    }

}
