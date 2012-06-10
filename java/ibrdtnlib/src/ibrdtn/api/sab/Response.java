package ibrdtn.api.sab;

public class Response {
	private int code;
	private String data;
	
	public Response(int code, String data) {
		this.code = code;
		this.data = data;
	}
	
	public int getCode() {
		return code;
	}
	
	public String getData() {
		return data;
	}
}
