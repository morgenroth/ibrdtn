/*
 * CallbackMode.java
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

public enum CallbackMode implements Parcelable {
	SIMPLE,
	FILEDESCRIPTOR,
	PASSTHROUGH;

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(final Parcel dest, final int flags) {
        dest.writeInt(ordinal());
    }

    public static final Creator<CallbackMode> CREATOR = new Creator<CallbackMode>() {
        @Override
        public CallbackMode createFromParcel(final Parcel source) {
            return CallbackMode.values()[source.readInt()];
        }

        @Override
        public CallbackMode[] newArray(final int size) {
            return new CallbackMode[size];
        }
    };
}
