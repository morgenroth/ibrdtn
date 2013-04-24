/*
 * DataHandler.java
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

package de.tubs.ibr.dtn.api;

import android.os.ParcelFileDescriptor;

public interface DataHandler {
	public void startBundle(Bundle bundle);
	public void endBundle();
	public TransferMode startBlock(Block block);
	public void endBlock(); 
	public void payload(byte[] data);
	public ParcelFileDescriptor fd();
	public void progress(long current, long length);
	public void finished(int startId);
}
