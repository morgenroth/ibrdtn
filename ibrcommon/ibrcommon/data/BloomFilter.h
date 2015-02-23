/*
 * BloomFilter.h
 *
 * This bloom filter implementation is based on
 * Open Bloom Filter (http://www.partow.net) written by Arash Partow.
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <cstddef>
#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <vector>
#include <list>

#ifndef BLOOMFILTER_H_
#define BLOOMFILTER_H_

namespace ibrcommon
{
	typedef unsigned int bloom_type;
	typedef unsigned char cell_type;

	class HashProvider
	{
	public:
		virtual ~HashProvider() = 0;

		/**
		 * Get the number of the available hash algorithms.
		 * @return
		 */
		virtual size_t count() const = 0;

		virtual void clear() = 0;
		virtual const std::list<bloom_type> hash(const unsigned char* begin, std::size_t remaining_length) const = 0;
	};

	class DefaultHashProvider : public HashProvider
	{
	public:
		DefaultHashProvider(size_t salt_count);
		virtual ~DefaultHashProvider();

		bool operator==(const DefaultHashProvider &provider) const;
		size_t count() const;
		void clear();

		const std::list<bloom_type> hash(const unsigned char* begin, std::size_t remaining_length) const;

	private:
		void add(bloom_type hash);
		void generate_salt();
		bloom_type hash_ap(const unsigned char* begin, std::size_t remaining_length, bloom_type hash) const;

		std::vector<bloom_type> _salt;
		std::size_t salt_count_;
	};

	class BloomFilter
	{
	protected:
		static const double table_allocation_max;
		static const std::size_t bits_per_char = 0x08;    		// 8 bits in 1 char(unsigned)
		static const unsigned char bit_mask[bits_per_char];

	public:
		BloomFilter(std::size_t table_size = 1024, std::size_t table_max = 524288, std::size_t salt_count = 2);
		BloomFilter(const BloomFilter& filter);
		virtual ~BloomFilter();

		void load(const cell_type* data, size_t len);

		BloomFilter& operator=(const BloomFilter& filter);

		bool operator!() const;
		void clear();

		void insert(const unsigned char* key_begin, const std::size_t length);

		template<typename T>
		void insert(const T& t)
		{
			insert(reinterpret_cast<const unsigned char*>(&t),sizeof(T));
		}

		void insert(const std::string& key);

		void insert(const char* data, const std::size_t& length);

		template<typename InputIterator>
		void insert(const InputIterator begin, const InputIterator end)
		{
			InputIterator it = begin;
			while(it != end)
			{
				insert(*(it++));
			}
		}

		virtual bool contains(const unsigned char* key_begin, const std::size_t length) const;

		template<typename T>
		bool contains(const T& t) const
		{
			return contains(reinterpret_cast<const unsigned char*>(&t),static_cast<std::size_t>(sizeof(T)));
		}

		bool contains(const std::string& key) const;

		bool contains(const char* data, const std::size_t& length) const;

		template<typename InputIterator>
		InputIterator contains_all(const InputIterator begin, const InputIterator end) const
		{
			InputIterator it = begin;
			while(it != end)
			{
				if (!contains(*it))
				{
					return it;
				}
				++it;
			}
			return end;
		}

		template<typename InputIterator>
		InputIterator contains_none(const InputIterator begin, const InputIterator end) const
		{
			InputIterator it = begin;
			while(it != end)
			{
				if (contains(*it))
				{
					return it;
				}
				++it;
			}
			return end;
		}

		virtual std::size_t size() const;

		BloomFilter& operator &= (const BloomFilter& filter);
		BloomFilter& operator |= (const BloomFilter& filter);
		BloomFilter& operator ^= (const BloomFilter& filter);

		const cell_type* table() const;

		/**
		 * Returns the allocation
		 */
		double getAllocation() const;

		/**
		 * Increase the Bloom-filter table by the initial size
		 * This operation also clears all elements
		 */
		bool grow(size_t num);

	protected:
		virtual void compute_indices(const bloom_type& hash, std::size_t& bit_index, std::size_t& bit) const;
		DefaultHashProvider _hashp;

		unsigned char*		bit_table_;
		std::size_t			table_size_;
		const std::size_t	table_size_init_;
		const std::size_t	table_size_max_;

		unsigned int _itemcount;
		std::size_t salt_count_;

	private:
		/**
		 * Returns the allocation under the assumption that the given number
		 * of items is stored within the filter
		 */
		double estimateAllocation(size_t items, size_t table_size) const;
	};
}


#endif /* BLOOMFILTER_H_ */
