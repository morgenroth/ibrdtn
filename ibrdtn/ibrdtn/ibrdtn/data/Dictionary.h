/*
 * Dictionary.h
 *
 *  Created on: 29.05.2009
 *      Author: morgenro
 */



#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include "ibrdtn/data/EID.h"
#include <list>
#include <sstream>
#include <list>

using namespace std;

namespace dtn
{
	namespace data
	{
		class Bundle;

		class Dictionary
		{
		public:
			/**
			 * create a empty dictionary
			 */
			Dictionary();

			/**
			 * create a dictionary with all EID of the given bundle
			 */
			Dictionary(const dtn::data::Bundle &bundle);

			/**
			 * copy constructor
			 */
			Dictionary(const Dictionary &d);

			/**
			 * assign operator
			 */
			Dictionary& operator=(const Dictionary &d);

			/**
			 * destructor
			 */
			virtual ~Dictionary();

			/**
			 * add a eid to the dictionary
			 */
			void add(const EID &eid);

			/**
			 * add a list of eids to the dictionary
			 */
			void add(const list<EID> &eids);

			/**
			 * return the eid for the reference [scheme,ssp]
			 */
			EID get(size_t scheme, size_t ssp);

			/**
			 * clear the dictionary
			 */
			void clear();

			/**
			 * returns the size of the bytearray
			 */
			size_t getSize() const;

			/**
			 * returns the references of the given eid
			 */
			pair<size_t, size_t> getRef(const EID &eid) const;

			friend std::ostream &operator<<(std::ostream &stream, const dtn::data::Dictionary &obj);
			friend std::istream &operator>>(std::istream &stream, dtn::data::Dictionary &obj);

		private:
			bool exists(const std::string) const;
			void add(const std::string);
			size_t get(const std::string) const;

			stringstream _bytestream;
		};
	}
}

#endif /* DICTIONARY_H_ */
