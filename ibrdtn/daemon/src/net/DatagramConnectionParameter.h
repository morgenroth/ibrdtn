/*
 * DatagramConnectionParameter.h
 *
 *  Created on: 23.11.2011
 *      Author: morgenro
 */

#ifndef DATAGRAMCONNECTIONPARAMETER_H_
#define DATAGRAMCONNECTIONPARAMETER_H_

namespace dtn
{
	namespace net
	{
		class DatagramConnectionParameter
		{
		public:
			enum FLOWCONTROL
			{
				FLOW_NONE = 0,
				FLOW_STOPNWAIT = 1
			};

			FLOWCONTROL flowcontrol;
			unsigned int max_seq_numbers;
			size_t max_msg_length;
		};
	}
}

#endif /* DATAGRAMCONNECTIONPARAMETER_H_ */
