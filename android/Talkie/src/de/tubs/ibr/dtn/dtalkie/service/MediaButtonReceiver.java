package de.tubs.ibr.dtn.dtalkie.service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class MediaButtonReceiver extends BroadcastReceiver {
    
    @Override
    public void onReceive(Context context, Intent intent) {
        Intent i = new Intent(context, HeadsetService.class);
        i.setAction(HeadsetService.MEDIA_BUTTON_PRESSED);
        i.putExtras(intent.getExtras());
        context.startService(i);
    }

}
