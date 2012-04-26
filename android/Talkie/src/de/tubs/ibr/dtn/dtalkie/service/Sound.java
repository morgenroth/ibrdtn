package de.tubs.ibr.dtn.dtalkie.service;

import java.util.HashMap;

public class Sound {
	private String filename = null;
	private HashMap<Integer, Integer> poolMap = new HashMap<Integer, Integer>();
	
	public Sound(String filename) {
		this.setFilename(filename);
	}

	public String getFilename() {
		return filename;
	}

	public void setFilename(String filename) {
		this.filename = filename;
	}

	public Integer getSoundId(int streamType) {
		return this.poolMap.get(streamType);
	}

	public void setSoundId(int streamType, int soundId) {
		this.poolMap.put(streamType, soundId);
	}
}
