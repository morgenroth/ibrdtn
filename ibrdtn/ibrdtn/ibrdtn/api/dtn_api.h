/*
 * dtn_api.h
 *
 *   Copyright 2011 Sebastian Schildt
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef __DTN_API_H__
#define __DTN_API_H__


#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>




/** strucutre for notifications from daemon */
struct dtn_notification  {
    uint16_t status;
    uint32_t data;
};


/** Valid notifications */
#define DTN_NOTIFIY_SHUTDOWN     1    //DTN Endpoint will be unavbailable in the future
#define DTN_NOTIFIY_ACK          2
#define DTN_NOTIFY_NACK          3
#define DTN_NOTIFY_TOOBIG	 4   //bundle to big for callback mode

/** Allow to set the bundlesize when using "dtn_write" API. Every DTN_OPTION_CHUNKSIZE bytes a new bundle is created 
  * Resetting TX_CHUNKSIZE forces flushing of current tx_buffer */ 
#define DTN_OPTION_TXCHUNKSIZE 	1

/** you have to set a destination EID before dtn_write() can be used */
#define DTN_OPTION_DSTEID	2 

/** Forces flushing of the tx buffer */
#define DTN_OPTION_FLUSH	3

/** Maximum number of endpoints per process. DO NOT CHANGE ATM (see impl why) */
#define MAX_DTN_FDS	4







/** DTN endpoint descriptor (like filedescriptor) */
typedef int32_t DTN_EP;


/** Test stuff */
void dtn_hithere();


/** Register an endpoint on local device.Tries to grab dtn://<this_nodes_address>/<ep>
  * Returns DTN endpoint descriptor on success, or a vlaue <0 if shit happens 
  * If receive and notification callbacks are given, they'll be called in case of received data
  * (all data has to fit in RAM in this case) or notifications.  Access to process_bundle und 
  * dtn_notification is serialized, no need t make them reentrant
  */
DTN_EP dtn_register_endpoint(char *ep, void (*process_bundle)(const void * data, uint32_t size), void (*status_callback)(struct dtn_notification info));


/** stateless fire&forget send method. No acks, all data has to reside in RAM
 *  when intermixing send_bundle calls with calls to the dtn_write() API dtn_send_bundle will be 
 *  transmitted out-of-band, i.e. if there is still data in 
 */
void dtn_send_bundle(int32_t dtnd , char *dst_uri, char *data, uint32_t length);


/** Send data to DTN_OPTION_DSTEID, data is buffered and will  be transmitted if at least 
  * DTN_OPTION_TXCHUNKSIZE bytes of data have been written */
void dtn_write(DTN_EP ep, const void *data, uint32_t length);

/** Blocking read. This can only be used if the process_bundle callback was NULL
  * in register_endpoint. Returns number of bytes read, or <0 if shit happened
  */
int32_t dtn_read(DTN_EP ep, void *buf, uint32_t length);

/** Close dtn endpoint */
void dtn_close_endpoint(DTN_EP ep);

/** Set options for this dtn socket */
void dtn_endpoint_set_option(DTN_EP ep, uint16_t option, ...);

#define dtn_flush(ep)  dtn_endpoint_set_option(ep, DTN_OPTION_FLUSH)



#define KBYTE(x) x*1024
#define MBYTE(x) x*1024*1024



#ifdef __cplusplus
}
#endif


#endif
