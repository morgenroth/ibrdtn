package ibrdtn.api;

import java.util.Map;

public class Event {
	
	private String _action = null;
	private String _name = null;
	private Map<String, String> _attributes = null;
	
	public Event(String name, String action, Map<String, String> attrs)
	{
		this._name = name;
		this._action = action;
		this._attributes = attrs;
	}

	public String getAction() {
		return _action;
	}

//	public void setAction(String action) {
//		this._action = action;
//	}

	public String getName() {
		return _name;
	}
	
//	public void setName(String name) {
//	this._name = name;
//}

	public Map<String, String> getAttributes() {
		return _attributes;
	}

	public void setAttributes(Map<String, String> attributes) {
		this._attributes = attributes;
	}
}
