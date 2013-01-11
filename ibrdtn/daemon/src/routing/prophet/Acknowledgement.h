/*
 * Acknowledgement.h
 *
 *  Created on: 11.01.2013
 *      Author: morgenro
 */

#ifndef ACKNOWLEDGEMENT_H_
#define ACKNOWLEDGEMENT_H_

#include <ibrdtn/data/BundleID.h>
#include <iostream>

namespace dtn
{
	namespace routing
	{
		/*!
		 * \brief Represents an Acknowledgement, i.e. the bundleID that is acknowledged
		 * and the lifetime of the acknowledgement
		 */
		class Acknowledgement
		{
		public:
			Acknowledgement();
			Acknowledgement(const dtn::data::BundleID&, size_t expire_time);
			virtual ~Acknowledgement();
			friend std::ostream& operator<<(std::ostream&, const Acknowledgement&);
			friend std::istream& operator>>(std::istream&, Acknowledgement&);

			bool operator<(const Acknowledgement &other) const;

			dtn::data::BundleID bundleID; ///< The BundleID that is acknowledged
			size_t expire_time; ///< The expire time of the acknowledgement, given in the same format as the bundle timestamps
		};
	} /* namespace routing */
} /* namespace dtn */
#endif /* ACKNOWLEDGEMENT_H_ */
