#ifndef UTILS_H_
#define UTILS_H_


#include "ibrdtn/data/Bundle.h"
#include "ibrdtn/data/CustodySignalBlock.h"
#include "ibrdtn/data/StatusReportBlock.h"
#include "ibrdtn/data/PayloadBlock.h"

namespace dtn
{
	namespace utils
	{
		class Utils
		{
		public:
			static void rtrim(std::string &str);
			static void ltrim(std::string &str);
			static void trim(std::string &str);

			static vector<string> tokenize(std::string token, std::string data, size_t max = std::string::npos);
			static double distance(double lat1, double lon1, double lat2, double lon2);

			static void encapsule(dtn::data::Bundle &capsule, const std::list<dtn::data::Bundle> &bundles);
			static void decapsule(const dtn::data::Bundle &capsule, std::list<dtn::data::Bundle> &bundles);

		private:
			static void encapsule(ibrcommon::BLOB::Reference &ref, const std::list<dtn::data::Bundle> &bundles);
			static double toRad(double value);
			static const double pi;
		};
	}
}

#endif /*UTILS_H_*/
