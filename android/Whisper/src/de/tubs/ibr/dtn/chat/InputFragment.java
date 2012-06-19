package de.tubs.ibr.dtn.chat;

import java.util.Date;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnKeyListener;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.ImageButton;
import de.tubs.ibr.dtn.chat.core.Buddy;
import de.tubs.ibr.dtn.chat.core.Message;
import de.tubs.ibr.dtn.chat.service.ChatService;

public class InputFragment extends Fragment {
	
	private final String TAG = "InputFragment";
	private String buddyId = null;
	private ChatService service = null;
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.input_view, container, false);
		
		// set "enter" handler
		EditText textedit = (EditText) v.findViewById(R.id.textMessage);
		textedit.setEnabled(false);
		
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
		ImageButton buttonSend = (ImageButton) v.findViewById(R.id.buttonSend);
		buttonSend.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				flushTextBox();
			}
		});
		
		return v;
	}
	
	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			InputFragment.this.service = ((ChatService.LocalBinder)service).getService();
			
			// load draft message
			loadDraftMessage();
			
			Log.i(TAG, "service connected");
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			Log.i(TAG, "service disconnected");
			InputFragment.this.service = null;
		}
	};
	
	@Override
	public void onDestroy() {
	    if (mConnection != null) {
	    	// Detach our existing connection.
	    	getActivity().unbindService(mConnection);
	    }

	    super.onDestroy();
	}
	
	@Override
	public void onAttach(Activity activity) {
		super.onAttach(activity);
		
		// Establish a connection with the service.  We use an explicit
		// class name because we want a specific service implementation that
		// we know will be running in our own process (and thus won't be
		// supporting component replacement by other applications).
		getActivity().bindService(new Intent(activity, ChatService.class), mConnection, Context.BIND_AUTO_CREATE);
	}
	
	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		super.onActivityCreated(savedInstanceState);
		
		// restore buddy id
		if (savedInstanceState != null) {
			// restore buddy id
			if (savedInstanceState.containsKey("buddyId")) {
				buddyId = savedInstanceState.getString("buddyId");
			}
		}
	}

	@Override
	public void onSaveInstanceState(Bundle outState) {
		super.onSaveInstanceState(outState);
		
		if (buddyId != null) {
			// save buddy id
			outState.putString("buddyId", buddyId);
		}
	}
	
	public void setBuddy(String buddyId) {
		if (this.buddyId != buddyId) {
			saveDraftMessage();
			
			this.buddyId = buddyId;
			
			// load draft message
			loadDraftMessage();
		}
	}
	
	@Override
	public void onPause() {
		saveDraftMessage();
		super.onPause();
	}

	@Override
	public void onResume() {
		super.onResume();
		
		// load draft message
		loadDraftMessage();
	}

	private void flushTextBox()
	{
		if (service == null) return;
		
		// load buddy from roster
		Buddy buddy = this.service.getRoster().get( buddyId );
		
		EditText text = (EditText) getView().findViewById(R.id.textMessage);
		
		if (text.getText().length() > 0) {
			Message msg = new Message(false, new Date(), new Date(), text.getText().toString());
			msg.setBuddy(buddy);
			
			Log.i(TAG, "send text to " + buddy.getNickname() + ": " + msg.getPayload());
			
			// send the message
			new SendChatMessageTask().execute(msg);
			
			// store the message in the database
			InputFragment.this.service.getRoster().storeMessage(msg);
			
			// clear the text field
			text.setText("");
			text.requestFocus();
		}
	}

	private void saveDraftMessage() {
		if (this.buddyId == null) return;
		if (this.service == null) return;
		
		// load buddy from roster
		Buddy buddy = this.service.getRoster().get( this.buddyId );
		
		EditText text = (EditText) getView().findViewById(R.id.textMessage);
		String msg = text.getText().toString();
		
		if (msg.length() > 0)
			buddy.setDraftMessage( msg );
		else
			buddy.setDraftMessage( null );
		
		this.service.getRoster().store(buddy);
	}
	
	private void loadDraftMessage() {
		EditText text = (EditText) getView().findViewById(R.id.textMessage);
		text.setText("");
		
		if (this.buddyId == null) {
			text.setEnabled(false);
			return;
		}
		
		if (this.service == null) return;
		
		// load buddy from roster
		Buddy buddy = this.service.getRoster().get( this.buddyId );
		String msg = buddy.getDraftMessage();
		
		if (msg != null) text.append(msg);

		text.setEnabled(true);
		
		text.requestFocus();
	}
	
	private class SendChatMessageTask extends AsyncTask<Message, Integer, Integer> {
		protected Integer doInBackground(Message... msgs) {
			int count = msgs.length;
			int totalSize = 0;
			for (int i = 0; i < count; i++)
			{
				try {
					InputFragment.this.service.sendMessage(msgs[i]);
					
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
}
