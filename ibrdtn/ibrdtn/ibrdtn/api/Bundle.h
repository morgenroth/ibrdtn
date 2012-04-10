/*
 * Bundle.h
 *
 *  Created on: 24.07.2009
 *      Author: morgenro
 */

#ifndef API_BUNDLE_H_
#define API_BUNDLE_H_

#include "ibrdtn/data/EID.h"
#include "ibrdtn/data/Bundle.h"
#include <ibrcommon/data/BLOB.h>

#include <iostream>
#include <fstream>

namespace dtn
{
	namespace api
	{
		/**
		 * This is the basic bundle object as part of the API. An application should use this
		 * or derived objects to construct bundles for transmission to a bundle daemon.
		 */
		class Bundle
		{
			friend class Client;
			friend class APIClient;

		public:
			/**
			 * Define the Bundle Priorities
			 * PRIO_LOW low priority for this bundle
			 * PRIO_MEDIUM medium priority for this bundle
			 * PRIO_HIGH high priority for this bundle
			 */
			enum BUNDLE_PRIORITY
			{
				PRIO_LOW = 0,
				PRIO_MEDIUM = 1,
				PRIO_HIGH = 2
			};

			/**
			 * Constructor for a bundle object without a destination
			 */
			Bundle();

			/**
			 * Constructor for a bundle object
			 * @param destination defines the destination of this bundle.
			 */
			Bundle(const dtn::data::EID &destination);

			/**
			 * Destructor
			 */
			virtual ~Bundle();

			/**
			 * Set the destination address as singleton or not.
			 * @param val
			 */
			void setSingleton(bool val);

			/**
			 * Set the lifetime of a bundle.
			 * @param lifetime The new lifetime of this bundle in seconds.
			 */
			void setLifetime(unsigned int lifetime);

			/**
			 * Returns the lifetime of a bundle
			 * @return The lifetime of the bundle in seconds.
			 */
			unsigned int getLifetime() const;

			/**
			 * Returns the timestamp of the bundle.
			 */
			time_t getTimestamp() const;

			/**
			 * Set the request delivery report bit in the bundle processing flags.
			 * This triggers an delivered report, if the bundle is delivered to an application.
			 */
			void requestDeliveredReport();

			/**
			 * Set the request forwarded report bit in the bundle processing flags.
			 * This triggers an forwarded report on each sending bundle daemon.
			 */
			void requestForwardedReport();

			/**
			 * Set the request deleted report bit in the bundle processing flags.
			 * This triggers an deleted report, if the bundle is deleted on a bundle
			 * daemon.
			 */
			void requestDeletedReport();

			/**
			 * Set the request reception report bit in the bundle processing flags.
			 * This triggers an reception report on each receiving bundle daemon.
			 */
			void requestReceptionReport();

			/**
			 * Set the request custody transfer flag.
			 */
			void requestCustodyTransfer();

			/**
			 * Set the request encryption bit in the bundle processing flags.
			 * The right after the bundle is transferred to the daemon, it will
			 * be encrypted if keys are available.
			 */
			void requestEncryption();

			/**
			 * Set the request signed bit in the bundle processing flags.
			 * The right after the bundle is transferred to the daemon, it will
			 * be signed if keys are available.
			 */
			void requestSigned();

			/**
			 * Set the request compression flag.
			 */
			void requestCompression();

			/**
			 * Return the state of the verified bit. This is set, if the daemon
			 * has verified the signature of this bundle successfully.
			 */
			bool statusVerified();

			/**
			 * Set the priority for this bundle.
			 * @param p The priority of the bundle.
			 */
			void setPriority(BUNDLE_PRIORITY p);

			/**
			 * Returns the priority for this bundle.
			 * @return The priority of the bundle.
			 */
			BUNDLE_PRIORITY getPriority() const;

			/**
			 * Serialize a bundle object into a any standard output stream. This method is also used to
			 * transmit a bundle to a bundle daemon.
			 * @param stream A standard output stream to serialize the bundle into.
			 * @param b The bundle to serialize.
			 * @return The same output stream reference as the "stream" parameter.
			 */
			friend std::ostream &operator<<(std::ostream &stream, const dtn::api::Bundle &b)
			{
				// To send a bundle, we construct a default serializer. Such a serializer convert
				// the bundle data to the standardized form as byte stream.
				dtn::data::DefaultSerializer(stream) << b._b;

				// Since this method is used to serialize bundles into an StreamConnection, we need to call
				// a flush on the StreamConnection. This signals the stream to set the bundle end flag on
				// the last segment of streaming.
				stream.flush();

				// To support concatenation of streaming calls, we return the reference to the output stream.
				return stream;
			}

			/**
			 * Deserialize a bundle out of an input stream. This method is also used to received a bundle from
			 * a bundle daemon.
			 * @param stream A standard input stream to get the bundle from.
			 * @param b A bundle object as container for the deserialized data.
			 * @return The same output stream reference as the "stream" parameter.
			 */
			friend std::istream &operator>>(std::istream &stream, dtn::api::Bundle &b)
			{
				// To receive a bundle, we construct a default deserializer. Such a deserializer
				// convert a byte stream into a bundle object. If this deserialization fails
				// an exception will be thrown.
				dtn::data::DefaultDeserializer(stream) >> b._b;

				// To support concatenation of streaming calls, we return the reference to the input stream.
				return stream;
			}

			/**
			 * With this method the payload of the bundle can be accessed as a BLOB reference object.
			 * If no payload block is attached to this bundle an MissingObjectException is thrown.
			 * @return A reference to the payload block of this bundle.
			 */
			ibrcommon::BLOB::Reference getData() throw (dtn::MissingObjectException);

			/**
			 * This method set the destination of a bundle. By default a destination of a bundle is a singleton, but
			 * it is possible to send one bundle to a set of destinations by unset the destination-is-singleton bit.
			 * @param singelton Set this to true, if the destination is a singleton (default). Use false if you use it for multicast.
			 */
			void setDestination(const dtn::data::EID &eid, const bool singleton = true);

			/**
			 * Sets the report-to EID of this bundle. All requested reports are delivered to
			 * this EID.
			 * @param eid The EID to deliver reports to.
			 */
			void setReportTo(const dtn::data::EID &eid);

			/**
			 * This method returns the destination of this bundle.
			 * @return The destination EID of this bundle.
			 */
			dtn::data::EID getDestination() const;

			/**
			 * This method returns the source of this bundle.
			 * @return The source EID of this bundle.
			 */
			dtn::data::EID getSource() const;

			/**
			 * This method returns the report-to EID of this bundle.
			 * @return The report-to EID of this bundle.
			 */
			dtn::data::EID getReportTo() const;

			/**
			 * This method returns the custodian of this bundle.
			 * @return The custody EID of this bundle.
			 */
			dtn::data::EID getCustodian() const;

			/**
			 * Compares a bundle to another bundle to get an ordering of bundles.
			 * The ordering if completely based on the primary block of the bundle
			 * and compares source, timestamp, sequencenumber and fragment offset (if available).
			 * @param other A bundle to compare.
			 * @return True, if the given bundle is smaller (e.g. timestamp older) as ourself.
			 */
			bool operator<(const Bundle& other) const;

			/**
			 * Compares a bundle to another bundle to get an ordering of bundles.
			 * The ordering if completely based on the primary block of the bundle
			 * and compares source, timestamp, sequencenumber and fragment offset (if available).
			 * @param other A bundle to compare.
			 * @return True, if the given bundle is greater (e.g. timestamp newer) as ourself.
			 */
			bool operator>(const Bundle& other) const;

		protected:
			/**
			 * Constructor for a bundle object. This protected because this method should only
			 * used by derived classes.
			 * @param b A bundle to copy into this basic bundle.
			 */
			Bundle(const dtn::data::Bundle &b);

			// base bundle object
			dtn::data::Bundle _b;
		};
	}
}

#endif /* API_BUNDLE_H_ */
