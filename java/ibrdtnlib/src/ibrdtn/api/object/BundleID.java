/*
 * BundleID.java
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
package ibrdtn.api.object;

import ibrdtn.api.object.Bundle.Flags;
import ibrdtn.api.object.Bundle.Priority;

public class BundleID {

    private String source = null;
    private String destination = null;
    private Long timestamp = null;
    private Long sequencenumber = null;
    private Long procflags = null;

    public BundleID() {
    }

    public BundleID(String source, Long timestamp, Long sequencenumber) {
        this.source = source;
        this.timestamp = timestamp;
        this.sequencenumber = sequencenumber;
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

    public Long getSequencenumber() {
        return sequencenumber;
    }

    public void setSequencenumber(Long sequencenumber) {
        this.sequencenumber = sequencenumber;
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

    public long getProcflags() {
        return procflags;
    }

    public void setProcflags(Long procflags) {
        this.procflags = procflags;
    }

    @Override
    public String toString() {
        return String.valueOf(this.timestamp) + " " + String.valueOf(this.sequencenumber) + " " + this.source;
    }
}
