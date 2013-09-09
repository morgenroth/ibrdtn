/*
 * MessageView.java
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

import android.content.Context;
import android.database.Cursor;
import android.provider.BaseColumns;
import android.support.v4.util.LruCache;
import android.support.v4.widget.CursorAdapter;
import android.text.format.DateUtils;
import android.text.format.Time;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import de.tubs.ibr.dtn.chat.core.Message;

public class MessageAdapter extends CursorAdapter {

	@SuppressWarnings("unused")
	private final static String TAG = "MessageAdapter";
	
	private LayoutInflater mInflater = null;
	private Context mContext = null;
	
    public static final String[] PROJECTION = new String[] {
    	BaseColumns._ID,
    	Message.BUDDY,
    	Message.DIRECTION,
    	Message.CREATED,
    	Message.RECEIVED,
    	Message.PAYLOAD,
    	Message.SENTID,
    	Message.FLAGS
    };
	
    // The indexes of the default columns which must be consistent
    // with above PROJECTION.
    static final int COLUMN_MESSAGE_ID        = 0;
    static final int COLUMN_MESSAGE_BUDDY     = 1;
    static final int COLUMN_MESSAGE_DIRECTION = 2;
    static final int COLUMN_MESSAGE_CREATED   = 3;
    static final int COLUMN_MESSAGE_RECEIVED  = 4;
    static final int COLUMN_MESSAGE_PAYLOAD   = 5;
    static final int COLUMN_MESSAGE_SENTID    = 6;
    static final int COLUMN_MESSAGE_FLAGS     = 7;
    
    private static final int CACHE_SIZE     = 50;

    private final MessageCache mMessageCache;
    private final ColumnsMap mColumnsMap;
    
	public MessageAdapter(Context context, Cursor c, ColumnsMap cmap)
	{
		super(context, c, FLAG_REGISTER_CONTENT_OBSERVER);
		this.mContext = context;
		this.mInflater = LayoutInflater.from(context);
		
		mMessageCache = new MessageCache(CACHE_SIZE);
		mColumnsMap = cmap;
	}
	
    public static class ColumnsMap {
        public int mColumnId;
        public int mColumnBuddy;
        public int mColumnDirection;
        public int mColumnCreated;
        public int mColumnReceived;
        public int mColumnPayload;
        public int mColumnSendId;
        public int mColumnFlags;

        public ColumnsMap() {
        	mColumnId        = COLUMN_MESSAGE_ID;
        	mColumnBuddy     = COLUMN_MESSAGE_BUDDY;
        	mColumnDirection = COLUMN_MESSAGE_DIRECTION;
        	mColumnCreated   = COLUMN_MESSAGE_CREATED;
        	mColumnReceived  = COLUMN_MESSAGE_RECEIVED;
        	mColumnPayload   = COLUMN_MESSAGE_PAYLOAD;
        	mColumnSendId    = COLUMN_MESSAGE_SENTID;
        	mColumnFlags     = COLUMN_MESSAGE_FLAGS;
        }

        public ColumnsMap(Cursor cursor) {
            // Ignore all 'not found' exceptions since the custom columns
            // may be just a subset of the default columns.
            try {
            	mColumnId = cursor.getColumnIndexOrThrow(BaseColumns._ID);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
            	mColumnBuddy = cursor.getColumnIndexOrThrow(Message.BUDDY);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
            	mColumnDirection = cursor.getColumnIndexOrThrow(Message.DIRECTION);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
            	mColumnCreated = cursor.getColumnIndexOrThrow(Message.CREATED);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
            	mColumnReceived = cursor.getColumnIndexOrThrow(Message.RECEIVED);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
            	mColumnPayload = cursor.getColumnIndexOrThrow(Message.PAYLOAD);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
            	mColumnSendId = cursor.getColumnIndexOrThrow(Message.SENTID);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }
            
            try {
            	mColumnFlags = cursor.getColumnIndexOrThrow(Message.FLAGS);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }
        }
    }
    
    @Override
	public void notifyDataSetChanged() {
		super.notifyDataSetChanged();
		mMessageCache.evictAll();
	}

	public static String formatTimeStampString(Context context, long when) {
        return formatTimeStampString(context, when, false);
    }

    public static String formatTimeStampString(Context context, long when, boolean fullFormat) {
        Time then = new Time();
        then.set(when);
        Time now = new Time();
        now.setToNow();

        // Basic settings for formatDateTime() we want for all cases.
        int format_flags = DateUtils.FORMAT_ABBREV_ALL;

        // If the message is from a different year, show the date and year.
        if (then.year != now.year) {
            format_flags |= DateUtils.FORMAT_SHOW_YEAR | DateUtils.FORMAT_SHOW_DATE;
        } else if (then.yearDay != now.yearDay) {
            // If it is from a different day than today, show only the date.
            format_flags |= DateUtils.FORMAT_SHOW_DATE;
        } else {
            // Otherwise, if the message is from today, show the time.
            format_flags |= DateUtils.FORMAT_SHOW_TIME;
        }

        // If the caller has asked for full details, make sure to show the date
        // and time no matter what we've determined above (but still make showing
        // the year only happen if it is a different year from today).
        if (fullFormat) {
            format_flags |= (DateUtils.FORMAT_SHOW_DATE | DateUtils.FORMAT_SHOW_TIME);
        }

        return DateUtils.formatDateTime(context, when, format_flags);
    }
    
    @Override
    public int getItemViewType(int position) {
        Cursor cursor = (Cursor)getItem(position);
        return getItemViewType(cursor);
    }

    private int getItemViewType(Cursor cursor) {
        String type = cursor.getString(COLUMN_MESSAGE_DIRECTION);
        if ("in".equals(type)) {
            return 1;
        } else {
            return 0;
        }
    }
    
    @Override
    public int getViewTypeCount() {
        return 2;   // Incoming and outgoing messages
    }

	@Override
	public void bindView(View view, Context context, Cursor cursor) {
        if (view instanceof MessageItem) {
            long msgId = cursor.getLong(mColumnsMap.mColumnId);

            Message msg = getCachedMessage(msgId, cursor);
            if (msg != null) {
            	MessageItem mli = (MessageItem) view;
                int position = cursor.getPosition();
                mli.bind(msg, position);
            }
        }
	}

	@Override
	public View newView(Context context, Cursor cursor, ViewGroup parent) {
		View view = null;
		if (getItemViewType(cursor) == 1) {
			view = mInflater.inflate(R.layout.chat_item_recv, parent, false);
		} else {
			view = mInflater.inflate(R.layout.chat_item_send, parent, false);
		}
        return view;
	}
	
    private boolean isCursorValid(Cursor cursor) {
        // Check whether the cursor is valid or not.
        if (cursor == null || cursor.isClosed() || cursor.isBeforeFirst() || cursor.isAfterLast()) {
            return false;
        }
        return true;
    }
	
    public Message getCachedMessage(long msgId, Cursor c) {
        Message item = mMessageCache.get(msgId);
        if (item == null && c != null && isCursorValid(c)) {
            item = new Message(mContext, c, mColumnsMap);
            mMessageCache.put(msgId, item);
        }
        return item;
    }
    
    private static class MessageCache extends LruCache<Long, Message> {
        public MessageCache(int maxSize) {
            super(maxSize);
        }

        @Override
        protected void entryRemoved(boolean evicted, Long key,
        		Message oldValue, Message newValue) {
            //oldValue.cancelPduLoading();
        }
    }
}
