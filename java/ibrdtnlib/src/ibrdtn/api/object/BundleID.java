/*
 * BundleID.java
 * 
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by:  Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *              Julian Timpner      <timpner@ibr.cs.tu-bs.de>
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
package ibrdtn.api.object;

import ibrdtn.api.object.Bundle.Flags;
import ibrdtn.api.object.Bundle.Priority;

public class BundleID implements Comparable<BundleID> {

    private String source = null;
    private Long timestamp = null;
    private Long sequenceNumber = null;
    private Long procflags = null;
    private boolean isFragment = false;
    private Long fragOffset = null;
    private String destination = null;

    public BundleID() {
    }
    
    public BundleID(Bundle b)
	{
		this.source = b.getSource().toString();
		this.timestamp = b.getTimestamp();
		this.sequenceNumber = b.getSequencenumber();
		
		this.fragment = b.get(Bundle.Flags.FRAGMENT);
		this.fragment_offset = b.getFragmentOffset();
	}

    public BundleID(String source, Long timestamp, Long sequencenumber) {
        this.source = source;
        this.timestamp = timestamp;
        this.sequenceNumber = sequencenumber;
    }

    public String getSource() {
        return source;
    }

    public void setSource(String source) {
        this.source = source;
    }

    public String getDestination() {
        return destination;
    }

    public void setDestination(String destination) {
        this.destination = destination;
    }

    public Long getTimestamp() {
        return timestamp;
    }

    public void setTimestamp(Long timestamp) {
        this.timestamp = timestamp;
    }

    public Long getSequenceNumber() {
        return sequenceNumber;
    }

    public void setSequenceNumber(Long seq) {
        this.sequenceNumber = seq;
    }

    public long getProcflags() {
        return procflags;
    }

    public void setProcflags(Long procflags) {
        this.procflags = procflags;
    }

    public void setFragOffset(long fragOffset) {
        this.fragOffset = fragOffset;
    }

    public Long getFragOffset() {
        return fragOffset;
    }

    public Priority getPriority() {
        int priority = (int) (procflags >> Flags.PRIORITY.getOffset()) & 0b11;
        // reduce the priority by modulo 3 since value 3 is not a valid priority
        return Priority.values()[priority % Priority.values().length];
    }

    public boolean isSingleton() {
        int single = (int) (procflags >> Flags.DESTINATION_IS_SINGLETON.getOffset()) & 0b11;
        if (single == 0) {
            return false;
        } else {
            return true;
        }
    }

    @Override
    public String toString() {
        String fragString = " ";
        if (fragOffset != null) {
            fragString += String.valueOf(this.fragOffset) + " ";
        }
        return String.valueOf(this.timestamp) + " " + String.valueOf(this.sequenceNumber) + fragString + this.source;
    }

    @Override
    public int compareTo(BundleID o) {
        final int BEFORE = -1;
        final int EQUAL = 0;
        final int AFTER = 1;

        if (this.getPriority().ordinal() < o.getPriority().ordinal()) {
            return BEFORE;
        } else if (this.getPriority().ordinal() > o.getPriority().ordinal()) {
            return AFTER;
        } else {
            return EQUAL;
        }
    }
}
