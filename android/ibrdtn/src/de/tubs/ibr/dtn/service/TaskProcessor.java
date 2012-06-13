/*
 * TaskProcessor.java
 * 
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
package de.tubs.ibr.dtn.service;

import ibrdtn.api.ExtendedClient;

import java.util.LinkedList;
import java.util.Queue;

import android.util.Log;

public class TaskProcessor extends Thread {
	
	private final static String TAG = "TaskProcessor";

	private Boolean abort = false;
	private ExtendedClient client = null;
	
	// queue for waiting tasks
	private Queue<ClientTask> tasks = new LinkedList<ClientTask>();
	
	public TaskProcessor(ExtendedClient client)
	{
		this.client = client;
	}
	
	public synchronized void offer(ClientTask t)
	{
		tasks.offer(t);
		notify();
	}
	
	public synchronized void abort()
	{
		abort = true;
		interrupt();
	}
	
	@Override
	public void run()
	{
		while (!abort)
		{
			try {
				// start the bundle get loop
				while (!abort)
				{
					// get the next task of the queue
					ClientTask t = getTask();

					// run task
					t.run(client);
					
					// allow other threads to execute
					yield();
				}
			} catch (InterruptedException e) {
				if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "ServiceProcess has been interrupted");
			}
			
			// allow other threads to execute
			yield();
		}
	}
	
	private synchronized ClientTask getTask() throws InterruptedException
	{
		while (tasks.isEmpty())
		{
			// wait for a notify
			wait();

			// break out if we are aborted
			if (abort) throw new InterruptedException("aborted");
		}
		
		// get the next task of the queue
		return tasks.poll();
	}
}
