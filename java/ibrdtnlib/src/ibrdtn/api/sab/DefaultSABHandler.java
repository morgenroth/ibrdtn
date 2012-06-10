package ibrdtn.api.sab;

import java.util.List;

public class DefaultSABHandler implements SABHandler {

	@Override
	public void startStream() {
	}

	@Override
	public void endStream() {
	}

	@Override
	public void startBundle() {
	}

	@Override
	public void endBundle() {
	}

	@Override
	public void startBlock(Integer type) {
	}

	@Override
	public void endBlock() {
	}

	@Override
	public void attribute(String keyword, String value) {
	}

	@Override
	public void characters(String data) throws SABException {
	}

	@Override
	public void notify(Integer type, String data) {
	}

	@Override
	public void response(Integer type, String data) {
	}

	@Override
	public void response(List<String> data) {
	}

}
