package de.tubs.ibr.dtn.keyexchange;

import android.annotation.TargetApi;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentTransaction;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.SingletonEndpoint;

@TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH)
public class KeyInformationActivity extends FragmentActivity {
	
	public static final String EXTRA_IS_LOCAL = "isLocal";
	
	private Boolean mIsLocal;
	private SingletonEndpoint mEndpoint;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		setContentView(R.layout.key_information_activity);
		
		mIsLocal = getIntent().getBooleanExtra(KeyInformationActivity.EXTRA_IS_LOCAL, false);
		mEndpoint = getIntent().getParcelableExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT);
		
		if (mEndpoint == null) {
			throw new IllegalArgumentException("endpoint is not set!");
		}
		
		Fragment f = getSupportFragmentManager().findFragmentById(R.id.keyExchange);
		if(f == null) {
			FragmentTransaction ft = getSupportFragmentManager().beginTransaction();
			ft.add(R.id.keyExchange, KeyInformationFragment.create(mEndpoint, mIsLocal)).commit();
			
			// change to fragment for the current state
			changeToFragment(getIntent());
		}
	}
	
	private void changeToFragment(Intent intent) {
		FragmentTransaction ft = getSupportFragmentManager().beginTransaction();
		ft.setCustomAnimations(R.anim.slide_in_left, R.anim.slide_out_right, R.anim.slide_in_right, R.anim.slide_out_left);
		
		if (KeyExchangeService.ACTION_COMPLETE.equals(intent.getAction())) {
			ft.replace(R.id.keyExchange, KeyExchangeResultFragment.create(intent.getAction(), mEndpoint)).addToBackStack(null);
		}
		else if (KeyExchangeService.ACTION_ERROR.equals(intent.getAction())) {
			ft.replace(R.id.keyExchange, KeyExchangeResultFragment.create(intent.getAction(), mEndpoint)).addToBackStack(null);
		}
		else if (KeyExchangeService.ACTION_PASSWORD_REQUEST.equals(intent.getAction())) {
			// get the session ID
			int session = intent.getIntExtra(KeyExchangeService.EXTRA_SESSION, -1);
			ft.replace(R.id.keyExchange, KeyExchangeFragment.create(mEndpoint, EnumProtocol.JPAKE_PROMPT, session)).addToBackStack(null);
		}
		else if (KeyExchangeService.ACTION_HASH_COMMIT.equals(intent.getAction())) {
			// get the session ID
			int session = intent.getIntExtra(KeyExchangeService.EXTRA_SESSION, -1);
			String data = intent.getStringExtra(KeyExchangeService.EXTRA_DATA);
			ft.replace(R.id.keyExchange, HashCommitFragment.create(mEndpoint, session, data)).addToBackStack(null);
		}
		else if (KeyExchangeService.ACTION_NEW_KEY.equals(intent.getAction())) {
			// get the session ID
			int session = intent.getIntExtra(KeyExchangeService.EXTRA_SESSION, -1);
			int protocol = intent.getIntExtra(KeyExchangeService.EXTRA_PROTOCOL, -1);
			String data = intent.getStringExtra(KeyExchangeService.EXTRA_DATA);
			
			ft.replace(R.id.keyExchange, NewKeyFragment.create(mEndpoint, protocol, session, data)).addToBackStack(null);
		}
		ft.commit();
	}

	@Override
	public void onBackPressed() {
		while (getSupportFragmentManager().getBackStackEntryCount() > 1) {
			getSupportFragmentManager().popBackStack();
			getSupportFragmentManager().executePendingTransactions();
		}
		super.onBackPressed();
	}
}
