/**
 * Simple API for Bundle Protocol (SAB)
 * This is a handler for Simple API events.
 * 
 * @author Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 */

package ibrdtn.api.sab;

import java.util.List;

public interface SABHandler {
	public void startStream();
	public void endStream();
	
	public void startBundle();
	public void endBundle();
	
	public void startBlock(Integer type);
	public void endBlock();
	
	public void attribute(String keyword, String value);
	public void characters(String data) throws SABException;
	public void notify(Integer type, String data);
	
	public void response(Integer type, String data);
	public void response(List<String> data);
}
