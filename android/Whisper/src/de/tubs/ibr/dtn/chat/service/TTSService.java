package de.tubs.ibr.dtn.chat.service;

import java.util.HashMap;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import android.app.Service;
import android.content.Intent;
import android.media.AudioManager;
import android.os.IBinder;
import android.speech.tts.TextToSpeech;
import android.util.Log;

public class TTSService extends Service {
	
	private final static String TAG = "TTSService";
	public final static String INTENT_SPEAK = "de.tubs.ibr.dtn.chat.service.INTENT_SPEAK";
	
	private ExecutorService executor = Executors.newSingleThreadExecutor();
	
	private TextToSpeech _tts_service = null;
	private ConcurrentLinkedQueue<SpeechTask> _tts_queue = null;
	private Boolean _tts_initialized = false;
	
	@Override
	public IBinder onBind(Intent intent) {
		return null;
	}
	
	private class SpeechTask {
		public Integer startId;
		public String speechText;
	}

	@SuppressWarnings("deprecation")
	@Override
	public void onCreate() {
		super.onCreate();
		
		// initialize text to speech service
		_tts_initialized = false;
		_tts_queue = new ConcurrentLinkedQueue<SpeechTask>();
		_tts_service = new TextToSpeech(this, _tts_init_listener);
		_tts_service.setOnUtteranceCompletedListener(_tts_complete_listener);
	}

	@Override
	public void onDestroy() {
		// unload tts service
		_tts_service.shutdown();
		
		try {
			// stop executor
			executor.shutdown();
			
			// ... and wait until all jobs are done
			if (!executor.awaitTermination(10, TimeUnit.SECONDS)) {
				executor.shutdownNow();
			}
		} catch (InterruptedException e) {
			Log.e(TAG, "Interrupted on service destruction.", e);
		}
		
		super.onDestroy();
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		if (intent == null) return super.onStartCommand(intent, flags, startId);
		if (intent.getAction() == null) return super.onStartCommand(intent, flags, startId);
		
		if (intent.getAction().equals(TTSService.INTENT_SPEAK)) {
			SpeechTask task = new SpeechTask();
			task.speechText = intent.getStringExtra("speechText");
			task.startId = startId;
			_tts_queue.add(task);

			if (_tts_initialized) {
				executor.execute(speechNow);
			}
			
			return Service.START_STICKY;
		}

		return super.onStartCommand(intent, flags, startId);
	}
	
	private TextToSpeech.OnInitListener _tts_init_listener = new TextToSpeech.OnInitListener() {
		public void onInit(int status) {
	        if (status == TextToSpeech.SUCCESS) {
				_tts_initialized = true;
				executor.execute(speechNow);
	        } else {
	            Log.e(TAG, "TTS initilization failed!");
	        }
		}
	};
	
	private TextToSpeech.OnUtteranceCompletedListener _tts_complete_listener = new TextToSpeech.OnUtteranceCompletedListener() {
		public void onUtteranceCompleted(String utteranceId) {
			TTSService.this.stopSelfResult(Integer.valueOf(utteranceId));
		}
	};
	
	private Runnable speechNow = new Runnable() {
		public void run() {
			SpeechTask data = _tts_queue.poll();
			if (data != null) {
				Log.d(TAG, "Speak: " + data.speechText);
				HashMap<String, String> params = new HashMap<String, String>();
				params.put(android.speech.tts.TextToSpeech.Engine.KEY_PARAM_UTTERANCE_ID, data.startId.toString());
				params.put(TextToSpeech.Engine.KEY_PARAM_STREAM, String.valueOf(AudioManager.STREAM_VOICE_CALL));
				_tts_service.speak(data.speechText, TextToSpeech.QUEUE_ADD, params);
				executor.execute(this);
			}
		}
	};
}
