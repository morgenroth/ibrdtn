package ibrdtn.api.object;

import java.io.IOException;
import java.io.OutputStream;


public abstract class Block extends BlockHeader {
	
	public static abstract class Data {
		/**
		 * Writes the data encapsulated by this class to an OutputStream
		 * @param stream the stream to write to
		 * @throws IOException if reading the data or writing to the stream failed
		 */
		abstract public void writeTo(OutputStream stream) throws IOException;
		/**
		 * @return The size of the data that will be written by writeTo()
		 */
		abstract public long size();
	}

	/**
	 * Create a new block from a byte array
	 * @param type The block type
	 * @param flags The flags of the block
	 * @param data the data
	 * @return the created block
	 * @throws InvalidDataException if there was not enough data to create a block of the given type
	 */
	public static Block createBlock(int type, long flags, byte[] data) throws InvalidDataException {
		return createBlock(type, flags, new ByteArrayBlockData(data));
	}
	
	/**
	 * Create a new Block from a Block.Data object
	 * @param type The block type
	 * @param flags The flags of the block
	 * @param data the data
	 * @return the created block
	 * @throws InvalidDataException if there was not enough data to create a block of the given type
	 */
	public static Block createBlock(int type, long flags, Data data) throws InvalidDataException {
		Block block;

		switch(type){
		case PayloadBlock.type:
			block = new PayloadBlock(data);
			break;
		case ScopeControlHopLimitBlock.type:
			block = new ScopeControlHopLimitBlock(data);
			break;
		default:
			block = new ExtensionBlock(type, data);
			break;
		}
		
		block.setFlags(flags);
		return block;
	}

	/**
	 * Create a new Block from a Block.Data object and a block header
	 * @param header The block header
	 * @param data the data
	 * @return the created block
	 * @throws InvalidDataException if there was not enough data to create a block of the given type
	 */
	public static Block createBlock(BlockHeader header, Data data) throws InvalidDataException {
		Block block = createBlock(header.getType(), header.procflags, data);
		block.setEIDs(header._eids);
		return block;
	}

	protected Block(int type) {
		super(type);
	}
	
	abstract Data getData();
	
	public class InvalidDataException extends Exception {
		/**
		 * generated serialVersionUID
		 */
		private static final long serialVersionUID = 2111512395823250878L;

		public InvalidDataException(String msg) {
			super(msg);
		}
	}
}
