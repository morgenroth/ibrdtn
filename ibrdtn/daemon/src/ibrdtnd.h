/*
 * ibrdtnd.h
 *
 *  Created on: 05.11.2012
 *      Author: morgenro
 */

#ifndef IBRDTND_H_
#define IBRDTND_H_

/**
 * States:
 * 0 = zero initialization (occurs directly after calling ibrdtn_daemon_initialize)
 * 1 = all components are created
 * 2 = all components are initialized
 * 3 = startup routine completed
 * 4 = main-loop returned
 * 5 = all components terminated
 * 6 = all components removed
 */
typedef void (*state_callback)(int state);

/**
 * Initialize the daemon modules and components
 * @return 0 on success
 */
extern int ibrdtn_daemon_initialize(state_callback cb);

/**
 * Execute the daemon main loop. This starts all the
 * components and blocks until the daemon is shut-down.
 * @return 0 after a clean shut-down
 */
extern int ibrdtn_daemon_main_loop(state_callback cb);

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
