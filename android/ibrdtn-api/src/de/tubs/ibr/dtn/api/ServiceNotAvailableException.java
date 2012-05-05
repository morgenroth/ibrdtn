package de.tubs.ibr.dtn.api;

public class ServiceNotAvailableException extends Exception {

	/**
	 * 
	 */
	private static final long serialVersionUID = 876730276266296815L;
	
	public ServiceNotAvailableException() {}

    public ServiceNotAvailableException(Exception innerException) {
        this.innerException = innerException;
    }
    
	public ServiceNotAvailableException(String what) {
		this.what = null;
	}
	
    @Override
	public String toString() {
    	if ((innerException == null) && (what == null)) return super.toString();
    	return "ServiceNotAvailableException: " + ((innerException == null) ? what : innerException.toString());
	}

	public Exception innerException = null;
    public String what = null;
}
