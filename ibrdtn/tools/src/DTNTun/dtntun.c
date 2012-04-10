

/*
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
*/

#include <inttypes.h>
#include <pthread.h>


#include "tun.h"
#include "receiver.h"

#include <ibrdtn/api/dtn_api.h>
//#include "/usr/local/include/ibrdtn/api/api/dtn_api.h"


static char devname[256];
static int tun_fd=-1;

void bundle_received(const void *data, uint32_t len) {
    printf("Bundle received, %i bytes\n",len);
    printf("Data: %s\n",(const char *)data);
}

int main()
{
    DTN_EP dtnfd=-1;

    pthread_t receiver;

    printf("Tunnel\n");
    dtn_hithere();
    dtnfd=dtn_register_endpoint("test",bundle_received, NULL);

    printf("Got dtnfd %i\n",dtnfd);

    //dtn_close_endpoint(dtnfd);

    strncpy(devname,"ibrdtn0",256);
    tun_fd=tun_create(devname);
    printf("Got %i\n",tun_fd);
    tun_conf_ipv4(devname, "172.16.0.1","172.16.0.2");

     pthread_create(&receiver,NULL, receiver_run, (void *) &tun_fd);

    while(1) {
        sleep(10);
    }
    return 0;
}
