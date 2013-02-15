/*
 * DeliveryPredictabilityMap.cpp
 *
 *  Created on: 08.01.2013
 *      Author: morgenro
 */

#include "routing/prophet/DeliveryPredictabilityMap.h"
#include "core/BundleCore.h"
#include <ibrdtn/utils/Clock.h>
#include <ibrcommon/Logger.h>

namespace dtn
{
	namespace routing
	{
		const size_t DeliveryPredictabilityMap::identifier = NodeHandshakeItem::DELIVERY_PREDICTABILITY_MAP;

		DeliveryPredictabilityMap::DeliveryPredictabilityMap()
		: NeighborDataset(DeliveryPredictabilityMap::identifier), _beta(0.0), _gamma(0.0), _lastAgingTime(0), _time_unit(0)
		{
		}

		DeliveryPredictabilityMap::DeliveryPredictabilityMap(const size_t &time_unit, const float &beta, const float &gamma)
		: NeighborDataset(DeliveryPredictabilityMap::identifier), _beta(beta), _gamma(gamma), _lastAgingTime(0), _time_unit(time_unit)
		{
		}

		DeliveryPredictabilityMap::~DeliveryPredictabilityMap() {
		}

		size_t DeliveryPredictabilityMap::getIdentifier() const
		{
			return identifier;
		}

		size_t DeliveryPredictabilityMap::getLength() const
		{
			size_t len = 0;
			for(predictmap::const_iterator it = _predictmap.begin(); it != _predictmap.end(); ++it)
			{
				/* calculate length of the EID */
				const std::string eid = it->first.getString();
				size_t eid_len = eid.length();
				len += data::SDNV(eid_len).getLength() + eid_len;

				/* calculate length of the float in fixed notation */
				const float& f = it->second;
				std::stringstream ss;
				ss << f << std::flush;

				size_t float_len = ss.str().length();
				len += data::SDNV(float_len).getLength() + float_len;
			}
			return data::SDNV(_predictmap.size()).getLength() + len;
		}

		std::ostream& DeliveryPredictabilityMap::serialize(std::ostream& stream) const
		{
			stream << data::SDNV(_predictmap.size());
			for(predictmap::const_iterator it = _predictmap.begin(); it != _predictmap.end(); ++it)
			{
				const std::string eid = it->first.getString();
				stream << data::SDNV(eid.length()) << eid;

				const float& f = it->second;
				/* write f into a stringstream to get final length */
				std::stringstream ss;
				ss << f << std::flush;

				stream << data::SDNV(ss.str().length());
				stream << ss.str();
			}
			IBRCOMMON_LOGGER_DEBUG_TAG("DeliveryPredictabilityMap", 20) << "Serialized with " << _predictmap.size() << " items." << IBRCOMMON_LOGGER_ENDL;
			IBRCOMMON_LOGGER_DEBUG_TAG("DeliveryPredictabilityMap", 60) << *this << IBRCOMMON_LOGGER_ENDL;
			return stream;
		}

		std::istream& DeliveryPredictabilityMap::deserialize(std::istream& stream)
		{
			data::SDNV elements_read(0);
			data::SDNV map_size;
			stream >> map_size;

			while(elements_read < map_size)
			{
				/* read the EID */
				data::SDNV eid_len;
				stream >> eid_len;
				char eid_cstr[eid_len.getValue()+1];
				stream.read(eid_cstr, sizeof(eid_cstr)-1);
				eid_cstr[sizeof(eid_cstr)-1] = 0;
				data::EID eid(eid_cstr);
				if(eid == data::EID())
					throw dtn::InvalidDataException("EID could not be casted, while parsing a dp_map.");

				/* read the probability (float) */
				float f;
				data::SDNV float_len;
				stream >> float_len;
				char f_cstr[float_len.getValue()+1];
				stream.read(f_cstr, sizeof(f_cstr)-1);
				f_cstr[sizeof(f_cstr)-1] = 0;

				std::stringstream ss(f_cstr);
				ss >> f;
				if(ss.fail())
					throw dtn::InvalidDataException("Float could not be casted, while parsing a dp_map.");

				/* check if f is in a proper range */
				if(f < 0 || f > 1)
					continue;

				/* insert the data into the map */
				_predictmap[eid] = f;

				elements_read += 1;
			}

			IBRCOMMON_LOGGER_DEBUG_TAG("DeliveryPredictabilityMap", 20) << "Deserialized with " << _predictmap.size() << " items." << IBRCOMMON_LOGGER_ENDL;
			IBRCOMMON_LOGGER_DEBUG_TAG("DeliveryPredictabilityMap", 60) << *this << IBRCOMMON_LOGGER_ENDL;
			return stream;
		}

		float DeliveryPredictabilityMap::get(const dtn::data::EID &neighbor) const throw (ValueNotFoundException)
		{
			predictmap::const_iterator it;
			if ((it = _predictmap.find(neighbor)) != _predictmap.end())
			{
				return it->second;
			}

			throw ValueNotFoundException();
		}

		void DeliveryPredictabilityMap::set(const dtn::data::EID &neighbor, float value)
		{
			_predictmap[neighbor] = value;
		}

		void DeliveryPredictabilityMap::clear()
		{
			_predictmap.clear();
		}

		void DeliveryPredictabilityMap::update(const dtn::data::EID &origin, const DeliveryPredictabilityMap &dpm, const float &p_encounter_first)
		{
			float neighbor_dp = p_encounter_first;

			try {
				neighbor_dp = this->get(origin);
			} catch (const DeliveryPredictabilityMap::ValueNotFoundException&) { }

			/**
			 * Calculate transitive values
			 */
			for (predictmap::const_iterator it = dpm._predictmap.begin(); it != dpm._predictmap.end(); ++it)
			{
				if ((it->first != origin) && (it->first != dtn::core::BundleCore::local))
				{
					float dp = 0;

					predictmap::iterator dp_it;
					if((dp_it = _predictmap.find(it->first)) != _predictmap.end())
						dp = dp_it->second;

					dp = max(dp, neighbor_dp * it->second * _beta);

					if(dp_it != _predictmap.end())
						dp_it->second = dp;
					else
						_predictmap[it->first] = dp;
				}
			}
		}

		void DeliveryPredictabilityMap::age(const float &p_first_threshold)
		{
			size_t current_time = dtn::utils::Clock::getUnixTimestamp();

			// prevent double aging
			if (current_time <= _lastAgingTime) return;

			unsigned int k = (current_time - _lastAgingTime) / _time_unit;

			predictmap::iterator it;
			for(it = _predictmap.begin(); it != _predictmap.end();)
			{
				if(it->first == dtn::core::BundleCore::local)
				{
					++it;
					continue;
				}

				it->second *= pow(_gamma, (int)k);

				if(it->second < p_first_threshold)
				{
					_predictmap.erase(it++);
				} else {
					++it;
				}
			}

			_lastAgingTime = current_time;
		}

		void DeliveryPredictabilityMap::toString(std::ostream &stream) const
		{
			predictmap::const_iterator it;
			for (it = _predictmap.begin(); it != _predictmap.end(); ++it)
			{
				stream << it->first.getString() << ": " << it->second << std::endl;
			}
		}

		std::ostream& operator<<(std::ostream& stream, const DeliveryPredictabilityMap& map)
		{
			map.toString(stream);
			return stream;
		}
	} /* namespace routing */
} /* namespace dtn */
