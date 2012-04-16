/*
 * dtnstream.cpp
 *
 * The application dtnstream can transfer a data stream to another instance of dtnstream.
 * It uses an extension block to mark the sequence of the stream.
 *
 *  Created on: 23.03.2011
 *      Author: morgenro
 */

#include "config.h"
#include "BundleStream.h"
#include <ibrdtn/api/Client.h>
#include <ibrdtn/data/EID.h>
#include <ibrdtn/data/PayloadBlock.h>
#include <ibrcommon/net/tcpclient.h>
#include <ibrcommon/data/File.h>
#include <ibrcommon/TimeMeasurement.h>
#include <iostream>
#include <unistd.h>

unsigned int __timeout_receive__ = 0;

StreamBundle::StreamBundle()
 : _ref(ibrcommon::BLOB::create())
{
	StreamBlock &block = _b.push_front<StreamBlock>();
	block.setSequenceNumber(0);

	_b.push_back(_ref);
}

StreamBundle::StreamBundle(const dtn::api::Bundle &b)
 : dtn::api::Bundle(b), _ref(getData())
{
}

StreamBundle::~StreamBundle()
{
}

void StreamBundle::append(const char* data, size_t length)
{
	ibrcommon::BLOB::iostream stream = _ref.iostream();
	(*stream).seekp(0, ios::end);
	(*stream).write(data, length);
}

void StreamBundle::clear()
{
	ibrcommon::BLOB::iostream stream = _ref.iostream();
	stream.clear();

	// increment the sequence number
	try {
		StreamBlock &block = _b.getBlock<StreamBlock>();
		block.setSequenceNumber(block.getSequenceNumber() + 1);
	} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) { };
}

size_t StreamBundle::size()
{
	ibrcommon::BLOB::iostream stream = _ref.iostream();
	return stream.size();
}

size_t StreamBundle::getSequenceNumber(const StreamBundle &b)
{
	try {
		const StreamBlock &block = b._b.getBlock<StreamBlock>();
		return block.getSequenceNumber();
	} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) { }

	return 0;
}

BundleStreamBuf::BundleStreamBuf(dtn::api::Client &client, StreamBundle &chunk, size_t buffer, bool wait_seq_zero)
 : _in_buf(new char[BUFF_SIZE]), _out_buf(new char[BUFF_SIZE]), _client(client), _chunk(chunk),
   _buffer(buffer), _chunk_offset(0), _in_seq(0), _streaming(wait_seq_zero)
{
	// Initialize get pointer.  This should be zero so that underflow is called upon first read.
	setg(0, 0, 0);
	setp(_in_buf, _in_buf + BUFF_SIZE - 1);
};

BundleStreamBuf::~BundleStreamBuf()
{
	delete[] _in_buf;
	delete[] _out_buf;
};

int BundleStreamBuf::sync()
{
	int ret = std::char_traits<char>::eq_int_type(this->overflow(
			std::char_traits<char>::eof()), std::char_traits<char>::eof()) ? -1
			: 0;

	// send the current chunk and clear it
	_client << _chunk; _client.flush();
	_chunk.clear();

	return ret;
}

std::char_traits<char>::int_type BundleStreamBuf::overflow(std::char_traits<char>::int_type c)
{
	char *ibegin = _in_buf;
	char *iend = pptr();

	// mark the buffer as free
	setp(_in_buf, _in_buf + BUFF_SIZE - 1);

	if (!std::char_traits<char>::eq_int_type(c, std::char_traits<char>::eof()))
	{
		*iend++ = std::char_traits<char>::to_char_type(c);
	}

	// if there is nothing to send, just return
	if ((iend - ibegin) == 0)
	{
		return std::char_traits<char>::not_eof(c);
	}

	// copy data into the bundles payload
	_chunk.append(_in_buf, iend - ibegin);

	// if size exceeds chunk limit, send it
	if (_chunk.size() > _buffer)
	{
		_client << _chunk; _client.flush();
		_chunk.clear();
	}

	return std::char_traits<char>::not_eof(c);
}

void BundleStreamBuf::received(const dtn::api::Bundle &b)
{
	ibrcommon::MutexLock l(_chunks_cond);

	if (StreamBundle::getSequenceNumber(b) < _in_seq) return;

	_chunks.insert(Chunk(b));
	_chunks_cond.signal(true);

	// bundle received
//	std::cerr << ". " << StreamBundle::getSequenceNumber(b) << std::flush;
}

std::char_traits<char>::int_type BundleStreamBuf::underflow()
{
	ibrcommon::MutexLock l(_chunks_cond);

	return __underflow();
}

std::char_traits<char>::int_type BundleStreamBuf::__underflow()
{
	// receive chunks until the next sequence number is received
	while (_chunks.empty())
	{
		// wait for the next bundle
		_chunks_cond.wait();
	}

	ibrcommon::TimeMeasurement tm;
	tm.start();

	// while not the right sequence number received -> wait
	while ((_in_seq != (*_chunks.begin())._seq))
	{
		try {
			// wait for the next bundle
			_chunks_cond.wait(1000);
		} catch (const ibrcommon::Conditional::ConditionalAbortException&) { };

		tm.stop();
		if (((__timeout_receive__ > 0) && (tm.getSeconds() > __timeout_receive__)) || !_streaming)
		{
			// skip the missing bundles and proceed with the last received one
			_in_seq = (*_chunks.begin())._seq;

			// set streaming to active
			_streaming = true;
		}
	}

	// get the first chunk in the buffer
	const Chunk &c = (*_chunks.begin());

	dtn::api::Bundle b = c._bundle;
	ibrcommon::BLOB::Reference r = b.getData();

	// get stream lock
	ibrcommon::BLOB::iostream stream = r.iostream();

	// jump to the offset position
	(*stream).seekg(_chunk_offset, ios::beg);

	// copy the data of the last received bundle into the buffer
	(*stream).read(_out_buf, BUFF_SIZE);

	// get the read bytes
	size_t bytes = (*stream).gcount();

	if ((*stream).eof())
	{
		// bundle consumed
//		std::cerr << std::endl << "# " << c._seq << std::endl << std::flush;

		// delete the last chunk
		_chunks.erase(c);

		// reset the chunk offset
		_chunk_offset = 0;

		// increment sequence number
		_in_seq++;

		// if no more bytes are read, get the next bundle -> call underflow() recursive
		if (bytes == 0)
		{
			return __underflow();
		}
	}
	else
	{
		// increment the chunk offset
		_chunk_offset += bytes;
	}
	
	// Since the input buffer content is now valid (or is new)
	// the get pointer should be initialized (or reset).
	setg(_out_buf, _out_buf, _out_buf + bytes);

	return std::char_traits<char>::not_eof((unsigned char) _out_buf[0]);
}

BundleStreamBuf::Chunk::Chunk(const dtn::api::Bundle &b)
 : _bundle(b), _seq(StreamBundle::getSequenceNumber(b))
{
}

BundleStreamBuf::Chunk::~Chunk()
{
}

bool BundleStreamBuf::Chunk::operator==(const Chunk& other) const
{
	return (_seq == other._seq);
}

bool BundleStreamBuf::Chunk::operator<(const Chunk& other) const
{
	return (_seq < other._seq);
}

BundleStream::BundleStream(ibrcommon::tcpstream &stream, size_t chunk_size, const std::string &app, const dtn::data::EID &group, bool wait_seq_zero)
 : dtn::api::Client(app, group, stream), _stream(stream), _buf(*this, _chunk, chunk_size, wait_seq_zero)
{};

BundleStream::~BundleStream() {};

BundleStreamBuf& BundleStream::rdbuf()
{
	return _buf;
}

dtn::api::Bundle& BundleStream::base()
{
	return _chunk;
}

void BundleStream::received(const dtn::api::Bundle &b)
{
	_buf.received(b);
}

void print_help()
{
	std::cout << "-- dtnstream (IBR-DTN) --" << std::endl;
	std::cout << "Syntax: dtnstream [options]"  << std::endl;
	std::cout << "" << std::endl;
	std::cout << "* common options *" << std::endl;
	std::cout << " -h               display this text" << std::endl;
	std::cout << " -U <socket>      use UNIX domain sockets" << std::endl;
	std::cout << " -s <identifier>  set the source identifier (e.g. stream)" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "* send options *" << std::endl;
	std::cout << " -d <destination> set the destination eid (e.g. dtn://node/stream)" << std::endl;
	std::cout << " -G               destination is a group" << std::endl;
	std::cout << " -c <bytes>       set the chunk size (max. size of each bundle)" << std::endl;
	std::cout << " -l <seconds>     set the lifetime of stream chunks default: 30" << std::endl;
	std::cout << " -E               request encryption on the bundle layer" << std::endl;
	std::cout << " -S               request signature on the bundle layer" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "* receive options *" << std::endl;
	std::cout << " -g <group>       join a destination group" << std::endl;
	std::cout << " -t <seconds>     set the timeout of the buffer" << std::endl;
	std::cout << " -w               wait for the bundle with seq zero" << std::endl;
	std::cout << "" << std::endl;
}

int main(int argc, char *argv[])
{
	int opt = 0;
	dtn::data::EID _destination;
	std::string _source = "stream";
	unsigned int _lifetime = 30;
	size_t _chunk_size = 4096;
	dtn::data::EID _group;
	bool _bundle_encryption = false;
	bool _bundle_signed = false;
	bool _bundle_group = false;
	bool _wait_seq_zero = false;
	ibrcommon::File _unixdomain;

	while((opt = getopt(argc, argv, "hg:Gd:t:s:c:l:ESU:w")) != -1)
	{
		switch (opt)
		{
		case 'h':
			print_help();
			return 0;

		case 'd':
			_destination = std::string(optarg);
			break;

		case 'g':
			_group = std::string(optarg);
			break;

		case 'G':
			_bundle_group = true;
			break;

		case 's':
			_source = optarg;
			break;

		case 'c':
			_chunk_size = atoi(optarg);
			break;

		case 't':
			__timeout_receive__ = atoi(optarg);
			break;

		case 'l':
			_lifetime = atoi(optarg);
			break;

		case 'E':
			_bundle_encryption = true;
			break;

		case 'S':
			_bundle_signed = true;
			break;

		case 'U':
			_unixdomain = ibrcommon::File(optarg);
			break;

		case 'w':
			_wait_seq_zero = true;
			break;

		default:
			std::cout << "unknown command" << std::endl;
			return -1;
		}
	}

	try {
		// Create a stream to the server using TCP.
		ibrcommon::tcpclient conn;

		// check if the unixdomain socket exists
		if (_unixdomain.exists())
		{
			// connect to the unix domain socket
			conn.open(_unixdomain);
		}
		else
		{
			// connect to the standard local api port
			conn.open("127.0.0.1", 4550);

			// enable nodelay option
			conn.enableNoDelay();
		}

		// Initiate a derivated client
		BundleStream bs(conn, _chunk_size, _source, _group, _wait_seq_zero);

		// Connect to the server. Actually, this function initiate the
		// stream protocol by starting the thread and sending the contact header.
		bs.connect();

		// transmitter mode
		if (_destination != dtn::data::EID())
		{
			bs.base().setDestination(_destination);
			bs.base().setLifetime(_lifetime);
			if (_bundle_encryption) bs.base().requestEncryption();
			if (_bundle_signed) bs.base().requestSigned();
			if (_bundle_group) bs.base().setSingleton(false);
			std::ostream stream(&bs.rdbuf());
			stream << std::cin.rdbuf() << std::flush;
		}
		// receiver mode
		else
		{
			std::istream stream(&bs.rdbuf());
			std::cout << stream.rdbuf() << std::flush;
		}

		// Shutdown the client connection.
		bs.close();
		conn.close();
	} catch (const ibrcommon::tcpclient::SocketException&) {
		std::cerr << "Can not connect to the daemon. Does it run?" << std::endl;
		return -1;
	} catch (const std::exception&) {

	}

}
