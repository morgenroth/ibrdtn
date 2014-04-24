package de.tubs.ibr.dtn.keyexchange;

import android.annotation.TargetApi;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Build;
import android.os.Bundle;
import android.os.Parcelable;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentTransaction;
import android.widget.Toast;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.SingletonEndpoint;

@TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH)
public class ProtocolSelectionActivity extends FragmentActivity {
	
	private SingletonEndpoint mEndpoint;
	
	@Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        setContentView(R.layout.protocol_selection_activity);
        
        mEndpoint = getIntent().getParcelableExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT);
        
        if (mEndpoint == null) {
        	throw new IllegalArgumentException("endpoint is not set!");
        }
        
        Fragment f = getSupportFragmentManager().findFragmentById(R.id.keyExchange);
        if(f == null) {
	        FragmentTransaction ft = getSupportFragmentManager().beginTransaction();
	        ft.add(R.id.keyExchange, ProtocolSelectionListFragment.create(mEndpoint));
	        ft.commit();
	        changeToFragment(getIntent());
        }
    }
	
	private void changeToFragment(Intent intent) {
		FragmentTransaction f = getSupportFragmentManager().beginTransaction();
        f.replace(R.id.keyExchange, ProtocolSelectionListFragment.create(mEndpoint));
        f.commit();

	    if (KeyExchangeService.ACTION_COMPLETE.equals(intent.getAction())) {
			FragmentTransaction ft = getSupportFragmentManager().beginTransaction();
	    	ft.setCustomAnimations(R.anim.slide_in_left, R.anim.slide_out_right, R.anim.slide_in_right, R.anim.slide_out_left);
	        ft.replace(R.id.keyExchange, new SuccessFragment()).addToBackStack(null);
	        ft.commit();
		}
	    else if (KeyExchangeService.ACTION_PASSWORD_REQUEST.equals(intent.getAction())) {
	    	FragmentTransaction ft = getSupportFragmentManager().beginTransaction();
	    	ft.setCustomAnimations(R.anim.slide_in_left, R.anim.slide_out_right, R.anim.slide_in_right, R.anim.slide_out_left);
	        ft.replace(R.id.keyExchange, KeyExchangeFragment.create(intent, mEndpoint, EnumProtocol.JPAKE_PROMPT)).addToBackStack(null);
	        ft.commit();
	    }
	    else if (KeyExchangeService.ACTION_HASH_COMMIT.equals(intent.getAction())) {
	    	FragmentTransaction ft = getSupportFragmentManager().beginTransaction();
	    	ft.setCustomAnimations(R.anim.slide_in_left, R.anim.slide_out_right, R.anim.slide_in_right, R.anim.slide_out_left);
	        ft.replace(R.id.keyExchange, new HashCommitFragment()).addToBackStack(null);
	        ft.commit();
	    }
	    else if (KeyExchangeService.ACTION_NEW_KEY.equals(intent.getAction())) {
			FragmentTransaction ft = getSupportFragmentManager().beginTransaction();
	    	ft.setCustomAnimations(R.anim.slide_in_left, R.anim.slide_out_right, R.anim.slide_in_right, R.anim.slide_out_left);
	        ft.replace(R.id.keyExchange, NewKeyFragment.create(intent, mEndpoint)).addToBackStack(null);
	        ft.commit();
        }
	}
	
	@Override
    public void onResume() {
        super.onResume();
		
		IntentFilter intentFilter = new IntentFilter(KeyExchangeService.INTENT_KEY_EXCHANGE);
		intentFilter.setPriority(1000);
		intentFilter.addCategory(Intent.CATEGORY_DEFAULT);
		registerReceiver(keyExchangeReceiver, intentFilter);
    }
	
	@Override
	protected void onPause() {
		unregisterReceiver(keyExchangeReceiver);
		super.onPause();
	}

	@Override
    public void onNewIntent(Intent intent) {
        // onResume gets called after this to handle the intent
		intent.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)mEndpoint);

        setIntent(intent);
        
        changeToFragment(intent);
    }

	@Override
	public void onBackPressed() {
		while(getSupportFragmentManager().getBackStackEntryCount() > 1) {
			getSupportFragmentManager().popBackStack();
			getSupportFragmentManager().executePendingTransactions();
		}
		getIntent().putExtra("protocol", -1);
		super.onBackPressed();
	}

	private BroadcastReceiver keyExchangeReceiver = new BroadcastReceiver() {
		
		@Override
		public void onReceive(Context context, Intent intent) {
			// get the action
			String action = intent.getStringExtra(KeyExchangeService.EXTRA_ACTION);
			
			// get the endpoint
			SingletonEndpoint endpoint = new SingletonEndpoint(intent.getStringExtra(KeyExchangeService.EXTRA_ENDPOINT));
			
			// stop here if the currently viewed endpoint does not match the event
			if (!mEndpoint.equals(endpoint)) return;
			
			// refresh current fragment if the ProtocolSelectionListFragment is shown
			Fragment f = getSupportFragmentManager().findFragmentById(R.id.keyExchange);
			if (f instanceof ProtocolSelectionListFragment) {
				((ProtocolSelectionListFragment) f).refresh();
			}
			
			// get the session ID
			int session = Integer.valueOf(intent.hasExtra(KeyExchangeService.EXTRA_SESSION) ? intent.getStringExtra(KeyExchangeService.EXTRA_SESSION) : "-1");
			
			// get the protocol ID
			int newProtocol = Integer.valueOf(intent.hasExtra(KeyExchangeService.EXTRA_PROTOCOL) ? intent.getStringExtra(KeyExchangeService.EXTRA_PROTOCOL) : "-1");
			
			// get the active protocol of this activity
			int protocol = getIntent().getIntExtra(KeyExchangeService.EXTRA_PROTOCOL, -1);

			// stop here if the currently viewed protocol does not match the event
			if (protocol != -1 && protocol != newProtocol) return;
			
			// assign the session to this view
			getIntent().putExtra(KeyExchangeService.EXTRA_SESSION, session);
			
			// abort any further broadcasts of this intent
			abortBroadcast();
			
			if (KeyExchangeService.ACTION_PASSWORD_WRONG.equals(action)) {
				Toast.makeText(context, "Sie haben ein falsches Passwort eingegeben.", Toast.LENGTH_SHORT).show();
			}
			else if (KeyExchangeService.ACTION_PASSWORD_REQUEST.equals(action)) {
				FragmentTransaction ft = getSupportFragmentManager().beginTransaction();
				ft.setCustomAnimations(R.anim.slide_in_left, R.anim.slide_out_right, R.anim.slide_in_right, R.anim.slide_out_left);
		        ft.replace(R.id.keyExchange, KeyExchangeFragment.create(getIntent(), mEndpoint, EnumProtocol.JPAKE_PROMPT)).addToBackStack(null);
		        ft.commit();
			}
			else if (KeyExchangeService.ACTION_HASH_COMMIT.equals(action)) {
				getIntent().putExtra("data", intent.getStringExtra("data"));
				
				FragmentTransaction ft = getSupportFragmentManager().beginTransaction();
				ft.setCustomAnimations(R.anim.slide_in_left, R.anim.slide_out_right, R.anim.slide_in_right, R.anim.slide_out_left);
		        ft.replace(R.id.keyExchange, new HashCommitFragment(), "Commit").addToBackStack(null);
		        ft.commit();
			}
			else if(KeyExchangeService.ACTION_NEW_KEY.equals(action)) {
				getIntent().putExtra("data", intent.getStringExtra("data"));
				
				FragmentTransaction ft = getSupportFragmentManager().beginTransaction();
		    	ft.setCustomAnimations(R.anim.slide_in_left, R.anim.slide_out_right, R.anim.slide_in_right, R.anim.slide_out_left);
		        ft.replace(R.id.keyExchange, NewKeyFragment.create(getIntent(), mEndpoint)).addToBackStack(null);
		        ft.commit();
			}
			else if(KeyExchangeService.ACTION_ERROR.equals(action)) {
				Toast.makeText(context, "Es ist ein Fehler aufgetreten.", Toast.LENGTH_SHORT).show();
			}
			else if (KeyExchangeService.ACTION_COMPLETE.equals(action)) {
				FragmentTransaction ft = getSupportFragmentManager().beginTransaction();
		    	ft.setCustomAnimations(R.anim.slide_in_left, R.anim.slide_out_right, R.anim.slide_in_right, R.anim.slide_out_left);
		        ft.replace(R.id.keyExchange, new SuccessFragment()).addToBackStack(null);
		        ft.commit();
			}
		}
	};
}
