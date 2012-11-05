/*
 * ibrdtnd.h
 *
 *  Created on: 05.11.2012
 *      Author: morgenro
 */

#ifndef IBRDTND_H_
#define IBRDTND_H_

/**
 * Initialize the daemon modules and components
 * @return 0 on success
 */
extern int ibrdtn_daemon_initialize();

/**
 * Execute the daemon main loop. This starts all the
 * components and blocks until the daemon is shut-down.
 * @return 0 after a clean shut-down
 */
extern int ibrdtn_daemon_main_loop();

/**
 * Shut-down the daemon. The call returns immediately.
 * @return 0 on success
 */
extern int ibrdtn_daemon_shutdown();

/**
 * Enable or disable debugging at runtime
 * @val Set val = true to enable debugging.
 * @return 0 on success
 */
extern int ibrdtn_daemon_runtime_debug(bool val);

/**
 * Generate a reload signal.
 * @return 0 on success
 */
extern int ibrdtn_daemon_reload();


#endif /* IBRDTND_H_ */
