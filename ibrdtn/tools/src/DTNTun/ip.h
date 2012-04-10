#ifndef __IP_H__
#define __IP_H__

#include <inttypes.h>
#include <arpa/inet.h>

#define _noalign

#define IP_VERSION      0

#define IP_VERSION_AND_HEADERLENGTH  0
#define IPV4_TYPE_OF_SERVICE           1
#define IPV4_LENGTH                    2
#define IPV4_IDENTFICATION             4
#define IPV4_FLAGS_AND_FRAGMENT_OFSET  6
#define IPV4_TTL                       8
#define IPV4_PROTOCOL                  9
#define IPV4_CHECKSUM                  10
#define IPV4_SOURCE                    12
#define IPV4_DESTINATION               16



#define IP_PROT_UDP     17
#define IP_PROT_TCP     11
#define IP_PROT_ICMP    1



#define ICMP_TYPE               0
#define ICMP_CODE               1
#define ICMP_CRC                2
#define ICMP_ECHO_IDENTIFIER    4
#define ICMP_ECHO_SEQNUMBER     6
#define ICMP_DATA               8

#define ICMP_ECHO_REQUEST  8
#define ICMP_ECHO_REPLY  0





#define GET_UINT16(buffer,position)  ( ntohs( *(_noalign uint16_t *)(buffer+position) )   )
#define GET_UINT32(buffer,position)  ( ntohl( *(_noalign uint32_t *)(buffer+position) )   )

#define SET_UINT16(buffer,position,data)  ( *( _noalign uint16_t *)(buffer+position)=htons(data) )
#define SET_UINT32(buffer,position,data)  ( *( _noalign uint32_t *)(buffer+position)=htonl(data) )



#endif // __IP_H__
