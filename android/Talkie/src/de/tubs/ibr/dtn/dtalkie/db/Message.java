package de.tubs.ibr.dtn.dtalkie.db;

import java.io.File;
import java.util.Date;

public class Message implements Comparable<Message> {
	
	private Long id = null;
	private String source = null;
	private String destination = null;
	private Date received = null;
	private Date created = null;
	private File file = null;
	private Integer priority = null;
	private Boolean marked = null;
	
	public Message(Long id) {
		this.id = id;
		this.created = new Date();
	}
	
	public Message(String source, String destination, File file) {
		this.source = source;
		this.destination = destination;
		this.file = file;
		this.created = new Date();
	}
	
	public String getSource() {
		return source;
	}
	
	public void setSource(String source) {
		this.source = source;
	}
	
	public String getDestination() {
		return destination;
	}
	
	public void setDestination(String destination) {
		this.destination = destination;
	}

	public File getFile() {
		return file;
	}

	public void setFile(File file) {
		this.file = file;
	}

	public Boolean isMarked() {
		if (marked == null) return false;
		return marked;
	}

	public void setMarked(Boolean marked) {
		this.marked = marked;
	}

	public Integer getPriority() {
		return priority;
	}

	public void setPriority(Integer priority) {
		this.priority = priority;
	}

	public Date getCreated() {
		return created;
	}

	public void setCreated(Date created) {
		this.created = created;
	}

	public Date getReceived() {
		return received;
	}

	public void setReceived(Date received) {
		this.received = received;
	}

	public Long getId() {
		return id;
	}
	
	public void setId(Long id) {
		this.id = id;
	}

	@Override
	public int compareTo(Message arg0) {
		if (this.created != null)
		{
			if (!this.created.equals(arg0.created))
				return this.created.compareTo(arg0.created);
		}
		
		if (this.received != null)
		{
			if (!this.received.equals(arg0.created))
				return this.received.compareTo(arg0.created);
		}
		
		if (this.source != null)
		{
			if (!this.source.equals(arg0.source))
				return this.source.compareTo(arg0.source);
		}
		
		if (this.priority != null)
		{
			if (!this.priority.equals(arg0.priority))
				return this.priority.compareTo(arg0.priority);
		}
		
		if (this.file != null)
		{
			if (!this.file.equals(arg0.file))
				return this.file.compareTo(arg0.file);
		}

		return 0;
	}
}
