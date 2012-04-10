/* DTNPing C API synchronous ersion */


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



/** Main entry */
int main(int argc, char *argv[])
{

    char buf[128];
    int count;
    int i;

    if (argc < 3)
        print_usage(argv[0]);

    //No receive callback=synchronous mode
    dep=dtn_register_endpoint("test",NULL, NULL); //sync mode

    if (dep < 0) {
        puts("Problems registering endpoint. Terminating");
        return -2;
    }

    printf("Sending %s\n", argv[2]);
    waitfor=strlen(argv[2]+1);

    dtn_endpoint_set_option(dep,DTN_OPTION_DSTEID,argv[1]);
    //Uncomment to test fragmentation: force fragment using small buffer
    //dtn_endpoint_set_option(dep,DTN_OPTION_TXCHUNKSIZE,5);

    dtn_write(dep, argv[2],strlen(argv[2])+1);
    dtn_flush(dep);

    while (waitfor > 0) {
            count=dtn_read(dep,buf,128);
            if (count < 0) {
                puts("Error reading from endpoint");
                waitfor=0;
                continue;
            }
            printf("Read %i bytes: ", count);
            for (i=0; i< count; i++)
                putchar( buf[i] );
            puts("");
            waitfor-=count;
    }

    return 0;
}
