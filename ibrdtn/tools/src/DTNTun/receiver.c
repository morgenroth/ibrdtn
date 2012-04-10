
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "receiver.h"
#include "ip.h"


void process_ipv4( uint16_t size, char *packet);
void process_icmp( uint16_t size, char *packet, uint16_t icmp_offset);
uint16_t ip_checksum(void *start, unsigned int count, uint32_t initial_value);


static int tun_fd = -1;

void process_packet(uint16_t size, char *packet) {
        if (size < 20) //minimal IPv4 Header
            return;

        if ( (packet[IP_VERSION] & 0xF0) == 0x40 ) {
            process_ipv4(size, packet);
        }
        else {
            printf("IP version %x not implemented\n",(packet[IP_VERSION] % 0xF0 ));
        }
}


void print_packet(uint16_t size, char *packet ) {
    int i;
    for (i=0; i<size; i++)
        printf("%02X ",*(uint8_t *)(packet+i));
    puts("");
}

void process_ipv4(uint16_t size, char *packet) {
    uint8_t protocol=packet[IPV4_PROTOCOL];
    switch(protocol) {
        case IP_PROT_ICMP:  printf("ICMP package, size %i\n",size-20);
                            process_icmp(size,packet,20);
                            break;
        case IP_PROT_UDP:   printf("UDP package, size %i\n",size-20);
                            break;
        case IP_PROT_TCP:   printf("TCP package, size %i\n",size-20);
                            break;
        default: printf("Unknown IP payload %x\n",protocol);
    }
}



/*        icmp_checksum=SAP_ip_request_Checksum(toSend->data,buffer->length-20,0); //Same length
as request, subtract IP headers

               //SET_UINT16(toSend->data,ICMP_CRC,icmp_checksum);
               buffer->data-=20; //Roll back for IP
               toSend->length=buffer->length-20; //as long as Echo Request, don't count I Header here
               SAP_ip_request_transmit(toSend->id,src_ip,PROTO_ICMP);

               */


#define PING_BOOSTER
void process_icmp( uint16_t size, char *packet, uint16_t icmp_offset) {
#ifdef PING_BOOSTER
    uint8_t *icmp_frame= (uint8_t *) packet+icmp_offset;
    uint16_t checksum;


    if ( icmp_frame[ICMP_TYPE] != ICMP_ECHO_REQUEST )
        return;

    puts("PingBoost!");
    uint8_t *ping_reply=malloc(size);
    uint32_t src,dst;

    if (ping_reply==NULL) {
        puts("Malloc failed");
        return;
    }

    memcpy(ping_reply, packet, size);
    icmp_frame= (uint8_t *) ping_reply+icmp_offset;
    dst=GET_UINT32(packet,IPV4_DESTINATION);
    src=GET_UINT32(packet,IPV4_SOURCE);

    icmp_frame[ICMP_TYPE]=ICMP_ECHO_REPLY;
    icmp_frame[ICMP_CODE]=ICMP_ECHO_REPLY;

    //Reset checksum
    icmp_frame[ICMP_CRC]  =0x00;
    icmp_frame[ICMP_CRC+1]=0x00;

    checksum=ip_checksum(icmp_frame,size-20,0);
    printf("ICMP Check over size %i: %x",size-20,checksum);
    SET_UINT16(icmp_frame,ICMP_CRC, checksum);

    //ICMP is done, change adressing in IP:
    SET_UINT32(ping_reply,IPV4_SOURCE,dst);
    SET_UINT32(ping_reply,IPV4_DESTINATION,src);

    SET_UINT16(ping_reply,IPV4_CHECKSUM,0);
    checksum=ip_checksum(ping_reply,20,0);
    SET_UINT16(ping_reply,IPV4_CHECKSUM,checksum);

    print_packet(size, (char *) ping_reply);

    //ugly, won't be threadsafe later
    if ( write(tun_fd,ping_reply,size) == -1) {
        perror("PingBooster write error: ");
    }


    free(ping_reply);




#endif
}




/** Calculates a IP checksum (as needed by IP, TCP and UDP). Parameters are
  *  start     points to the beginning of the data to be checksummed
  *  count     Indicates how many bytes should be checksummed
  * Since protcols like IP and UDP work with so called Pseudo Headers (see RFC for details) you can
  * precalculate that part and give it as initial_value to this function
  */
uint16_t ip_checksum(void *start, unsigned int count, uint32_t initial_value)
{
  uint32_t sum = initial_value; //might be not 0 if protcol includes pseudo header (udp, tcp)

  uint16_t * piStart;

  piStart = start;
  while (count > 1) {
    sum += *piStart++;
    count -= 2;
  }

  if (count)                                     // add left-over byte, if any
    sum += *(unsigned char *)piStart;

  while (sum >> 16)                              // fold 32-bit sum to 16 bits
    sum = (sum & 0xFFFF) + (sum >> 16);

  sum=htons(sum);
  return ~sum;
}




void * receiver_run(void *arg) {
    char buf[BUFSIZE];
    ssize_t bytes_read;
    tun_fd = *((int *)arg);
    int running=1;

    while(running) {
        bytes_read=read(tun_fd, buf, BUFSIZE);
        if (bytes_read == 0) { //EOF
            running=0;
            continue;
        }
        else if (bytes_read == -1) {
            perror("Error reading from TUN: ");
            running=1;
            continue;
        }
        printf("Read %i bytes\n", bytes_read);
        print_packet(bytes_read,buf);
        process_packet(bytes_read, buf);
    }
    puts("Receiver thread down.");
    return 0;
}
