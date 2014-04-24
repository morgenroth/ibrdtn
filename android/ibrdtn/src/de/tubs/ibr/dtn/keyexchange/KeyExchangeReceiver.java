package de.tubs.ibr.dtn.keyexchange;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class KeyExchangeReceiver extends BroadcastReceiver {

	@Override
	public void onReceive(Context context, Intent i) {
		// generate an intent to start the KeyExchangeService
		Intent servIntent = new Intent(context, KeyExchangeService.class);
		
		if (KeyExchangeService.INTENT_KEY_EXCHANGE.equals(i.getAction()))
		{
			// get action of the key-exchange event
			String action = i.getStringExtra(KeyExchangeService.EXTRA_ACTION);
			
			// stop here if the action is empty
			if (action.isEmpty()) return;
			
			// set intent action
			servIntent.setAction(action);

			// copy extras to new intent
			servIntent.putExtras(i.getExtras());
		}
		else if (KeyExchangeService.INTENT_NFC_EXCHANGE.equals(i.getAction()))
		{
			// get name of the key-exchange event
			String name = i.getStringExtra(KeyExchangeService.EXTRA_NAME);
			
			// stop here if the name is empty
			if (name.isEmpty()) return;
			
			// set intent action
			servIntent.setAction(name);

			// copy extras to new intent
			servIntent.putExtra("protocol", EnumProtocol.NFC.getValue());
		}
		else {
			return;
		}
		
		// start the KeyExchangeService
		context.startService(servIntent);
	}

}
