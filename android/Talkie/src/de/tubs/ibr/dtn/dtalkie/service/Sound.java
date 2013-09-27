/*
 * Sound.java
 * 
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
package de.tubs.ibr.dtn.dtalkie.service;


public class Sound {
    public static final Sound BEEP = new Sound("beep.mp3");
    public static final Sound CONFIRM = new Sound("confirm.mp3");
    public static final Sound QUIT = new Sound("quit.mp3");
    public static final Sound RING = new Sound("ring.mp3");
    public static final Sound SQUELSH_LONG = new Sound("squelsh_long.mp3");
    public static final Sound SQUELSH_SHORT = new Sound("squelsh_short.mp3");
    
	private String mFilename = null;
	private int mId = 0;
	private boolean mReady = false;
	
	public Sound(Sound s) {
		this.setFilename(s.getFilename());
	}
	
	public Sound(String filename) {
		this.setFilename(filename);
	}

	public String getFilename() {
		return mFilename;
	}

	public void setFilename(String filename) {
		this.mFilename = filename;
	}

	public Integer getSoundId() {
		return mId;
	}

	public void setSoundId(int soundId) {
		mId = soundId;
	}

	public boolean isReady() {
		return mReady;
	}

	public void setReady(boolean mReady) {
		this.mReady = mReady;
	}

	@Override
	public boolean equals(Object o) {
		if (o instanceof Sound) {
			return mFilename.equals(((Sound)o).getFilename());
		}
		return super.equals(o);
	}
}
