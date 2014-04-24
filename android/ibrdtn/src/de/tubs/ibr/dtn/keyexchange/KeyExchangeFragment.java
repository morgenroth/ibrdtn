package de.tubs.ibr.dtn.keyexchange;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.nfc.NdefMessage;
import android.nfc.NdefRecord;
import android.nfc.NfcAdapter;
import android.nfc.NfcAdapter.CreateNdefMessageCallback;
import android.nfc.NfcEvent;
import android.os.Build;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Parcelable;
import android.os.RemoteException;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.service.DaemonService;

@TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH)
public class KeyExchangeFragment extends Fragment implements CreateNdefMessageCallback {

	private static final String TAG = "KeyExchangeFragment";
	
	private KeyExchangeManager mManager = null;
	
	private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, IBinder s) {
        	mManager = (KeyExchangeManager)s;
            if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service connected");
            
            if (mProtocol == EnumProtocol.QR)
            {
				// check if key exists
				try {
					if (!mManager.hasKey(mEndpoint)) {
						mInfoText.setText(getString(R.string.qr_nokey));
						mButton.setText(getString(android.R.string.ok));
						mProtocol = EnumProtocol.NONE;
						
						Intent startIntent = new Intent(getActivity(), DaemonService.class);
						startIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_START_KEY_EXCHANGE);
						startIntent.putExtra("protocol", EnumProtocol.NONE.getValue());
						startIntent.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)mEndpoint);
						
						getActivity().startService(startIntent);
					}
				} catch (RemoteException e) {
					// error
					Log.e(TAG, "Failed to check if the peer key is available", e);
					getFragmentManager().popBackStack();
				}
            }
        }

        public void onServiceDisconnected(ComponentName name) {
        	mManager = null;
            if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service disconnected");
        }
    };
    
	private NfcAdapter mNfcAdapter;
	private SingletonEndpoint mEndpoint;
	private int mSession;
	private EnumProtocol mProtocol;
	
	// GUI elements
	private TextView mInfoText = null;
	private EditText mEditTextPassword = null;
	private Button mButton = null;
	
	public static KeyExchangeFragment create(Intent intent, SingletonEndpoint endpoint, EnumProtocol protocol) {
		KeyExchangeFragment f = new KeyExchangeFragment();
		Bundle args = new Bundle();
		args.putParcelable(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, endpoint);
		args.putInt(KeyExchangeService.EXTRA_SESSION, intent.getIntExtra("session", -1));
		args.putInt(KeyExchangeService.EXTRA_PROTOCOL, protocol.getValue());
		f.setArguments(args);
		return f;
	}
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.key_exchange_fragment, container, false);
        
		final Activity activity = getActivity();
		
        mEndpoint = getArguments().getParcelable(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT);
        mSession = getArguments().getInt(KeyExchangeService.EXTRA_SESSION);
        mProtocol = EnumProtocol.valueOf(getArguments().getInt(KeyExchangeService.EXTRA_PROTOCOL));
		
        // get GUI elements
        mInfoText = (TextView) v.findViewById(R.id.textInfo);
        mEditTextPassword = (EditText) v.findViewById(R.id.editTextPassword);
        mButton = (Button) v.findViewById(R.id.buttonOk);
		
		switch (mProtocol)
		{
		case NONE:
			mInfoText.setText(getString(R.string.none_pending));
			mButton.setText(getString(android.R.string.ok));
			mButton.setOnClickListener(mButtonListener);
			break;
			
		case DH:
			mInfoText.setText(getString(R.string.dh_pending));
			mButton.setText(getString(android.R.string.ok));
			mButton.setOnClickListener(mButtonListener);
			break;
			
		case JPAKE:
			mInfoText.setText(getString(R.string.jpake_pending));
			mEditTextPassword.setVisibility(View.VISIBLE);
			mButton.setText(getString(android.R.string.ok));
			mButton.setOnClickListener(mButtonListener);
			break;
			
		case HASH:
			mInfoText.setText(getString(R.string.hash_pending));
			mButton.setText(getString(android.R.string.ok));
			mButton.setOnClickListener(mButtonListener);
			break;
			
		case QR:
			mInfoText.setText(getString(R.string.qr_scan_hint));
			mButton.setText(getString(R.string.qr_scan_action));
			mButton.setOnClickListener(mButtonListener);
			break;
			
		case NFC:
			mInfoText.setText(getString(R.string.nfc_hint));
			mButton.setVisibility(View.INVISIBLE);
			
			mNfcAdapter = NfcAdapter.getDefaultAdapter(activity);
			mNfcAdapter.setNdefPushMessageCallback(this, activity);
			break;
			
		case JPAKE_PROMPT:
			mInfoText.setText(getString(R.string.jpake_password_prompt));
			mEditTextPassword.setVisibility(View.VISIBLE);
			mButton.setText(getString(android.R.string.ok));
			mButton.setOnClickListener(mButtonListener);
			break;
			
		default:
			mInfoText.setText(getString(R.string.error_hint));
			mButton.setText(getString(android.R.string.ok));
			mButton.setOnClickListener(mButtonListener);
			break;
		}
		return v;
    }
	
	private View.OnClickListener mButtonListener = new View.OnClickListener() {

		@Override
		public void onClick(View v) {
			switch (mProtocol)
			{
			case NONE:
				getFragmentManager().popBackStack();
				break;
				
			case DH:
				getFragmentManager().popBackStack();
				break;
				
			case JPAKE: {
				InputMethodManager inputManager = (InputMethodManager)getActivity().getSystemService(Context.INPUT_METHOD_SERVICE);
				if (getActivity().getCurrentFocus() != null) {
					inputManager.hideSoftInputFromWindow(getActivity().getCurrentFocus().getWindowToken(), InputMethodManager.HIDE_NOT_ALWAYS);
				}
				
				Intent startIntent = new Intent(getActivity(), DaemonService.class);
				startIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_START_KEY_EXCHANGE);
				startIntent.putExtra(KeyExchangeService.EXTRA_PROTOCOL, EnumProtocol.JPAKE.getValue());
				startIntent.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)mEndpoint);
				startIntent.putExtra(KeyExchangeService.EXTRA_PASSWORD, mEditTextPassword.getText().toString());
				
				getActivity().startService(startIntent);
				
				getFragmentManager().popBackStack();
				break;
			}
				
			case HASH:
				getFragmentManager().popBackStack();
				break;
				
			case QR: {
				getActivity().getIntent().setAction("QR");
				Intent i = new Intent(getActivity(), CaptureActivity.class);
				i.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)mEndpoint);
				startActivity(i);
				break;
			}
				
			case JPAKE_PROMPT: {
				InputMethodManager inputManager = (InputMethodManager)getActivity().getSystemService(Context.INPUT_METHOD_SERVICE);
				if (getActivity().getCurrentFocus() != null) {
					inputManager.hideSoftInputFromWindow(getActivity().getCurrentFocus().getWindowToken(), InputMethodManager.HIDE_NOT_ALWAYS);
				}
				
				Intent startIntent = new Intent(getActivity(), DaemonService.class);
				startIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_GIVE_PASSWORD_RESPONSE);
				startIntent.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)mEndpoint);
				startIntent.putExtra(KeyExchangeService.EXTRA_SESSION, mSession);
				startIntent.putExtra(KeyExchangeService.EXTRA_PASSWORD, mEditTextPassword.getText().toString());
				
				getActivity().startService(startIntent);
				
				getFragmentManager().popBackStack();
				break;
			}
				
			default:
				getFragmentManager().popBackStack();
				break;
			}
		}
	};

	@TargetApi(Build.VERSION_CODES.JELLY_BEAN)
	@Override
	public NdefMessage createNdefMessage(NfcEvent event) {
		String text = "dtn:none";
		
		try {
			SingletonEndpoint endpoint = mManager.getEndpoint();
			text = endpoint.toString();
			text += "_NFC_SEPERATOR_";
			text += mManager.getKeyInfo(endpoint).getString("data");

		} catch (RemoteException e) {
			Log.e(TAG, "failed to get local EID", e);
		}
		
        NdefMessage msg = new NdefMessage(
                new NdefRecord[] { NdefRecord.createMime(
                        "text/plain", text.getBytes())
        });
        return msg;
	}
	
	@Override
	public void onResume() {
		super.onResume();
        
        // Establish a connection with the service. We use an explicit
        // class name because we want a specific service implementation that
        // we know will be running in our own process (and thus won't be
        // supporting component replacement by other applications).
        Intent bindIntent = DaemonService.createKeyExchangeManagerIntent(getActivity());
        getActivity().bindService(bindIntent, mConnection, Context.BIND_AUTO_CREATE);
	}
	
	@Override
	public void onPause() {
        // Detach our existing connection.
        getActivity().unbindService(mConnection);
        
		super.onPause();
	}
}
