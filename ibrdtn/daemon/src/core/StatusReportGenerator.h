/*
 * StatusReportGenerator.h
 *
 *  Created on: 17.02.2010
 *      Author: morgenro
 */

#ifndef STATUSREPORTGENERATOR_H_
#define STATUSREPORTGENERATOR_H_

#include "core/EventReceiver.h"
#include "ibrdtn/data/StatusReportBlock.h"
#include "ibrdtn/data/MetaBundle.h"

namespace dtn
{
	namespace core
	{
		class StatusReportGenerator : public EventReceiver
		{
		public:
			StatusReportGenerator();
			virtual ~StatusReportGenerator();

			void raiseEvent(const Event *evt);

		private:
			/**
			 * This method generates a status report.
			 * @param b The attributes of the given bundle where used to generate the status report.
			 * @param type Defines the type of the report.
			 * @param reason Give a additional reason.
			 * @return A bundle with a status report block.
			 */
			void createStatusReport(const dtn::data::MetaBundle &b, dtn::data::StatusReportBlock::TYPE type, dtn::data::StatusReportBlock::REASON_CODE reason = dtn::data::StatusReportBlock::NO_ADDITIONAL_INFORMATION);
		};
	}
}


#endif /* STATUSREPORTGENERATOR_H_ */
