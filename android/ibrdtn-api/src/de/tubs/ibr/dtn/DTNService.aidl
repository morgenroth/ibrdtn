package de.tubs.ibr.dtn;
import de.tubs.ibr.dtn.DaemonState;
import de.tubs.ibr.dtn.api.DTNSession;

interface DTNService {
    /**
     * Get the state of the daemon.
     */
	DaemonState getState();
	
	/**
	 * Determine if the daemon is running or not.
	 */
	boolean isRunning();
	
	/**
	 * Returns the log in the ring-buffer of the service.
	 */
	List<String> getLog();
	
	/**
	 * Returns the available neighbors of the daemon.
	 */
	List<String> getNeighbors();
	
	/**
	 * Deletes all bundles in the storage of the daemon.
	 * (Do not work if the daemon is running.)
	 */
	void clearStorage();
	
	/**
	 * Get a previously created session.
	 * @returns null if the session is not available.
	 */
	DTNSession getSession(String packageName);
}
