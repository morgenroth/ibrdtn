/*
 * MainActivity.java
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
package de.tubs.ibr.dtn.chat;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.RemoteInput;
import de.tubs.ibr.dtn.chat.service.ChatService;

public class ReplyActivity extends Activity {

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
	
		Bundle remoteInput = RemoteInput.getResultsFromIntent(getIntent());
		Long buddyId = getIntent().getLongExtra(ChatService.EXTRA_BUDDY_ID, -1L);
		
		if (remoteInput != null && buddyId >= 0) {
			final Intent intent = new Intent(this, ChatService.class);
			intent.setAction(ChatService.ACTION_SEND_MESSAGE);
			intent.putExtra(ChatService.EXTRA_BUDDY_ID, buddyId);
			intent.putExtra(ChatService.EXTRA_TEXT_BODY, remoteInput.getCharSequence(ChatService.EXTRA_VOICE_REPLY).toString());
			startService(intent);
			
			ChatService.cancelNotification(this, buddyId);
		}
		
		this.finish();
	}
}
