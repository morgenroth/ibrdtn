package ibrdtn.api.object;

import java.util.LinkedList;

public class Bundle {
	EID destination;
	EID source;
	EID custodian;
	EID reportto;
	long lifetime;
	Long timestamp;
	Long sequencenumber;
	long procflags;
	LinkedList<Block> blocks = new LinkedList<Block>();
	
	public enum Priority {
		BULK,
		NORMAL,
		EXPEDITED
	}

	@SuppressWarnings("unused")
	private static final class FlagOffset {
		static final int FRAGMENT = 0;
		static final int ADM_RECORD = 1;
		static final int NO_FRAGMENT = 2;
		static final int CUSTODY_REQUEST = 3;
		static final int DESTINATION_IS_SINGLETON = 4;
		static final int APP_ACK_REQUEST = 5;
		static final int PRIORITY = 7;
		static final int RECEPTION_REPORT = 14;
		static final int CUSTODY_REPORT = 15;
		static final int FORWARD_REPORT = 16;
		static final int DELIVERY_REPORT = 17;
		static final int DELETION_REPORT = 18;
	}
	
	public Bundle(EID destination, long lifetime) {
		this.destination = destination;
		this.lifetime = lifetime;
		setPriority(Priority.NORMAL);
		if(destination instanceof SingletonEndpoint)
			setFlag(FlagOffset.DESTINATION_IS_SINGLETON, true);
	}
	
	Bundle() {
	}
	
	public void setPriority(Priority p) {
		procflags &= ~(0x11 << FlagOffset.PRIORITY);
		procflags |= p.ordinal() << FlagOffset.PRIORITY;
	}
	
	public Priority getPriority() {
		int priority = (int) (procflags >> FlagOffset.PRIORITY) & 0x11;
		// reduce the priority by modulo 3 since value 3 is not a valid priority
		return Priority.values()[priority % Priority.values().length];
	}
	
	/**
	 * Set a flag in the bundle
	 * @param flagOffset should be a constant from Bundle.FlagOffset
	 * @param value true to set the flag, false to clear it
	 */
	private void setFlag(int flagOffset, boolean value) {
		if(value)
			procflags |= 0x1 << flagOffset;
		else
			procflags &= ~(0x1 << flagOffset);
	}
	
	/**
	 * Append a new Block to this bundle
	 * @param block the block to append
	 */
	public void appendBlock(Block block)
	{
		if(blocks.size() > 0)
			blocks.getLast().procflags &= ~(0x1 << Block.FlagOffset.LAST_BLOCK.ordinal());
		block.procflags |= (0x1 << Block.FlagOffset.LAST_BLOCK.ordinal());
		blocks.addLast(block);
	}

	public String toString() {
		StringBuilder builder = new StringBuilder();
		builder.append("Bundle: ");
		builder.append(source);
		builder.append(" -> ");
		builder.append(destination);
		builder.append(" ");
		for(Block block : blocks) {
			builder.append("[");
			builder.append(block);
			builder.append("]");
		}
		return builder.toString();
	}
}
