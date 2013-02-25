/*
 * Bundle.java
 * 
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: 
 *  Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *  Julian Timpner<timpner@ibr.cs.tu-bs.de>
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

import java.util.LinkedList;
import java.util.logging.Level;
import java.util.logging.Logger;

public class Bundle {

    private static final Logger logger = Logger.getLogger(Bundle.class.getName());
    protected EID destination;
    protected EID source;
    protected EID custodian;
    protected EID reportto;
    protected long lifetime;
    protected Long timestamp;
    protected Long sequencenumber;
    protected long procflags;
    protected LinkedList<Block> blocks = new LinkedList<Block>();

    public enum Priority {

        BULK,
        NORMAL,
        EXPEDITED
    }

    public enum Flags {

        FRAGMENT(0),
        ADM_RECORD(1),
        NO_FRAGMENT(2),
        CUSTODY_REQUEST(3),
        DESTINATION_IS_SINGLETON(4),
        APP_ACK_REQUEST(5),
        PRIORITY(7),
        RECEPTION_REPORT(14),
        CUSTODY_REPORT(15),
        FORWARD_REPORT(16),
        DELIVERY_REPORT(17),
        DELETION_REPORT(18),
        // DTNSEC FLAGS (these are customized flags and not written down in any draft)
        DTNSEC_REQUEST_SIGN(26),
        DTNSEC_REQUEST_ENCRYPT(27),
        DTNSEC_STATUS_VERIFIED(28),
        DTNSEC_STATUS_CONFIDENTIAL(29),
        DTNSEC_STATUS_AUTHENTICATED(30),
        COMPRESSION_REQUEST(31);
        private int offset;

        Flags(int offset) {
            this.offset = offset;
        }

        public int getOffset() {
            return offset;
        }
    }

    public Bundle(EID destination, long lifetime) {
        this.destination = destination;
        this.lifetime = lifetime;
        setPriority(Priority.NORMAL);
        if (destination instanceof SingletonEndpoint) {
            setFlag(Flags.DESTINATION_IS_SINGLETON, true);
        }
    }

    public Bundle() {
    }

    public void setPriority(Priority p) {
        procflags &= ~(0b11 << Flags.PRIORITY.getOffset());
        procflags |= p.ordinal() << Flags.PRIORITY.getOffset();
    }

    public Priority getPriority() {
        int priority = (int) (procflags >> Flags.PRIORITY.getOffset() & 0b11);
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

    /**
     * Set a flag in the bundle
     *
     * @param flag a constant from Bundle.Flags
     * @param value true to set the flag, false to clear it
     */
    public void setFlag(Flags flag, boolean value) {
        logger.log(Level.FINE, "Setting {0}", flag.toString());
        if (value) {
            procflags |= 0b1L << flag.getOffset();
        } else {
            procflags &= ~(0b1L << flag.getOffset());
        }
    }

    public long getProcflags() {
        return procflags;
    }

    public void setProcflags(long procflags) {
        this.procflags = procflags;
    }

    /**
     * Appends a new Block to this bundle
     *
     * @param block the block to append
     */
    public void appendBlock(Block block) {
        if (blocks.size() > 0) {
            blocks.getLast().procflags &= ~(1 << Block.FlagOffset.LAST_BLOCK.ordinal());
        }
        block.procflags |= (1 << Block.FlagOffset.LAST_BLOCK.ordinal());
        blocks.addLast(block);
    }

    public LinkedList<Block> getBlocks() {
        return blocks;
    }

    public EID getDestination() {
        return destination;
    }

    public void setDestination(EID destination) {
        this.destination = destination;
    }

    public EID getSource() {
        return source;
    }

    public void setSource(EID source) {
        this.source = source;
    }

    public EID getCustodian() {
        return custodian;
    }

    public void setCustodian(EID custodian) {
        this.custodian = custodian;
    }

    public EID getReportto() {
        return reportto;
    }

    public void setReportto(EID reportto) {
        this.reportto = reportto;
    }

    public long getLifetime() {
        return lifetime;
    }

    public void setLifetime(long lifetime) {
        this.lifetime = lifetime;
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

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();
        builder.append("Bundle: ");
        builder.append(source);
        builder.append(" -> ");
        builder.append(destination);
        builder.append(" ");
        for (Block block : blocks) {
            builder.append("[");
            builder.append(block);
            builder.append("]");
        }
        return builder.toString();
    }
}
