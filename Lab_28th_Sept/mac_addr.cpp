#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <net/if.h>
#include <unistd.h>

using namespace std;


void getMacAddress(char * uc_Mac)
{
     int fd;
    
    struct ifreq ifr;
    
    //standard struct
    /*
struct ifreq {
    char ifr_name[IFNAMSIZ]; interface name
    union {
        struct sockaddr ifr_addr;
        struct sockaddr ifr_dstaddr;
        struct sockaddr ifr_broadaddr;
        struct sockaddr ifr_netmask;
        struct sockaddr ifr_hwaddr;
        short           ifr_flags;
        int             ifr_ifindex;
        int             ifr_metric;
        int             ifr_mtu;
        struct ifmap    ifr_map;
        char            ifr_slave[IFNAMSIZ];
        char            ifr_newname[IFNAMSIZ];
        char           *ifr_data;
    };
}
    */
    char *iface = (char*)("eth0");//standard interface eth0

    char *mac;
    
    fd = socket(AF_INET, SOCK_DGRAM, 0);//udp packet since SOCK_DGRAM

    ifr.ifr_addr.sa_family = AF_INET; //setting family name to ipv4

    strncpy((char *)ifr.ifr_name , (const char *)iface , IFNAMSIZ-1);//copied to ifr.ifr_name from iface

    //get hardware address in SIOCGIFHWADDR and encode in ifr
    ioctl(fd, SIOCGIFHWADDR, &ifr);//fd is an open file descriptor && returns 0 if success

    close(fd); //closing descriptor 
    
    mac = (char *)ifr.ifr_hwaddr.sa_data; //storing mac address(stored in hwaddr) in mac
    
    //display mac address and %.2x denotes print in hex form (atleast two hex digits in precision)
    sprintf((char *)uc_Mac,(const char *)"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n" , mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
}

int main()
{
    char mac[32]={0};

    getMacAddress (mac);

    cout<<endl<<"Mac Address : "<<mac;
    return 0;
}

//Output On My Machine :: "Mac Address : 69:6d:65:00:5f:5f"