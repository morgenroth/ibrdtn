/*
 * EID.h
 *
 *  Created on: 09.03.2009
 *      Author: morgenro
 */

#ifndef EID_H_
#define EID_H_

#include <string>
#include "ibrcommon/Exceptions.h"

using namespace std;

namespace dtn
{
	namespace data
	{
		class EID
		{
		public:
			static const std::string DEFAULT_SCHEME;
			static const std::string CBHE_SCHEME;

			EID();
			EID(std::string scheme, std::string ssp);
			EID(std::string value);

			/**
			 * Constructor for CBHE EIDs.
			 * @param node Node number.
			 * @param application Application number.
			 */
			EID(size_t node, size_t application);

			virtual ~EID();

			EID& operator=(const EID &other);

			bool operator==(EID const& other) const;

			bool operator==(string const& other) const;

			bool operator!=(EID const& other) const;

			EID operator+(string suffix) const;

			bool sameHost(string const& other) const;
			bool sameHost(EID const& other) const;

			bool operator<(EID const& other) const;
			bool operator>(const EID& other) const;

			std::string getString() const;
			std::string getApplication() const throw (ibrcommon::Exception);
			std::string getHost() const throw (ibrcommon::Exception);
			std::string getScheme() const;
			std::string getSSP() const;

			std::string getDelimiter() const;

			EID getNode() const throw (ibrcommon::Exception);

			bool hasApplication() const;

			/**
			 * check if a EID is compressable.
			 * @return True, if the EID is compressable.
			 */
			bool isCompressable() const;

			/**
			 * Determine if this EID is null
			 * @return True, if the EID is null
			 */
			bool isNone() const;

			/**
			 * Get the compressed EID as two numeric values. Both values
			 * are set to zero if the EID is not compressable.
			 * @return A pair of two numeric values.
			 */
			std::pair<size_t, size_t> getCompressed() const;

		private:
			std::string _scheme;
			std::string _ssp;
		};
	}
}

#endif /* EID_H_ */
