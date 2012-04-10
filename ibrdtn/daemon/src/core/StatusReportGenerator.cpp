/*
 * StatusReportGenerator.cpp
 *
 *  Created on: 17.02.2010
 *      Author: morgenro
 */

#include "core/StatusReportGenerator.h"
#include "core/BundleEvent.h"
#include "core/BundleGeneratedEvent.h"
#include "core/BundleCore.h"
#include <ibrdtn/data/MetaBundle.h>

namespace dtn
{
	namespace core
	{
		using namespace dtn::data;

		StatusReportGenerator::StatusReportGenerator()
		{
			bindEvent(BundleEvent::className);
		}

		StatusReportGenerator::~StatusReportGenerator()
		{
			unbindEvent(BundleEvent::className);
		}

		void StatusReportGenerator::createStatusReport(const dtn::data::MetaBundle &b, StatusReportBlock::TYPE type, StatusReportBlock::REASON_CODE reason)
		{
			// create a new bundle
			Bundle bundle;

			// create a new statusreport block
			StatusReportBlock &report = bundle.push_back<StatusReportBlock>();

			bundle.set(dtn::data::PrimaryBlock::APPDATA_IS_ADMRECORD, true);

			// get the flags and set the status flag
			report._status |= type;

			// set the reason code
			report._reasoncode |= reason;

			switch (type)
			{
				case StatusReportBlock::RECEIPT_OF_BUNDLE:
					report._timeof_receipt.set();
				break;

				case StatusReportBlock::CUSTODY_ACCEPTANCE_OF_BUNDLE:
					report._timeof_custodyaccept.set();
				break;

				case StatusReportBlock::FORWARDING_OF_BUNDLE:
					report._timeof_forwarding.set();
				break;

				case StatusReportBlock::DELIVERY_OF_BUNDLE:
					report._timeof_delivery.set();
				break;

				case StatusReportBlock::DELETION_OF_BUNDLE:
					report._timeof_deletion.set();
				break;

				default:

				break;
			}

			// set source and destination
			bundle._source = dtn::core::BundleCore::local;
			bundle.set(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON, true);
			bundle._destination = b.reportto;

			// set bundle parameter
			if (b.get(Bundle::FRAGMENT))
			{
				report._fragment_offset = b.offset;
				report._fragment_length = b.appdatalength;
				report._admfield |= 1;
			}

			report._bundle_timestamp = b.timestamp;
			report._bundle_sequence = b.sequencenumber;
			report._source = b.source;

			dtn::core::BundleGeneratedEvent::raise(bundle);
		}

		void StatusReportGenerator::raiseEvent(const Event *evt)
		{
			try {
				const BundleEvent &bundleevent = dynamic_cast<const BundleEvent&>(*evt);
				const dtn::data::MetaBundle &b = bundleevent.getBundle();

				// do not generate status reports for other status reports or custody signals
				if (b.get(dtn::data::PrimaryBlock::APPDATA_IS_ADMRECORD)) return;

				switch (bundleevent.getAction())
				{
				case BUNDLE_RECEIVED:
					if ( b.get(Bundle::REQUEST_REPORT_OF_BUNDLE_RECEPTION))
					{
						createStatusReport(b, StatusReportBlock::RECEIPT_OF_BUNDLE, bundleevent.getReason());
					}
					break;
				case BUNDLE_DELETED:
					if ( b.get(Bundle::REQUEST_REPORT_OF_BUNDLE_DELETION))
					{
						createStatusReport(b, StatusReportBlock::DELETION_OF_BUNDLE, bundleevent.getReason());
					}
					break;

				case BUNDLE_FORWARDED:
					if ( b.get(Bundle::REQUEST_REPORT_OF_BUNDLE_FORWARDING))
					{
						createStatusReport(b, StatusReportBlock::FORWARDING_OF_BUNDLE, bundleevent.getReason());
					}
					break;

				case BUNDLE_DELIVERED:
					if ( b.get(Bundle::REQUEST_REPORT_OF_BUNDLE_DELIVERY))
					{
						createStatusReport(b, StatusReportBlock::DELIVERY_OF_BUNDLE, bundleevent.getReason());
					}
					break;

				case BUNDLE_CUSTODY_ACCEPTED:
					if ( b.get(Bundle::REQUEST_REPORT_OF_CUSTODY_ACCEPTANCE))
					{
						createStatusReport(b, StatusReportBlock::CUSTODY_ACCEPTANCE_OF_BUNDLE, bundleevent.getReason());
					}
					break;

				default:
					break;
				}
			} catch (const std::bad_cast&) { };
		}
	}
}
