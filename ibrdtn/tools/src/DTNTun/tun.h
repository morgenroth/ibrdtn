#ifndef __TUN_H__
#define __TUN_H__

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <string.h>


#include <net/if.h>
#include <linux/if_tun.h>

#include <sys/ioctl.h>

/* Forward decl */
int tun_create(char *dev);

int tun_conf_ipv4(char *ifname,  char *ip_from, char *ip_to);




#endif // __TUN_H__
