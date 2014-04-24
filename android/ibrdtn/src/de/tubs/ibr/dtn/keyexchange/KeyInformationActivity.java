package de.tubs.ibr.dtn.keyexchange;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.widget.ImageView;
import android.widget.TextView;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.WriterException;
import com.jwetherell.quick_response_code.data.Contents;
import com.jwetherell.quick_response_code.qrcode.QRCodeEncoder;

import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.service.DaemonService;

public class KeyInformationActivity extends Activity {

	private static final String TAG = "KeyInformationActivity";
	
	private KeyExchangeManager mManager = null;
	
	private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, IBinder s) {
        	mManager = (KeyExchangeManager)s;
    		refreshView();
            if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service connected");
        }

        public void onServiceDisconnected(ComponentName name) {
        	mManager = null;
            if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service disconnected");
        }
    };
    
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		setContentView(R.layout.key_information_activity);
	}
	
	private void refreshView() {
		try {
			SingletonEndpoint endpoint = mManager.getEndpoint();
			Bundle keyinfo = mManager.getKeyInfo(endpoint);
			
			TextView textViewEID = (TextView) findViewById(R.id.textViewEID);
			textViewEID.setText(endpoint.toString());
			
			ImageView qrCode = (ImageView) findViewById(R.id.imageViewQRCode);
			
			String fingerprint = keyinfo.getString("fingerprint");
			
			QRCodeEncoder qrCodeEncoder = new QRCodeEncoder(fingerprint, null, Contents.Type.TEXT, BarcodeFormat.QR_CODE.toString(), 500);

			Bitmap bitmap = qrCodeEncoder.encodeAsBitmap();
			qrCode.setImageBitmap(bitmap);
			
			TextView textViewFingerprint = (TextView) findViewById(R.id.textViewFingerprint);
			textViewFingerprint.setText(Utils.hashToReadableString(fingerprint));
		} catch (WriterException e) {
			Log.e(TAG, "", e);
		} catch (RemoteException e) {
			Log.e(TAG, "", e);
		}
	}
	
	@Override
    public void onPause() {
        // Detach our existing connection.
        unbindService(mConnection);
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();

        // Establish a connection with the service. We use an explicit
        // class name because we want a specific service implementation that
        // we know will be running in our own process (and thus won't be
        // supporting component replacement by other applications).
        Intent bindIntent = DaemonService.createKeyExchangeManagerIntent(this);
        bindService(bindIntent, mConnection, Context.BIND_AUTO_CREATE);
    }

}
