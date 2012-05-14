package de.tubs.ibr.dtn.chat;

import java.util.Date;

import android.app.ListActivity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnKeyListener;
import android.widget.BaseAdapter;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.TextView;
import de.tubs.ibr.dtn.chat.core.Buddy;
import de.tubs.ibr.dtn.chat.core.Message;
import de.tubs.ibr.dtn.chat.core.Roster;
import de.tubs.ibr.dtn.chat.service.ChatService;

public class MessageActivity extends ListActivity {
	
	private final String TAG = "MessageActivity";
	private MessageView view = null;
	private Buddy buddy = null;
	private static Buddy visibleBuddy = null; 
	private ChatService service = null;
	
	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			MessageActivity.this.service = ((ChatService.LocalBinder)service).getService();
			Log.i(TAG, "service connected");
			
			// load buddy from roster
			buddy = MessageActivity.this.service.getRoster().get( getIntent().getStringExtra("buddy") );
			
			if (buddy == null) {
				Log.e(TAG, "Error buddy not found: " + getIntent().getStringExtra("buddy"));
				return;
			}
			
			// activate message view
			MessageActivity.this.view = new MessageView(MessageActivity.this, MessageActivity.this.service.getRoster(), MessageActivity.this.buddy);
			MessageActivity.this.setListAdapter(MessageActivity.this.view);
			
			IntentFilter i = new IntentFilter(Roster.REFRESH);
			registerReceiver(notify_receiver, i);
			
			refresh();
			
			// set "enter" handler
			EditText textedit = (EditText) findViewById(R.id.textMessage);
			textedit.setOnKeyListener(new OnKeyListener() {
				@Override
				public boolean onKey(View v, int keycode, KeyEvent event) {
					if ((KeyEvent.KEYCODE_ENTER == keycode) && (event.getAction() == KeyEvent.ACTION_DOWN))
					{
						flushTextBox();
						return true;
					}
					return false;
				}
			});
			
			// set send handler
			ImageButton buttonSend = (ImageButton) findViewById(R.id.buttonSend);
			buttonSend.setOnClickListener(new OnClickListener() {
				@Override
				public void onClick(View v) {
					flushTextBox();
				}
			});
			
			// set the current visible buddy
			visibleBuddy = buddy;
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			Log.i(TAG, "service disconnected");
			MessageActivity.this.service = null;
		}
	};
	
	private void flushTextBox()
	{
		EditText text = (EditText) findViewById(R.id.textMessage);
		
		if (text.getText().length() > 0) {
			Message msg = new Message(false, new Date(), new Date(), text.getText().toString());
			msg.setBuddy(buddy);
			
			Log.i(TAG, "send text to " + buddy.getNickname() + ": " + msg.getPayload());
			
			// send the message
			new SendChatMessageTask().execute(msg);
			
			// store the message in the database
			MessageActivity.this.service.getRoster().storeMessage(msg);
			
			// clear the text field
			text.setText("");
			text.requestFocus();
		}
	}

	void doUnbindService() {
        // Detach our existing connection.
        unbindService(mConnection);
	}

	@Override
	protected void onDestroy() {
		unregisterReceiver(notify_receiver);
		
		MessageActivity.this.view.onDestroy(this);
		MessageActivity.this.view = null;
	    super.onDestroy();
	    doUnbindService();
	}
	
	private BroadcastReceiver notify_receiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent i) {
			if (i.getStringExtra("buddy").equals(buddy.getEndpoint())) {
				MessageActivity.this.refresh();
			}
		}
	};
	
	public static Buddy getVisibleBuddy()
	{
		return visibleBuddy;
	}
	
	@Override
	protected void onPause() {
		super.onPause();
		visibleBuddy = null;
	}

	@Override
	protected void onResume() {
		super.onResume();
		
		// set the current visible buddy
		visibleBuddy = buddy;
		
		if (this.view != null)
		{
			this.view.refresh();
		}
		
		// refresh visible data
		refresh();
	}

	public void refreshCallback()
	{
		MessageActivity.this.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				refresh();
				((BaseAdapter)getListView().getAdapter()).notifyDataSetChanged();
			}
		});
	}
	
	private class SendChatMessageTask extends AsyncTask<Message, Integer, Integer> {
		protected Integer doInBackground(Message... msgs) {
			int count = msgs.length;
			int totalSize = 0;
			for (int i = 0; i < count; i++)
			{
				try {
					MessageActivity.this.service.sendMessage(msgs[i]);
					
					// update total size
					totalSize += msgs[i].getPayload().length();
					
					// publish the progress
					publishProgress((int) ((i / (float) count) * 100));
				} catch (Exception e) {
					Log.e(TAG, "could not send the message");
				}
			}
			return totalSize;
		}

		protected void onProgressUpdate(Integer... progress) {
			//setProgressPercent(progress[0]);
		}

		protected void onPostExecute(Integer result) {
			//showDialog("Downloaded " + result + " bytes");
		}
	}
	
	private void refresh()
	{
		if (buddy == null) 	return;

		setTitle(getResources().getString(R.string.conversation_with) + " " + buddy.getNickname());
		
		ImageView iconTitleBar = (ImageView) findViewById(R.id.iconTitleBar);
		TextView labelTitleBar = (TextView) findViewById(R.id.labelTitleBar);
		TextView bottomtextTitleBar = (TextView) findViewById(R.id.bottomtextTitleBar);
		
		labelTitleBar.setText( buddy.getNickname() );
		bottomtextTitleBar.setText( buddy.getStatus() );
		
		if (buddy.getStatus() != null)
		{
			if (buddy.getStatus().length() > 0) { 
				bottomtextTitleBar.setText(buddy.getStatus());
			} else {
				bottomtextTitleBar.setText(buddy.getEndpoint());
			}
		}
		else
		{
			bottomtextTitleBar.setText(buddy.getEndpoint());
		}
		
		String presence = buddy.getPresence();
		
		iconTitleBar.setImageResource(R.drawable.online);
		
		if (presence != null)
		{
			if (presence.equalsIgnoreCase("unavailable"))
			{
				iconTitleBar.setImageResource(R.drawable.offline);
			}
			else if (presence.equalsIgnoreCase("xa"))
			{
				iconTitleBar.setImageResource(R.drawable.xa);
			}
			else if (presence.equalsIgnoreCase("away"))
			{
				iconTitleBar.setImageResource(R.drawable.away);
			}
			else if (presence.equalsIgnoreCase("dnd"))
			{
				iconTitleBar.setImageResource(R.drawable.busy);
			}
			else if (presence.equalsIgnoreCase("chat"))
			{
				iconTitleBar.setImageResource(R.drawable.online);
			}
		}
		
		// if the presence is older than 60 minutes then mark the buddy as offline
		if (!buddy.isOnline())
		{
			iconTitleBar.setImageResource(R.drawable.offline);
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	    MenuInflater inflater = getMenuInflater();
	    inflater.inflate(R.menu.message_menu, menu);
	    return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
	    switch (item.getItemId()) {
	    	case R.id.itemClose:
	    		this.finish();
	    		return true;
	    		
	    	case R.id.itemClearMessages:
	    		this.service.getRoster().clearMessages(buddy);
	    		return true;
	    	
	    	default:
	    		return super.onOptionsItemSelected(item);
	    }
	}

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.chat_main);
		
	    // Establish a connection with the service.  We use an explicit
	    // class name because we want a specific service implementation that
	    // we know will be running in our own process (and thus won't be
	    // supporting component replacement by other applications).
	    bindService(new Intent(MessageActivity.this, 
	            ChatService.class), mConnection, Context.BIND_AUTO_CREATE);
	}

}
