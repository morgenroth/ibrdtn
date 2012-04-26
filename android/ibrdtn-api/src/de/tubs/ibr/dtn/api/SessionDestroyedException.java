package de.tubs.ibr.dtn.api;

public class SessionDestroyedException extends Exception {

	/**
	 * serial for this exception
	 */
	private static final long serialVersionUID = -3111946764474786987L;

    public SessionDestroyedException() {}

    public SessionDestroyedException(Exception innerException) {
        this.innerException = innerException;
    }
    
	public SessionDestroyedException(String what) {
		this.what = null;
	}
	
    @Override
	public String toString() {
    	if ((innerException == null) && (what == null)) return super.toString();
    	return "SessionDestroyedException: " + ((innerException == null) ? what : innerException.toString());
	}

	public Exception innerException = null;
    public String what = null;
}
