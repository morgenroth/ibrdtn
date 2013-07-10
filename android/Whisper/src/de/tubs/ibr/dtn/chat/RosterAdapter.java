/*
 * RosterView.java
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

import android.annotation.SuppressLint;
import android.content.Context;
import android.database.Cursor;
import android.os.Build;
import android.provider.BaseColumns;
import android.support.v4.util.LruCache;
import android.support.v4.widget.CursorAdapter;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import de.tubs.ibr.dtn.chat.core.Buddy;

public class RosterAdapter extends CursorAdapter {

	@SuppressWarnings("unused")
	private final static String TAG = "RosterView";
	
	private LayoutInflater mInflater = null;
	private Context mContext = null;
	private Long mSelectedBuddyId = null;
	
    public static final String[] PROJECTION = new String[] {
    	BaseColumns._ID,
    	Buddy.NICKNAME,
    	Buddy.ENDPOINT,
    	Buddy.LASTSEEN,
    	Buddy.PRESENCE,
    	Buddy.STATUS,
    	Buddy.DRAFTMSG,
    	Buddy.VOICEEID,
    	Buddy.LANGUAGE,
    	Buddy.COUNTRY
    };
	
    // The indexes of the default columns which must be consistent
    // with above PROJECTION.
    static final int COLUMN_ROSTER_ID       = 0;
    static final int COLUMN_ROSTER_NICKNAME = 1;
    static final int COLUMN_ROSTER_ENDPOINT = 2;
    static final int COLUMN_ROSTER_LASTSEEN = 3;
    static final int COLUMN_ROSTER_PRESENCE = 4;
    static final int COLUMN_ROSTER_STATUS   = 5;
    static final int COLUMN_ROSTER_DRAFTMSG = 6;
    static final int COLUMN_ROSTER_VOICEEID = 7;
    static final int COLUMN_ROSTER_LANGUAGE = 8;
    static final int COLUMN_ROSTER_COUNTRY  = 9;
    
    private static final int CACHE_SIZE     = 50;
    
    private final BuddyCache mBuddyCache;
    private final ColumnsMap mColumnsMap;
	
	public RosterAdapter(Context context, Cursor c, ColumnsMap cmap)
	{
		super(context, c, FLAG_REGISTER_CONTENT_OBSERVER);
		this.mContext = context;
		this.mInflater = LayoutInflater.from(context);
		
		mBuddyCache = new BuddyCache(CACHE_SIZE);
		mColumnsMap = cmap;
	}
	
	@SuppressLint("NewApi")
	@Override
	public void bindView(View view, Context context, Cursor cursor) {
        if (view instanceof RosterItem) {
            long buddyId = cursor.getLong(mColumnsMap.mColumnId);

            Buddy buddy = getCachedBuddy(buddyId, cursor);
            if (buddy != null) {
            	RosterItem bli = (RosterItem) view;
                int position = cursor.getPosition();
                bli.bind(buddy, position);
                if (buddy.getId().equals(mSelectedBuddyId)) {
                	if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB) {
                		bli.setActivated(true);
                	}
                } else {
                	if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB) {
                		bli.setActivated(false);
                	}
                }
            }
        }
	}

	@Override
	public View newView(Context context, Cursor cursor, ViewGroup parent) {
        View view = mInflater.inflate(R.layout.roster_item, parent, false);
        return view;
	}
	
	public void setSelected(Long buddyId) {
		mSelectedBuddyId = buddyId;
		super.notifyDataSetChanged();
	}
	
    public static class ColumnsMap {
        public int mColumnId;
        public int mColumnNickname;
        public int mColumnEndpoint;
        public int mColumnLastseen;
        public int mColumnPresence;
        public int mColumnStatus;
        public int mColumnDraftMsg;
        public int mColumnVoiceEndpoint;
        public int mColumnLanguage;
        public int mColumnCountry;

        public ColumnsMap() {
        	mColumnId       = COLUMN_ROSTER_ID;
        	mColumnNickname = COLUMN_ROSTER_NICKNAME;
        	mColumnEndpoint = COLUMN_ROSTER_ENDPOINT;
        	mColumnLastseen = COLUMN_ROSTER_LASTSEEN;
        	mColumnPresence = COLUMN_ROSTER_PRESENCE;
        	mColumnStatus   = COLUMN_ROSTER_STATUS;
        	mColumnDraftMsg = COLUMN_ROSTER_DRAFTMSG;
        	mColumnVoiceEndpoint = COLUMN_ROSTER_VOICEEID;
        	mColumnLanguage = COLUMN_ROSTER_LANGUAGE;
        	mColumnCountry = COLUMN_ROSTER_COUNTRY;
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
            	mColumnNickname = cursor.getColumnIndexOrThrow(Buddy.NICKNAME);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
            	mColumnEndpoint = cursor.getColumnIndexOrThrow(Buddy.ENDPOINT);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
            	mColumnLastseen = cursor.getColumnIndexOrThrow(Buddy.LASTSEEN);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
            	mColumnPresence = cursor.getColumnIndexOrThrow(Buddy.PRESENCE);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
            	mColumnStatus = cursor.getColumnIndexOrThrow(Buddy.STATUS);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
            	mColumnDraftMsg = cursor.getColumnIndexOrThrow(Buddy.DRAFTMSG);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }
            
            try {
                mColumnVoiceEndpoint = cursor.getColumnIndexOrThrow(Buddy.VOICEEID);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }
            
            try {
                mColumnLanguage = cursor.getColumnIndexOrThrow(Buddy.LANGUAGE);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }
            
            try {
                mColumnCountry = cursor.getColumnIndexOrThrow(Buddy.COUNTRY);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }
        }
    }
    
    @Override
	public void notifyDataSetChanged() {
		super.notifyDataSetChanged();
		mBuddyCache.evictAll();
	}
    
    private boolean isCursorValid(Cursor cursor) {
        // Check whether the cursor is valid or not.
        if (cursor == null || cursor.isClosed() || cursor.isBeforeFirst() || cursor.isAfterLast()) {
            return false;
        }
        return true;
    }
    
    public Buddy getCachedBuddy(long buddyId, Cursor c) {
        Buddy item = mBuddyCache.get(buddyId);
        if (item == null && c != null && isCursorValid(c)) {
            item = new Buddy(mContext, c, mColumnsMap);
            mBuddyCache.put(buddyId, item);
        }
        return item;
    }
    
    private static class BuddyCache extends LruCache<Long, Buddy> {
        public BuddyCache(int maxSize) {
            super(maxSize);
        }

        @Override
        protected void entryRemoved(boolean evicted, Long key,
        		Buddy oldValue, Buddy newValue) {
            //oldValue.cancelPduLoading();
        }
    }
}
