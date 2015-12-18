package de.tubs.ibr.dtn.keyexchange;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Parcelable;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.text.InputType;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.google.zxing.integration.android.IntentIntegrator;
import com.google.zxing.integration.android.IntentResult;

import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.service.DaemonService;

public class KeyExchangeFragment extends Fragment {

	private static final String TAG = "KeyExchangeFragment";
	
	private SingletonEndpoint mEndpoint;
	private int mSession;
	private EnumProtocol mProtocol;
	
	// GUI elements
	private TextView mInfoText = null;
	private EditText mEditTextPassword = null;
	private CheckBox mCheckVisiblePassword = null;
	private Button mButton = null;
	
	public final class FragmentIntentIntegrator extends IntentIntegrator {
		private final Fragment fragment;

		public FragmentIntentIntegrator(Fragment fragment) {
			super(fragment.getActivity());
			this.fragment = fragment;
		}

		@Override
		protected void startActivityForResult(Intent intent, int code) {
			fragment.startActivityForResult(intent, code);
		}
	}
	
	public static KeyExchangeFragment create(SingletonEndpoint endpoint, EnumProtocol protocol, int session) {
		KeyExchangeFragment f = new KeyExchangeFragment();
		Bundle args = new Bundle();
		args.putParcelable(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, endpoint);
		args.putInt(KeyExchangeService.EXTRA_SESSION, session);
		args.putInt(KeyExchangeService.EXTRA_PROTOCOL, protocol.getValue());
		f.setArguments(args);
		return f;
	}
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.key_exchange_fragment, container, false);
		
		mEndpoint = getArguments().getParcelable(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT);
		mSession = getArguments().getInt(KeyExchangeService.EXTRA_SESSION);
		mProtocol = EnumProtocol.valueOf(getArguments().getInt(KeyExchangeService.EXTRA_PROTOCOL));
		
		// get GUI elements
		mInfoText = (TextView) v.findViewById(R.id.textInfo);
		mEditTextPassword = (EditText) v.findViewById(R.id.editTextPassword);
		mCheckVisiblePassword = (CheckBox) v.findViewById(R.id.checkVisiblePassword);
		
		mButton = (Button) v.findViewById(R.id.buttonOk);
		mButton.setOnClickListener(mButtonListener);
		
		mCheckVisiblePassword.setOnCheckedChangeListener(new OnCheckedChangeListener() {
			@Override
			public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
				if (isChecked) {
					mEditTextPassword.setInputType(InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD);
				} else {
					mEditTextPassword.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
				}
			}
		});
		
		switch (mProtocol)
		{
		case NONE:
			mInfoText.setText(getString(R.string.none_pending));
			mEditTextPassword.setVisibility(View.GONE);
			mCheckVisiblePassword.setVisibility(View.GONE);
			mButton.setText(getString(android.R.string.ok));
			break;
			
		case DH:
			mInfoText.setText(getString(R.string.dh_pending));
			mEditTextPassword.setVisibility(View.GONE);
			mCheckVisiblePassword.setVisibility(View.GONE);
			mButton.setText(getString(android.R.string.ok));
			break;
			
		case JPAKE:
			mInfoText.setText(getString(R.string.jpake_pending));
			mEditTextPassword.setVisibility(View.VISIBLE);
			mCheckVisiblePassword.setVisibility(View.VISIBLE);
			mButton.setText(getString(android.R.string.ok));
			break;
			
		case HASH:
			mInfoText.setText(getString(R.string.hash_pending));
			mEditTextPassword.setVisibility(View.GONE);
			mCheckVisiblePassword.setVisibility(View.GONE);
			mButton.setText(getString(android.R.string.ok));
			break;
			
		case QR:
			mInfoText.setText(getString(R.string.qr_scan_hint));
			mEditTextPassword.setVisibility(View.GONE);
			mCheckVisiblePassword.setVisibility(View.GONE);
			mButton.setText(getString(R.string.qr_scan_action));
			break;
			
		case JPAKE_PROMPT:
			mInfoText.setText(getString(R.string.jpake_password_prompt));
			mEditTextPassword.setVisibility(View.VISIBLE);
			mCheckVisiblePassword.setVisibility(View.VISIBLE);
			mButton.setText(getString(android.R.string.ok));
			break;
			
		default:
			mInfoText.setText(getString(R.string.error_hint));
			mEditTextPassword.setVisibility(View.GONE);
			mCheckVisiblePassword.setVisibility(View.GONE);
			mButton.setText(getString(android.R.string.ok));
			break;
		}
		return v;
	}

	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data) {
		Log.d(TAG, "onActivityResult");
		IntentResult scanResult = IntentIntegrator.parseActivityResult(requestCode, resultCode, data);
		if (resultCode == Activity.RESULT_OK && scanResult != null) {
			Intent startIntent = new Intent(getActivity(), DaemonService.class);
			startIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_GIVE_QR_RESPONSE);
			startIntent.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)mEndpoint);
			startIntent.putExtra(KeyExchangeService.EXTRA_DATA, scanResult.getContents());
			getActivity().startService(startIntent);
		}
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
				
			case QR:
				IntentIntegrator integrator = new FragmentIntentIntegrator(KeyExchangeFragment.this);
				integrator.initiateScan();
				break;
				
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

	@Override
	public void onResume() {
		super.onResume();
		
		IntentFilter intentFilter = new IntentFilter(KeyExchangeService.INTENT_KEY_EXCHANGE);
		intentFilter.setPriority(1000);
		intentFilter.addCategory(Intent.CATEGORY_DEFAULT);
		getActivity().registerReceiver(mUpdateReceiver, intentFilter);
	}
	
	@Override
	public void onPause() {
		getActivity().unregisterReceiver(mUpdateReceiver);
		super.onPause();
	}
	
	private BroadcastReceiver mUpdateReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			// get the action
			String action = intent.getStringExtra(KeyExchangeService.EXTRA_ACTION);
			
			// get the endpoint
			SingletonEndpoint endpoint = new SingletonEndpoint(intent.getStringExtra(KeyExchangeService.EXTRA_ENDPOINT));
			
			// stop here if the currently viewed endpoint does not match the event
			if (!mEndpoint.equals(endpoint)) return;
			
			// refresh current fragment if the ProtocolSelectionListFragment is shown
			Fragment f = getActivity().getSupportFragmentManager().findFragmentById(R.id.keyExchange);
			if (f instanceof KeyInformationFragment) {
				((KeyInformationFragment) f).refresh();
			}
			
			// get the session ID
			int session = Integer.valueOf(intent.hasExtra(KeyExchangeService.EXTRA_SESSION) ? intent.getStringExtra(KeyExchangeService.EXTRA_SESSION) : "-1");
			
			// get the protocol ID
			int newProtocol = Integer.valueOf(intent.hasExtra(KeyExchangeService.EXTRA_PROTOCOL) ? intent.getStringExtra(KeyExchangeService.EXTRA_PROTOCOL) : "-1");
			
			// get the active protocol of this activity
			int protocol = getActivity().getIntent().getIntExtra(KeyExchangeService.EXTRA_PROTOCOL, -1);
	
			// stop here if the currently viewed protocol does not match the event
			if (protocol == -1 || protocol != newProtocol) return;
			
			// stop here if the currently viewed session does not match the event
			if (mSession != -1 && mSession != session) return;
			
			// assign the session to this view
			getActivity().getIntent().putExtra(KeyExchangeService.EXTRA_SESSION, session);
			
			// abort any further broadcasts of this intent
			abortBroadcast();
			
			if (KeyExchangeService.ACTION_PASSWORD_WRONG.equals(action)) {
				Toast.makeText(context, "Sie haben ein falsches Passwort eingegeben.", Toast.LENGTH_SHORT).show();
			}
			else if (KeyExchangeService.ACTION_PASSWORD_REQUEST.equals(action)) {
				FragmentTransaction ft = getActivity().getSupportFragmentManager().beginTransaction();
				ft.setCustomAnimations(R.anim.slide_in_left, R.anim.slide_out_right, R.anim.slide_in_right, R.anim.slide_out_left);
				ft.replace(R.id.keyExchange, KeyExchangeFragment.create(mEndpoint, EnumProtocol.JPAKE_PROMPT, session)).addToBackStack(null);
				ft.commit();
			}
			else if (KeyExchangeService.ACTION_HASH_COMMIT.equals(action)) {
				String data = intent.getStringExtra(KeyExchangeService.EXTRA_DATA);
				FragmentTransaction ft = getActivity().getSupportFragmentManager().beginTransaction();
				ft.setCustomAnimations(R.anim.slide_in_left, R.anim.slide_out_right, R.anim.slide_in_right, R.anim.slide_out_left);
				ft.replace(R.id.keyExchange, HashCommitFragment.create(mEndpoint, session, data), "Commit").addToBackStack(null);
				ft.commit();
			}
			else if(KeyExchangeService.ACTION_NEW_KEY.equals(action)) {
				String data = intent.getStringExtra(KeyExchangeService.EXTRA_DATA);
				FragmentTransaction ft = getActivity().getSupportFragmentManager().beginTransaction();
				ft.setCustomAnimations(R.anim.slide_in_left, R.anim.slide_out_right, R.anim.slide_in_right, R.anim.slide_out_left);
				ft.replace(R.id.keyExchange, NewKeyFragment.create(mEndpoint, protocol, session, data)).addToBackStack(null);
				ft.commit();
			}
			else if(KeyExchangeService.ACTION_ERROR.equals(action)) {
				FragmentTransaction ft = getActivity().getSupportFragmentManager().beginTransaction();
				ft.setCustomAnimations(R.anim.slide_in_left, R.anim.slide_out_right, R.anim.slide_in_right, R.anim.slide_out_left);
				ft.replace(R.id.keyExchange, KeyExchangeResultFragment.create(action, mEndpoint)).addToBackStack(null);
				ft.commit();
			}
			else if (KeyExchangeService.ACTION_COMPLETE.equals(action)) {
				FragmentTransaction ft = getActivity().getSupportFragmentManager().beginTransaction();
				ft.setCustomAnimations(R.anim.slide_in_left, R.anim.slide_out_right, R.anim.slide_in_right, R.anim.slide_out_left);
				ft.replace(R.id.keyExchange, KeyExchangeResultFragment.create(action, mEndpoint)).addToBackStack(null);
				ft.commit();
			}
		}
	};
}
