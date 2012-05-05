package de.tubs.ibr.dtn.dtalkie.service;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Date;
import java.util.LinkedList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import android.app.Service;
import android.content.Intent;
import android.content.SharedPreferences;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.MediaRecorder;
import android.os.Binder;
import android.os.Environment;
import android.os.FileObserver;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.util.Log;
import de.tubs.ibr.dtn.DaemonState;
import de.tubs.ibr.dtn.api.Block;
import de.tubs.ibr.dtn.api.BundleID;
import de.tubs.ibr.dtn.api.CallbackMode;
import de.tubs.ibr.dtn.api.DTNClient;
import de.tubs.ibr.dtn.api.DataHandler;
import de.tubs.ibr.dtn.api.GroupEndpoint;
import de.tubs.ibr.dtn.api.Registration;
import de.tubs.ibr.dtn.api.ServiceNotAvailableException;
import de.tubs.ibr.dtn.api.SessionDestroyedException;
import de.tubs.ibr.dtn.dtalkie.db.Message;
import de.tubs.ibr.dtn.dtalkie.db.MessageDatabase;
import de.tubs.ibr.dtn.dtalkie.db.MessageDatabase.Folder;

public class DTalkieService extends Service {
	
	private static final String TAG = "DTalkieService";
	private static final String ACTION_PLAY = "android.dtn.dtalkie.PLAY";
	
	public static final GroupEndpoint DTALKIE_GROUP_EID = new GroupEndpoint("dtn://dtalkie.dtn/broadcast");
	private Registration _registration = null;
	private Boolean _service_available = false;
	
	private final MediaRecorder recorder = new MediaRecorder();
	private final QueuingMediaPlayer player = new QueuingMediaPlayer();
	
	private boolean _onEar = false;
	private boolean connected = false;
	private File current_record_file = null;

	private DTalkieState _state = DTalkieState.UNINITIALIZED;
	private MessageDatabase _database = null;
	
	private SoundFXManager soundManager = null;
	
	private Sound SOUND_BEEP = new Sound("beep.mp3");
	private Sound SOUND_CONFIRM = new Sound("confirm.mp3");
	private Sound SOUND_QUIT = new Sound("quit.mp3");
	private Sound SOUND_RING = new Sound("ring.mp3");
	private Sound SOUND_SQUELSH_LONG = new Sound("squelsh_long.mp3");
	private Sound SOUND_SQUELSH_SHORT = new Sound("squelsh_short.mp3");
	
	private ExecutorService executor = null;
	
    // This is the object that receives interactions from clients.  See
    // RemoteService for a more complete example.
    private final IBinder mBinder = new LocalBinder();
    
    // basic dtn client
    private LocalDTNClient _client = null;
    
	private class LocalDTNClient extends DTNClient {
		
		public LocalDTNClient() {
			super(getApplicationInfo().packageName);
		}
		
		@Override
		protected void sessionConnected(Session session) {
			Log.d(TAG, "DTN session connected");
			_state = DTalkieState.READY;
			DTalkieService.this.setConnected(true);
		}

		@Override
		protected CallbackMode sessionMode() {
			return CallbackMode.FILEDESCRIPTOR;
		}

		@Override
		protected void online() {
		}

		@Override
		protected void offline() {
			DTalkieService.this.setConnected(false);
		}
	};

    private DataHandler _data_handler = new DataHandler()
    {
    	private de.tubs.ibr.dtn.api.Bundle bundle = null;
		private File file = null;
		private ParcelFileDescriptor fd = null;

		@Override
		public void startBundle(de.tubs.ibr.dtn.api.Bundle bundle) {
			this.bundle = bundle;
		}

		@Override
		public void endBundle() {
			if (file != null)
			{
				Log.i(TAG, "New message received.");
				
				final de.tubs.ibr.dtn.api.Bundle received = bundle;
				final File playfile = file;

				// run the queue and delivered process asynchronously
				executor.execute(new Runnable() {
			        public void run() {
			        	// add message to database
			        	Message msg = new Message(received.source, received.destination, playfile);
			        	msg.setCreated(received.timestamp);
			        	msg.setReceived(new Date());
			        	_database.put(Folder.INBOX, msg);
			        	
						try {
							_client.getSession().delivered( new BundleID(received) );
						} catch (Exception e) {
							Log.e(TAG, "Can not mark bundle as delivered.", e);
						}
						
						Intent intent = new Intent(DTalkieService.this, DTalkieService.class);
						intent.setAction(ACTION_PLAY);
						startService(intent);
			        }
				});
				
				file = null;
			}
			
			bundle = null;
		}

		@Override
		public void startBlock(Block block) {
			if ((block.type == 1) && (file == null))
			{
				File folder = DTalkieService.getStoragePath();
				
				// create a new temporary file
				try {
					file = File.createTempFile("msg", ".3gp", folder);
				} catch (IOException e) {
					Log.e(TAG, "Can not create temporary file.", e);
					file = null;
				}
			}
			
			Log.d(TAG, "startBlock: " + String.valueOf(block.type));
		}

		@Override
		public void endBlock() {
			if (fd != null)
			{
				// close filedescriptor
				try {
					fd.close();
					fd = null;
				} catch (IOException e) {
					Log.e(TAG, "Can not close filedescriptor.", e);
				}
			}
			
			if (file != null)
			{
				// unset the payload file
				Log.i(TAG, "File received: " + file.getAbsolutePath());
			}
		}

		@Override
		public void characters(String data) {
			// nothing to do here, since we using FILEDESCRIPTOR mode
		}

		@Override
		public void payload(byte[] data) {
			// nothing to do here, since we using FILEDESCRIPTOR mode
		}

		@Override
		public ParcelFileDescriptor fd() {
			// create new filedescriptor
			try {
				fd = ParcelFileDescriptor.open(file, 
						ParcelFileDescriptor.MODE_CREATE + 
						ParcelFileDescriptor.MODE_READ_WRITE);
				
				return fd;
			} catch (FileNotFoundException e) {
				Log.e(TAG, "Can not create a filedescriptor.", e);
			}
			
			return null;
		}

		@Override
		public void progress(long current, long length) {
			Log.i(TAG, "Payload: " + current + " of " + length + " bytes.");
		}

		@Override
		public void finished(int startId) {
			stopSelfResult(startId);
		}
    };
    
    /**
     * Class for clients to access.  Because we know this service always
     * runs in the same process as its clients, we don't need to deal with
     * IPC.
     */
    public class LocalBinder extends Binder {
    	public DTalkieService getService() {
            return DTalkieService.this;
        }
    }
    
	public synchronized void waitUntilConnected() throws InterruptedException
	{
		while (!connected) wait();
	}
	
	public synchronized Boolean isConnected()
	{
		return connected;
	}
	
	private synchronized void setConnected(Boolean val)
	{
		connected = val;
		notifyAll();
	}

	@Override
	public IBinder onBind(Intent intent) {
		return mBinder;
	}
	
	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i(TAG, "Received start id " + startId + ": " + intent);
        
        if (intent == null) return START_NOT_STICKY;
        if (intent.getAction() == null) return START_NOT_STICKY;

        // create a task to check for messages
        if (intent.getAction().equals(de.tubs.ibr.dtn.Intent.RECEIVE)) {
        	final int stopId = startId;
        	
			// schedule next bundle query
        	executor.execute(new Runnable() {
		        public void run() {
			        try {
		        		while (_client.query());
		        	} catch (Exception e) { };
		        	
		        	stopSelfResult(stopId);
		        }
			});
			
			return START_STICKY;
        }
        else if (intent.getAction().equals(ACTION_PLAY)) {
        	final int stopId = startId;
        	
			// schedule next bundle query
        	executor.execute(new Runnable() {
		        public void run() {
					// restore autoplay settings
					SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(DTalkieService.this);
					setAutoPlay( prefs.getBoolean("autoplay", false) );
					
		        	// wait until the player is idle
		        	try {
						player.waitIdle();
					} catch (InterruptedException e) {
						Log.e(TAG, null, e);
					}
		        	
		        	stopSelfResult(stopId);
		        }
			});
        	
        	return START_STICKY;
        }
        
        return START_NOT_STICKY;
	}
	
	@Override
	public void onCreate()
	{
		Log.i(TAG, "Service created.");
		super.onCreate();
		
		// create executor
		executor = Executors.newSingleThreadExecutor();
		
		// create message database
		_database = new MessageDatabase(this);
		
		// init sound pool
		soundManager = new SoundFXManager();
		soundManager.initialize(AudioManager.STREAM_VOICE_CALL, 4);
		soundManager.initialize(AudioManager.STREAM_MUSIC, 4);
		
		soundManager.load(this, SOUND_BEEP);
		soundManager.load(this, SOUND_CONFIRM);
		soundManager.load(this, SOUND_QUIT);
		soundManager.load(this, SOUND_RING);
		soundManager.load(this, SOUND_SQUELSH_LONG);
		soundManager.load(this, SOUND_SQUELSH_SHORT);
		
		// create registration
    	_registration = new Registration("dtalkie");
    	_registration.add(DTALKIE_GROUP_EID);
		
		// register own data handler for incoming bundles
    	_client = new LocalDTNClient();
		_client.setDataHandler(_data_handler);
		
		try {
			_client.initialize(this, _registration);
			_service_available = true;
		} catch (ServiceNotAvailableException e) {
			_service_available = false;
		}
	}
	
	public Boolean isServiceAvailable() {
		return _service_available;
	}
	
	public Boolean isServiceRunning() {
		try {
			return (this._client.getDTNService().getState() == DaemonState.ONLINE);
		} catch (RemoteException e) {
			return false;
		}
	}
	
	@Override
	public void onDestroy()
	{
		executor.shutdown();
		
		try {
			while (!executor.awaitTermination(1, TimeUnit.MINUTES));
			_database.close();
			_client.terminate();
		} catch (InterruptedException e) {
			Log.e(TAG, "Interrupted on service destruction.", e);
		}
		
	    _client = null;
		
		super.onDestroy();
		Log.i(TAG, "Service destroyed.");
	}
	
	public MessageDatabase getDatabase() {
		return _database;
	}
	
	public static File getStoragePath()
	{
		if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED))
		{
			File externalStorage = Environment.getExternalStorageDirectory();
			
			// create working directory
			File sharefolder = new File(externalStorage.getPath() + File.separatorChar + "dtalkie");
			if (!sharefolder.exists())
			{
					sharefolder.mkdir();
			}
			
			return sharefolder;
		}
		
		return null;
	}
	
	public void setOnEar(boolean val) {
		_onEar = val;
	}
	
	private void playSound(Sound s) {
		if (_onEar) {
			soundManager.play(this, AudioManager.STREAM_VOICE_CALL, s);
		} else {
			soundManager.play(this, AudioManager.STREAM_MUSIC, s);
		}
	}

    public synchronized void startRecording() throws IOException
    {
    	// only start recording in state READY
    	if (_state != DTalkieState.READY) return;
    	
    	Log.i(TAG, "start recording audio");
    	
    	File path = DTalkieService.getStoragePath();
    	
    	if (path == null) throw new IOException("no storage path available");
    	
    	Log.i(TAG, "create temporary file in " + path.getAbsolutePath());
    	
    	// create temporary file
    	current_record_file = File.createTempFile("record", ".3gp", path);
    	
        recorder.setAudioSource(MediaRecorder.AudioSource.DEFAULT);
        recorder.setOutputFormat(MediaRecorder.OutputFormat.THREE_GPP);
        recorder.setAudioEncoder(MediaRecorder.AudioEncoder.AMR_NB);
        recorder.setOutputFile(current_record_file.getAbsolutePath());
        recorder.prepare();
        recorder.start();
        
        _state = DTalkieState.RECORDING;
        
        playSound(SOUND_BEEP);
        
    	FileObserver _observer = new FileObserver(current_record_file.getAbsolutePath(), FileObserver.CLOSE_WRITE) {
			@Override
			public void onEvent(int arg0, String arg1) {
				if (arg0 == FileObserver.CLOSE_WRITE)
				{
					try {
						finalizeRecording();
						stopWatching();
					} catch (Exception ex) { }
				}
			}
    	};
    	
    	_observer.startWatching();
    }
    
    public synchronized void stopRecording() throws IOException
    {
    	// only stop recording in state RECORDING
    	if (_state != DTalkieState.RECORDING) return;
    	_state = DTalkieState.STOPPING;
    	
    	Log.i(TAG, "stop recording audio");
    	
    	// stop the recorder
    	recorder.stop();
    }

    private synchronized void finalizeRecording() throws Exception
    {
    	// only stop recording in state RECORDING
    	if (_state != DTalkieState.STOPPING)
    		throw new Exception("illegal state exception");

    	if (current_record_file != null)
    	{
    		(new Thread(new SendMessageTask(current_record_file))).start();
    		current_record_file = null;
    	}
    	
    	recorder.reset();
    	_state = DTalkieState.STOPPED;
    	
    	playSound(SOUND_QUIT);
    }
    
    public void setAutoPlay(boolean val) {
    	player.setAutoPlay(val);
    }
    
    public void play(Folder folder, Message msg) {
		// unmark the message
		_database.mark(folder, msg, false);

		// requeue the message to the player
		player.play(msg);
    }
    
    private class SendMessageTask implements Runnable {
    	private File msg = null;
    	
    	public SendMessageTask(File msg) {
    		this.msg = msg;
    	}
    	
		@Override
		public void run() {
			try {
				DTalkieService.this.waitUntilConnected();

				try {
					ParcelFileDescriptor fd = ParcelFileDescriptor.open(this.msg, ParcelFileDescriptor.MODE_READ_ONLY);
					_client.getSession().sendFileDescriptor(DTALKIE_GROUP_EID, 1800, fd, this.msg.length());
					this.msg.delete();
					Log.i(TAG, "Recording sent");
				} catch (FileNotFoundException ex) {
					Log.e(TAG, "Can not open message file for transmission", ex);
				} catch (SessionDestroyedException ex) {
					Log.e(TAG, "DTN session has been destroyed.", ex);
				} catch (Exception e) {
					Log.e(TAG, "Unexpected exception while transmit message.", e);
				}
			} catch (InterruptedException e) { }
			
			_state = DTalkieState.READY;
		}
    }
	  
    private class QueuingMediaPlayer
    {
    	private final MediaPlayer player = new MediaPlayer();
    	
    	private Message current_message = null;
    	private LinkedList<Message> play_queue = new LinkedList<Message>();
    	private Boolean autoplay = false;
    	private Boolean idle = true;
    	
        private void load(Message msg) throws IOException
        {
        	idle = false;
        	current_message = msg;
    		player.setOnCompletionListener(playerCompletionListener);
    		player.setOnPreparedListener(playerPrepareListener);
    		player.setDataSource(msg.getFile().getAbsolutePath());
    		player.setAudioStreamType(AudioManager.STREAM_MUSIC);
    		player.prepareAsync();
        }
        
        public synchronized void waitIdle() throws InterruptedException
        {
        	while (!idle) wait();
        }
        
        public synchronized void setAutoPlay(boolean val) {
        	autoplay = val;
        	notifyNewMessage();
        }
        
        public synchronized void notifyNewMessage() {
        	idle = false;
        	if (autoplay) next();
        }
        
        public synchronized void play(Message msg)
        {
        	if (current_message == null)
        	{
    			try {
					load(msg);
				} catch (IOException e) {
					Log.e(TAG, "failed to load message", e);
				}
        	}
        	else
        	{
        		play_queue.offer(msg);
        	}
        }
        
        private synchronized void next()
        {
        	idle = true; notifyAll();
        	
        	if (current_message != null) {
				// mark message
	        	_database.mark(Folder.INBOX, current_message, true);
	        	current_message = null;
	    		player.reset();
        	}
        	
        	// get next message in the queue
        	Message next = play_queue.poll();
        	
        	// abort if autoplay is off and no message in the queue
        	if ((next == null) && !autoplay) return;
        	
        	// search for the next autoplay message
        	if (next == null) next = _database.getMarkedMessage(Folder.INBOX, false);
        	
        	// play the next file if object is not null
    		if (next != null) {
    			try {
					load(next);
				} catch (IOException e) {
					Log.e(TAG, "failed to load next message", e);
				}
    		}
        }
        
        private MediaPlayer.OnPreparedListener playerPrepareListener = new MediaPlayer.OnPreparedListener() {

    		@Override
    		public void onPrepared(MediaPlayer mp) {
    			playSound(SOUND_BEEP);
    			player.start();
    		}
    		
        };
        
        private MediaPlayer.OnCompletionListener playerCompletionListener = new MediaPlayer.OnCompletionListener() {
    		@Override
    		public void onCompletion(MediaPlayer arg0) {
    			playSound(SOUND_CONFIRM);   			
    			next();
    		}
        };
    }
    

}
