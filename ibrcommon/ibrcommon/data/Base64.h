/*
 * Base64.h
 *
 *  Created on: 08.11.2011
 *      Author: morgenro
 */

#ifndef BASE64_H_
#define BASE64_H_

#include <stdlib.h>
#include <stdint.h>

namespace ibrcommon
{
	class Base64
	{
	public:
		static const char encodeCharacterTable[];
		static const char decodeCharacterTable[];

		static const int EQUAL_CHAR = -2;
		static const int UNKOWN_CHAR = -1;

		static int getCharType(int val);

		/**
		 * returns the encoded length of given payload
		 * @param length
		 * @return
		 */
		static size_t getLength(size_t length);

		class Group
		{
		public:
			Group();
			virtual ~Group();

			void zero();
			uint8_t get_0() const;
			uint8_t get_1() const;
			uint8_t get_2() const;

			void set_0(uint8_t val);
			void set_1(uint8_t val);
			void set_2(uint8_t val);

			int b64_0() const;
			int b64_1() const;
			int b64_2() const;
			int b64_3() const;

			void b64_0(int val);
			void b64_1(int val);
			void b64_2(int val);
			void b64_3(int val);

		private:
			uint8_t _data[3];
		};
	};
} /* namespace dtn */
#endif /* BASE64_H_ */
