/* DTNPing C API asynchronous ersion */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ibrdtn/api/dtn_api.h>


/** DTN endpoint descriptor */
DTN_EP dep=-1;

/** Still wait for some byts */
int16_t waitfor=0;


/** Commandline usage */
void print_usage(char *me) {
    printf("Usage: %s dst-eid message\n",me);
    exit(-1);
}


void bundle_received(const void *data, uint32_t len) {
    int i;
    printf("Bundle received, %i bytes\n",len);
    printf("Data: \"");
    for (i=0; i< len; i++)
        putchar( *((char *)data+i) );
    puts("\"");
    waitfor-=len;
    if (waitfor < 0) {
        dtn_close_endpoint(dep);
        exit(0);
    }
}


/** Main entry */
int main(int argc, char *argv[])
{


    if (argc < 3)
        print_usage(argv[0]);

    //Register endpoint and receive callback
    dep=dtn_register_endpoint("test",bundle_received, NULL);

    if (dep < 0) {
        puts("Problems registering endpoint. Terminating");
        return -2;
    }

    printf("Sending %s\n", argv[2]);
    waitfor=strlen(argv[2]+1);

    dtn_send_bundle(dep,argv[1],argv[2],strlen(argv[2])+1);




    //Wait for answer
    while(1)
        sleep(5);

    return 0;
}
