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
