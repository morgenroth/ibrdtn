package de.tubs.ibr.dtn.keyexchange;

import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.service.DaemonService;
import android.app.Activity;
import android.content.Intent;
import android.nfc.NdefMessage;
import android.nfc.NfcAdapter;
import android.os.Parcelable;

public class NFCReceiver extends Activity {
	
	@Override
	protected void onResume() {
		super.onResume();
		if (NfcAdapter.ACTION_NDEF_DISCOVERED.equals(getIntent().getAction())) {
			processIntent(getIntent());
		}
		finish();
	}

	@Override
	protected void onNewIntent(Intent intent) {
		setIntent(intent);
	}
	
	/**
	 * Parses the NDEF Message from the intent
	 */
	void processIntent(Intent intent) {
		Parcelable[] rawMsgs = intent.getParcelableArrayExtra(NfcAdapter.EXTRA_NDEF_MESSAGES);
		// only one message sent during the beam
		NdefMessage ndefMessage = (NdefMessage) rawMsgs[0];
		// record 0 contains the MIME type, record 1 is the AAR, if present
		String rawMessage = new String(ndefMessage.getRecords()[0].getPayload());

		String[] messageArray = rawMessage.split("_NFC_SEPERATOR_");
		
		SingletonEndpoint endpoint = new SingletonEndpoint(messageArray[0]);
		String key = messageArray[1];
		
		Intent startIntent = new Intent(this, DaemonService.class);
		startIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_GIVE_NFC_RESPONSE);
		startIntent.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)endpoint);
		startIntent.putExtra(KeyExchangeService.EXTRA_DATA, key);
		
		startService(startIntent);
	}

}
