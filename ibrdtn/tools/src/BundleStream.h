/*
 * BundleStream.h
 *
 *  Created on: 23.03.2011
 *      Author: morgenro
 */

#ifndef BUNDLESTREAM_H_
#define BUNDLESTREAM_H_

#include <ibrdtn/api/Bundle.h>
#include <ibrdtn/api/Client.h>
#include <ibrdtn/data/EID.h>
#include <ibrdtn/data/SDNV.h>
#include <ibrdtn/data/StreamBlock.h>
#include <ibrcommon/net/tcpclient.h>

class StreamBundle : public dtn::api::Bundle
{
public:
	StreamBundle();
	StreamBundle(const dtn::api::Bundle &b);
	virtual ~StreamBundle();

	/**
	 * Append data to the payload.
	 */
	void append(const char* data, size_t length);

	/**
	 * deletes the hole payload of the bundle
	 */
	void clear();

	/**
	 * returns the size of the current payload
	 * @return
	 */
	size_t size();

	static size_t getSequenceNumber(const StreamBundle &b);

private:
	// reference to the BLOB where all data is stored until transmission
	ibrcommon::BLOB::Reference _ref;
};

class BundleStreamBuf : public std::basic_streambuf<char, std::char_traits<char> >
{
public:
	// The size of the input and output buffers.
	static const size_t BUFF_SIZE = 5120;

	BundleStreamBuf(dtn::api::Client &client, StreamBundle &chunk, size_t buffer = 4096, bool wait_seq_zero = false);
	virtual ~BundleStreamBuf();

	virtual void received(const dtn::api::Bundle &b);

protected:
	virtual int sync();
	virtual std::char_traits<char>::int_type overflow(std::char_traits<char>::int_type = std::char_traits<char>::eof());
	virtual std::char_traits<char>::int_type underflow();
	std::char_traits<char>::int_type __underflow();

private:
	class Chunk
	{
	public:
		Chunk(const dtn::api::Bundle &b);
		virtual ~Chunk();

		bool operator==(const Chunk& other) const;
		bool operator<(const Chunk& other) const;

		dtn::api::Bundle _bundle;
		size_t _seq;
	};

	// Input buffer
	char *_in_buf;
	// Output buffer
	char *_out_buf;

	dtn::api::Client &_client;
	StreamBundle &_chunk;
	size_t _buffer;

	ibrcommon::Conditional _chunks_cond;
	std::set<Chunk> _chunks;
	size_t _chunk_offset;

	size_t _in_seq;
	bool _streaming;
};

class BundleStream : public dtn::api::Client
{
public:
	BundleStream(ibrcommon::tcpstream &stream, size_t chunk_size, const std::string &app = "stream", const dtn::data::EID &group = dtn::data::EID(), bool wait_seq_zero = false);
	virtual ~BundleStream();

	BundleStreamBuf& rdbuf();
	dtn::api::Bundle& base();

protected:
	virtual void received(const dtn::api::Bundle &b);

private:
	ibrcommon::tcpstream &_stream;

	BundleStreamBuf _buf;
	StreamBundle _chunk;
};

#endif /* BUNDLESTREAM_H_ */
