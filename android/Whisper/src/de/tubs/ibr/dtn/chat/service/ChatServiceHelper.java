package de.tubs.ibr.dtn.chat.service;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.IBinder;
import de.tubs.ibr.dtn.chat.core.Roster;

public class ChatServiceHelper {
	
	private Context context = null;
	private ChatService service = null;
	private ChatServiceListener callback = null;
	
	public ChatServiceHelper(Context context, ChatServiceListener callback) {
		this.context = context;
		this.callback = callback;
	}
	
	public class ServiceNotConnectedException extends Exception {
		/**
		 * 
		 */
		private static final long serialVersionUID = 4670444143341726164L;
	};
	
	public ChatService getService() throws ServiceNotConnectedException {
		if (!isConnected()) {
			throw new ServiceNotConnectedException();
		}
		return this.service;
	}
	
	public void bind() {
		IntentFilter i = new IntentFilter(Roster.REFRESH);
		this.context.registerReceiver(notify_receiver, i);
		
		// Establish a connection with the service.  We use an explicit
		// class name because we want a specific service implementation that
		// we know will be running in our own process (and thus won't be
		// supporting component replacement by other applications).
		this.context.bindService(new Intent(this.context, ChatService.class), mConnection, Context.BIND_AUTO_CREATE);
	}
	
	public void unbind() {
		this.context.unregisterReceiver(notify_receiver);
		this.context.unbindService(mConnection);
	}
	
	// Container Activity must implement this interface
    public interface ChatServiceListener {
        public void onContentChanged(String buddyId);
        public void onServiceConnected(ChatService service);
        public void onServiceDisconnected();
    }
    
	private BroadcastReceiver notify_receiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent i) {
			callback.onContentChanged(i.getStringExtra("buddy"));
		}
	};
	
	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			ChatServiceHelper.this.service = ((ChatService.LocalBinder)service).getService();
			ChatServiceHelper.this.callback.onServiceConnected(ChatServiceHelper.this.service);
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			ChatServiceHelper.this.callback.onServiceDisconnected();
		}
	};
	
	public boolean isConnected() {
		return (this.service != null);
	}

}
