package de.tubs.ibr.dtn.p2p.service;

import android.content.*;
import de.tubs.ibr.dtn.p2p.scheduler.*;

public class BootCompletedReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        Intent i = new Intent(context, AlarmReceiver.class);
        i.setAction(AlarmReceiver.ACTION);
//        context.startService(i);
    }

}
