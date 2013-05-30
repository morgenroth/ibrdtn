package de.tubs.ibr.dtn.dtalkie;

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
import de.tubs.ibr.dtn.dtalkie.db.Message;

public class MessageAdapter  extends CursorAdapter {

    @SuppressWarnings("unused")
    private final static String TAG = "MessageView";
    
    private LayoutInflater mInflater = null;
    private Context mContext = null;
    
    public static final String[] PROJECTION = new String[] {
        BaseColumns._ID,
        Message.SOURCE,
        Message.DESTINATION,
        Message.RECEIVED,
        Message.CREATED,
        Message.FILENAME,
        Message.PRIORITY,
        Message.MARKED
    };
    
    // The indexes of the default columns which must be consistent
    // with above PROJECTION.
    static final int COLUMN_MESSAGE_ID          = 0;
    static final int COLUMN_MESSAGE_SOURCE      = 1;
    static final int COLUMN_MESSAGE_DESTINATION = 2;
    static final int COLUMN_MESSAGE_RECEIVED    = 3;
    static final int COLUMN_MESSAGE_CREATED     = 4;
    static final int COLUMN_MESSAGE_FILENAME    = 5;
    static final int COLUMN_MESSAGE_PRIORITY    = 6;
    static final int COLUMN_MESSAGE_MARKED      = 7;
    
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
        public int mColumnSource;
        public int mColumnDestination;
        public int mColumnReceived;
        public int mColumnCreated;
        public int mColumnFilename;
        public int mColumnPriority;
        public int mColumnMarked;

        public ColumnsMap() {
            mColumnId          = COLUMN_MESSAGE_ID;
            mColumnSource      = COLUMN_MESSAGE_SOURCE;
            mColumnDestination = COLUMN_MESSAGE_DESTINATION;
            mColumnReceived    = COLUMN_MESSAGE_RECEIVED;
            mColumnCreated     = COLUMN_MESSAGE_CREATED;
            mColumnFilename    = COLUMN_MESSAGE_FILENAME;
            mColumnPriority    = COLUMN_MESSAGE_PRIORITY;
            mColumnMarked      = COLUMN_MESSAGE_MARKED;
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
                mColumnSource = cursor.getColumnIndexOrThrow(Message.SOURCE);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
                mColumnDestination = cursor.getColumnIndexOrThrow(Message.DESTINATION);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
                mColumnReceived = cursor.getColumnIndexOrThrow(Message.RECEIVED);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
                mColumnCreated = cursor.getColumnIndexOrThrow(Message.CREATED);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
                mColumnFilename = cursor.getColumnIndexOrThrow(Message.FILENAME);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }

            try {
                mColumnPriority = cursor.getColumnIndexOrThrow(Message.PRIORITY);
            } catch (IllegalArgumentException e) {
                Log.w("colsMap", e.getMessage());
            }
            
            try {
                mColumnMarked = cursor.getColumnIndexOrThrow(Message.MARKED);
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
        String type = cursor.getString(COLUMN_MESSAGE_MARKED);
        if ("in".equals(type)) {
            return 1;
        } else {
            return 0;
        }
    }
    
    @Override
    public int getViewTypeCount() {
        return 2;   // marked and non-marked messages
    }

    @Override
    public void bindView(View view, Context context, Cursor cursor) {
        if (view instanceof MessageItem) {
            long msgId = cursor.getLong(COLUMN_MESSAGE_ID);

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
            view = mInflater.inflate(R.layout.message_item_marked, parent, false);
        } else {
            view = mInflater.inflate(R.layout.message_item, parent, false);
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
