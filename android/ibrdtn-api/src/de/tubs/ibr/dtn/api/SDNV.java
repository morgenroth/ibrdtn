/**
 * copied from teamfrednet-mockups
 * http://code.google.com/p/teamfrednet-mockups/
 *
 */
package de.tubs.ibr.dtn.api;

import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class SDNV extends Object
{
    public final static int Read(InputStream in)
        throws IOException
    {
        int value = 0;
        for (int cc = 3; ; cc--){
            int u8 = in.read();
            if (-1 == u8)
                throw new EOFException();
            else {
                int u7 = (u8 & 0x7f);
                value |= (u7 << (cc*7));
                if (0 == cc || u7 == u8)
                    return value;
            }
        }
    }
    public final static void Write(OutputStream out, int value)
        throws IOException
    {
        boolean inline = false;
        int bb;
        for (int cc = 3; ; cc--){
            bb = ((value >>> (cc*7)) & 0x7f);
            if (inline || 0 != bb){
                inline = true;
                if (0 == cc)
                    out.write(bb);
                else {
                    bb |= 0x80;
                    out.write(bb);
                }
            }
        }
    }
    
	/** 
	 * Converts max 8-byte number into an SDNV
	 * 
	 * @param val
	 * @return
	 */
	public static byte[] longToSDNV(long val) {
		if (val==0) {
			byte[] ret = new byte[1];
			ret[0] = 0x00;
			return ret;
		}
		
		// Count how many bits we have
		long tmp = val;
		int count=64;
		while ((tmp>>63)==0) {
			count--;
			tmp = tmp << 1;
		}
		int num_bytes = (int)(Math.ceil(count / 7.0));
		
		
		// Build the SDNV
		byte[] bytes = new byte[num_bytes];
		for (int i=0; i<num_bytes; i++) {
			bytes[num_bytes-i-1] = (byte)(val & 0x7f);
			if (i!=0) bytes[num_bytes-i-1] |= 0x80;
			val = val >> 7;
		}
		return bytes;
	}
	
	/** 
	 * 
	 * @param in
	 * @param offset
	 * @return array with the value in index 0 and length of read bytes in 1
	 * @throws Exception
	 */
	public static long[] parseSDNV(byte[] in, int offset) throws Exception {
        long val=0;
        long t;
        boolean cont=true;
        
        int i = offset;
        int read = 0;
        while (cont) {
        	if (i>in.length) throw new Exception();
            t = in[i];
            i++; read++;
            t &= 0xff;                  // make sure there are no extra bits
            cont = ( (t&0x80) != 0);    // check MSB
            t &= 0x7f;                  // get the 7 data bits
            val = val << 7;           	// make room for the data bits
            val |= t;                   // add the data bits
        }
        
        long[] ret = {val, read};
        return ret;
    }
	
	public static long parseSDNV(InputStream in) {
		long val=0;
		int t;
		boolean cont=true;
		
		while (cont) {
			try {
				t = in.read();
			} catch (IOException ex) {
				return -1;
			}
			t &= 0xff;                  // make sure there are no extra bits
			cont = ( (t&0x80) != 0);    // check MSB
			t &= 0x7f;                  // get the 7 data bits
			val = val << 7;           	// make room for the data bits
			val |= t;                   // add the data bits
		}

		return val;
	}
}
