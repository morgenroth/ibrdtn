/*
 * ManagementConnection.h
 *
 *  Created on: 13.10.2011
 *      Author: morgenro
 */

#ifndef MANAGEMENTCONNECTION_H_
#define MANAGEMENTCONNECTION_H_

#include "api/ClientHandler.h"

namespace dtn
{
	namespace api
	{
		class ManagementConnection : public ProtocolHandler
		{
		public:
			ManagementConnection(ClientHandler &client, ibrcommon::tcpstream &stream);
			virtual ~ManagementConnection();

			void run();
			void finally();
			void setup();
			void __cancellation();

		private:
			void processCommand(const std::vector<std::string> &cmd);
		};
	} /* namespace api */
} /* namespace dtn */
#endif /* MANAGEMENTCONNECTION_H_ */
