#include "tun.h"


 int tun_create(char *dev)
  {
      struct ifreq ifr;
      int fd, err;

      if( (fd = open("/dev/net/tun", O_RDWR)) < 0 )
         return -1;

      memset(&ifr, 0, sizeof(ifr));

      /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
       *        IFF_TAP   - TAP device
       *
       *        IFF_NO_PI - Do not provide packet information
       */
      ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
      if( *dev )
         strncpy(ifr.ifr_name, dev, IFNAMSIZ);

      if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ){
          perror("Can't create TUN device: ");
         close(fd);
         return err;
      }
      strcpy(dev, ifr.ifr_name);
      return fd;
  }


/** Set tunnel addresses */
  int tun_conf_ipv4(char *ifname,  char *ip_from, char *ip_to) {
    char ifcfg_cmd[256];

    if (strlen(ip_from) > 15) {
        printf("tun_set_ipv4: Invalid from address %s\n",ip_from);
        return -1;
    }
    if (strlen(ip_to) > 15) {
        printf("tun_set_ipv4: Invalid to address %s\n",ip_to);
        return -1;
    }
    snprintf(ifcfg_cmd, 256, "ifconfig %s -pointopoint %s dstaddr %s",ifname, ip_from, ip_to);
    if ( system(ifcfg_cmd) != 0) {
        puts("Error in ifconfig");
        return -1;
    }


    return 0;

}
