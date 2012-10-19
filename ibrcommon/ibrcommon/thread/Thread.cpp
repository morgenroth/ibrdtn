/*
 * Thread.cpp
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "ibrcommon/config.h"
#include "ibrcommon/thread/Thread.h"
#include "ibrcommon/thread/MutexLock.h"
#include "ibrcommon/Logger.h"
#include <stdexcept>
#include <pthread.h>
#include <sys/times.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#ifdef __DEVELOPMENT_ASSERTIONS__
#include <cassert>
#endif

namespace ibrcommon
{
	void Thread::finalize_thread(void *obj)
	{
#ifdef __DEVELOPMENT_ASSERTIONS__
		// the given object should never be null
		assert(obj != NULL);
#endif
		// cast the thread object
		Thread *th = static_cast<Thread *>(obj);

		// set the state to finalizing, blocking all threads until this is done
		th->_state = THREAD_FINALIZING;

		// call the finally method
		th->finally();

		// set the state to DONE
		th->_state = THREAD_FINALIZED;

		// delete the thread-object is requested
		if (th->__delete_on_exit)
		{
			delete th;
		}
	}

	void* Thread::exec_thread(void *obj)
	{
#ifdef __DEVELOPMENT_ASSERTIONS__
		// the given object should never be null
		assert(obj != NULL);
#endif
		// cast the thread object
		Thread *th = static_cast<Thread *>(obj);

		// add cleanup-handler to finalize threads
		pthread_cleanup_push(Thread::finalize_thread, (void *)th);

		// set the state to RUNNING
		// enter the setup stage, if this thread is still in preparation state
		{
			ibrcommon::ThreadsafeState<THREAD_STATE>::Locked ls = th->_state.lock();
			if (ls != THREAD_PREPARE)
			{
				throw ibrcommon::Exception("Thread has been canceled.");
			}

			ls = THREAD_SETUP;
		}

		// call the setup method
		th->setup();

		// set the state to running
		th->_state = THREAD_RUNNING;

		// run threads run routine
		th->run();

		// remove cleanup-handler and call it
		pthread_cleanup_pop(1);

		// exit the thread
		Thread::exit();

		return NULL;
	}

	Thread::Thread(size_t size, bool delete_on_exit)
	 : _state(THREAD_INITIALIZED, THREAD_JOINED), tid(0), stack(size), priority(0), __delete_on_exit(delete_on_exit)
	{
		pthread_attr_init(&attr);
	}

	void Thread::yield(void)
	{
#if defined(HAVE_PTHREAD_YIELD_NP)
		pthread_yield_np();
#elif defined(HAVE_PTHREAD_YIELD)
		pthread_yield();
#else
		sched_yield();
#endif
	}

	void Thread::concurrency(int level)
	{
	#if defined(HAVE_PTHREAD_SETCONCURRENCY)
		pthread_setconcurrency(level);
	#endif
	}

	void Thread::sleep(size_t timeout)
	{
		timespec ts;
		ts.tv_sec = timeout / 1000l;
		ts.tv_nsec = (timeout % 1000l) * 1000000l;
	#if defined(HAVE_PTHREAD_DELAY)
		pthread_delay(&ts);
	#elif defined(HAVE_PTHREAD_DELAY_NP)
		pthread_delay_np(&ts);
	#else
		usleep(timeout * 1000);
	#endif
	}

	Thread::~Thread()
	{
		pthread_attr_destroy(&attr);
	}

	void Thread::detach(void)
	{
		pthread_detach(tid);
	}

	void Thread::exit(void)
	{
		pthread_exit(NULL);
	}

	int Thread::kill(int sig)
	{
		if (tid == 0) return -1;
		return pthread_kill(tid, sig);
	}

	void Thread::cancel() throw (ThreadException)
	{
		// block multiple cancel calls
		{
			ibrcommon::ThreadsafeState<THREAD_STATE>::Locked ls = _state.lock();

			// is this thread running?
			if (ls == THREAD_INITIALIZED)
			{
				// deny any futher startup of this thread
				ls = THREAD_JOINED;
				return;
			}

			if ( (ls == THREAD_PREPARE) || (ls == THREAD_SETUP) )
			{
				// wait until RUNNING, ERROR, JOINED or FINALIZED is reached
				ls.wait(THREAD_RUNNING | THREAD_JOINED | THREAD_ERROR | THREAD_FINALIZED | THREAD_CANCELLED);
			}

			// exit if the thread is not running
			if (ls != THREAD_RUNNING) return;

			// set state to cancelled
			ls = THREAD_CANCELLED;
		}

		// run custom cancellation
		__cancellation();
	}

	bool Thread::equal(pthread_t t1, pthread_t t2)
	{
		return (pthread_equal(t1, t2) != 0);
	}

	JoinableThread::JoinableThread(size_t size)
	 : Thread(size, false)
	{
	}

	JoinableThread::~JoinableThread()
	{
#ifdef __DEVELOPMENT_ASSERTIONS__
		// every thread should be joined, when the destructor is called.
		if ( (_state != THREAD_JOINED) && (_state != THREAD_ERROR) )
		{
			std::cerr << "FAILURE: Thread not joined! Current state:" << _state.get() << std::endl;
			assert( (_state != THREAD_JOINED) && (_state != THREAD_ERROR) );
		}
#endif
		join();
	}

	void JoinableThread::start(int adj) throw (ThreadException)
	{
		int ret;

		ibrcommon::ThreadsafeState<THREAD_STATE>::Locked ls = _state.lock();
		if (ls != THREAD_INITIALIZED) return;

		// set the thread state to PREPARE
		ls = THREAD_PREPARE;

		// set priority
		priority = adj;

#ifndef	__PTH__
		// modify the threads attributes - set as joinable thread
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

		// ignore scheduling policy and use the same as the parent
		pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
#endif
		// we typically use "stack 1" for min stack...
#ifdef	PTHREAD_STACK_MIN
		// if the given stack size is too small...
		if(stack && stack < PTHREAD_STACK_MIN)
		{
			// set it to the min stack size
			stack = PTHREAD_STACK_MIN;
		}
#else
		// if stack size if larger than zero and smaller than two...
		if (stack && stack < 2)
		{
			// set it to zero (we will not set the stack size)
			stack = 0;
		}
#endif

#ifdef	__PTH__
		// spawn a new thread
		tid = pth_spawn(PTH_ATTR_DEFAULT, &Thread::exec_thread, this);
#else
		// if the stack size is specified (> 0)
		if (stack)
		{
			// set the stack size attribute
			pthread_attr_setstacksize(&attr, stack);
		}

		// spawn a new thread
		ret = pthread_create(&tid, &attr, &Thread::exec_thread, this);

		
		switch (ret)
		{
		case EAGAIN:
			ls = THREAD_ERROR;
			throw ThreadException(ret, "The system lacked the necessary resources to create another thread, or the system-imposed limit on the total number of threads in a process PTHREAD_THREADS_MAX would be exceeded.");
		case EINVAL:
			ls = THREAD_ERROR;
			throw ThreadException(ret, "The value specified by attr is invalid.");
		case EPERM:
			ls = THREAD_ERROR;
			throw ThreadException(ret, "The caller does not have appropriate permission to set the required scheduling parameters or scheduling policy.");
		}
#endif
	}

	void JoinableThread::stop() throw (ThreadException)
	{
		// Cancel throws the exception of the terminated thread
		// so we have to catch any possible exception here
		try {
			Thread::cancel();
		} catch (const ThreadException&) {
			throw;
		} catch (const std::exception&) {
		}
	}

	void JoinableThread::join(void)
	{
		// first to some state checking
		{
			ibrcommon::ThreadsafeState<THREAD_STATE>::Locked ls = _state.lock();

			// if the thread never has been started: exit and deny any further startup
			if (ls == THREAD_INITIALIZED)
			{
				ls = THREAD_JOINED;
				return;
			}

			// wait until the finalized state is reached
			ls.wait(THREAD_FINALIZED | THREAD_JOINED | THREAD_ERROR);

			// do not join if an error occured
			if (ls == THREAD_ERROR) return;

			// if the thread has been joined already: exit
			if (ls == THREAD_JOINED) return;

			// get the thread-id of the calling thread
			pthread_t self = pthread_self();

			// if we try to join our own thread, just call Thread::exit()
			if (equal(tid, self))
			{
				Thread::exit();
				return;
			}
		}

		// if the thread has been started, do join
		if (pthread_join(tid, NULL) == 0)
		{
			// set the state to joined
			_state = THREAD_JOINED;
		}
		else
		{
			IBRCOMMON_LOGGER(error) << "Join on a thread failed[" << tid << "]" << IBRCOMMON_LOGGER_ENDL;
			_state = THREAD_ERROR;
		}
	}

	DetachedThread::DetachedThread(size_t size) : Thread(size, true)
	{
	}

	DetachedThread::~DetachedThread()
	{
	}

	void DetachedThread::start(int adj) throw (ThreadException)
	{
		int ret = 0;

		ibrcommon::ThreadsafeState<THREAD_STATE>::Locked ls = _state.lock();
		if (ls != THREAD_INITIALIZED) return;

		// set the thread state to PREPARE
		ls = THREAD_PREPARE;

		// set the priority
		priority = adj;

#ifndef	__PTH__
		// modify the threads attributes - set as detached thread
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		// ignore scheduling policy and use the same as the parent
		pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
#endif
		// we typically use "stack 1" for min stack...
#ifdef	PTHREAD_STACK_MIN
		// if the given stack size is too small...
		if(stack && stack < PTHREAD_STACK_MIN)
		{
			// set it to the min stack size
			stack = PTHREAD_STACK_MIN;
		}
#else
		// if stack size if larger than zero and smaller than two...
		if (stack && stack < 2)
		{
			// set it to zero (we will not set the stack size)
			stack = 0;
		}
#endif

#ifdef	__PTH__
		// spawn a new thread
		tid = pth_spawn(PTH_ATTR_DEFAULT, &Thread::exec_thread, this);
#else
		// if the stack size is specified (> 0)
		if (stack)
		{
			// set the stack size attribute
			pthread_attr_setstacksize(&attr, stack);
		}

		// spawn a new thread
		ret = pthread_create(&tid, &attr, &Thread::exec_thread, this);

		// check for errors
		switch (ret)
		{
		case EAGAIN:
			ls = THREAD_ERROR;
			throw ThreadException(ret, "The system lacked the necessary resources to create another thread, or the system-imposed limit on the total number of threads in a process PTHREAD_THREADS_MAX would be exceeded.");
		case EINVAL:
			ls = THREAD_ERROR;
			throw ThreadException(ret, "The value specified by attr is invalid.");
		case EPERM:
			ls = THREAD_ERROR;
			throw ThreadException(ret, "The caller does not have appropriate permission to set the required scheduling parameters or scheduling policy.");
		}
#endif
	}

	void DetachedThread::stop() throw (ThreadException)
	{
		// Cancel throws the exception of the terminated thread
		// so we have to catch any possible exception here
		try {
			Thread::cancel();
		} catch (const ThreadException&) {
			throw;
		} catch (const std::exception&) {
		}
	}
}
