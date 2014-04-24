package de.tubs.ibr.dtn.keyexchange;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Parcelable;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.ListView;
import android.widget.Toast;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.service.DaemonService;

public class HashCommitFragment extends Fragment {
	
	private SingletonEndpoint mEndpoint = null;
	private int mSession;
	private int mPosition;

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.hash_commit_fragment, container, false);
		
		final Activity activity = getActivity();
		final Intent intent = activity.getIntent();
		
		mEndpoint = intent.getParcelableExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT);
		mSession = intent.getIntExtra("session", -1);
        
        HashCommitListAdapter adapter = new HashCommitListAdapter(activity);
        
        try {
			MessageDigest md = MessageDigest.getInstance("SHA-256");
			byte[] messageDigest = md.digest(intent.getStringExtra("data").getBytes());
			
			int random = (int)Math.floor(Math.random() * 2.99);
			for(int j = 0; j < 3; j++) {
				if(j == random) {
					adapter.add(messageDigest);
					mPosition = j;
				}
				else {
					adapter.add(md.digest(("" + Math.random()).getBytes()));
				}
			}
		} catch (NoSuchAlgorithmException e) {
			e.printStackTrace();
		}
        
        ListView lw = (ListView) v.findViewById(R.id.listViewHash);
        lw.setAdapter(adapter);
        
        lw.setOnItemClickListener(new OnItemClickListener() {

			@Override
			public void onItemClick(AdapterView<?> ad, View v, int position, long id) {
				Intent startIntent = new Intent(getActivity(), DaemonService.class);
				startIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_GIVE_HASH_RESPONSE);
				startIntent.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)mEndpoint);
				startIntent.putExtra("session", mSession);
				startIntent.putExtra("equals", (mPosition != position) ? 0 : 1);
				
				getActivity().startService(startIntent);
				
				getFragmentManager().popBackStack();
				
				if(mPosition != position) {
					Toast.makeText(activity, getString(R.string.hash_wrong_combination), Toast.LENGTH_SHORT).show();
				}
			}
		});
        
        Button buttonNoMatch = (Button) v.findViewById(R.id.buttonNoMatch);
        buttonNoMatch.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				Intent startIntent = new Intent(getActivity(), DaemonService.class);
				startIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_GIVE_HASH_RESPONSE);
				startIntent.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)mEndpoint);
				startIntent.putExtra("session", mSession);
				startIntent.putExtra("equals", 0);
				
				getActivity().startService(startIntent);
				
				getFragmentManager().popBackStack();
			}
		});
        
		return v;
    }
}
