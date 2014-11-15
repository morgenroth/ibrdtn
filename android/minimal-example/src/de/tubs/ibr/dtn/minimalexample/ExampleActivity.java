package de.tubs.ibr.dtn.minimalexample;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

public class ExampleActivity extends Activity {
	
	TextView mDestination = null;
	TextView mMessage = null;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_example);
		
		mDestination = (TextView)findViewById(R.id.textDestination);
		mMessage = (TextView)findViewById(R.id.textMessage);
		
		findViewById(R.id.button1).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				// forward message to the DTN service
				Intent i = new Intent(ExampleActivity.this, MyDtnIntentService.class);
				i.setAction(MyDtnIntentService.ACTION_SEND_MESSAGE);
				i.putExtra(MyDtnIntentService.EXTRA_DESTINATION, mDestination.getText().toString());
				i.putExtra(MyDtnIntentService.EXTRA_PAYLOAD, mMessage.getText().toString().getBytes());
				startService(i);
			}
		});
		
		IntentFilter filter = new IntentFilter(MyDtnIntentService.ACTION_RECV_MESSAGE);
		registerReceiver(mReceiver, filter);
	}
	
	@Override
	protected void onDestroy() {
		unregisterReceiver(mReceiver);
		super.onDestroy();
	}

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			if (MyDtnIntentService.ACTION_RECV_MESSAGE.equals(intent.getAction())) {
				mDestination.setText(intent.getStringExtra(MyDtnIntentService.EXTRA_SOURCE));
				Toast.makeText(ExampleActivity.this, new String(intent.getByteArrayExtra(MyDtnIntentService.EXTRA_PAYLOAD)), Toast.LENGTH_LONG).show();
			}
		}
	};
}
