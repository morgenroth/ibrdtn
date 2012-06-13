/*
 * Block.java
 * 
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

package de.tubs.ibr.dtn.api;

import android.os.Parcel;
import android.os.Parcelable;

public class Block implements Parcelable {
	public Integer type = null;
	public Long procflags = null;
	public Long length = null;
	
	@Override
	public int describeContents() {
		return 0;
	}

	@Override
	public void writeToParcel(Parcel dest, int flags) {
		dest.writeInt( (type == null) ? 0 : type );
		dest.writeLong( (procflags == null) ? 0L : procflags );
		dest.writeLong( (length == null) ? 0L : length );
	}
	
    public static final Creator<Block> CREATOR = new Creator<Block>() {
        @Override
        public Block createFromParcel(final Parcel source) {
        	Block b = new Block();
        	b.type = source.readInt();
        	b.procflags = source.readLong();
        	b.length = source.readLong();
        	return b;
        }

        @Override
        public Block[] newArray(final int size) {
            return new Block[size];
        }
    };
}
