#include <stdio.h>

#include <signal.h>

#include <arpa/inet.h>

#include <sys/types.h>

#include <sys/socket.h>

#include <unistd.h>

#include <netinet/in.h>

#include <netinet/ip.h>

#include <netinet/ip_icmp.h>

#include <netdb.h>

#include <setjmp.h>

#include <errno.h>

#include <stdlib.h>

#include <stdarg.h>

#include <strings.h>

#include <sys/time.h>

#include <bits/stdc++.h>

using namespace std;

int	datalen = 56; /* data that goes with ICMP echo request */

int len,sockfd;		

struct sockaddr_in sasend;

pid_t pid; //pid of the process that is sending the packet in this case

int nsent = 0;//sequence number of the sent packet

//subtracting received time and sent time of the packets
//subtracting two values 1.seconds and 2.micro-seconds
void tv_sub(struct timeval *out, const struct timeval *in)

{

    if ((out->tv_usec -= in->tv_usec) < 0)

    {

        --out->tv_sec;

        out->tv_usec += 1000000;

    } 

    out->tv_sec -= in->tv_sec;

}

//calculate check sum to ensure data integrity
unsigned short in_cksum(unsigned short *addr, int len)
{
    register int sum = 0;

    u_short answer = 0;
    
    register u_short *w = addr;
    
    register int nleft = len;
    /*
     * Our algorithm is simple, using a 32 bit accumulator (sum), we add
     * sequential 16 bit words to it, and at the end, fold back all the
     * carry bits from the top 16 bits into the lower 16 bits.
     */
    
    while (nleft > 1)
    
    {
      sum += *w++;
      nleft -= 2;
    }
    /* mop up an odd byte, if necessary */
    
    if (nleft == 1)
    {
      *(u_char *) (&answer) = *(u_char *) w;
      sum += answer;
    }
    /* add back carry outs from top 16 bits to low 16 bits */
    
    sum = (sum >> 16) + (sum & 0xffff);        /* add hi 16 to low 16 */
    sum += (sum >> 16);                /* add carry */
    answer = ~sum;                /* truncate to 16 bits */
    return (answer);
}



//process the received packet

void proc_v4(char *ptr, ssize_t len, struct timeval *tvrecv, char *host)
{

struct ip *ip = (struct ip *) ptr; /* start of IP header */

int hlen1 = ip->ip_hl << 2; /* length of IP header */

struct icmp *icmp = (struct icmp *) (ptr + hlen1); /* start of ICMP header */

	int icmplen;

	if ( (icmplen = len - hlen1) < 8)

	printf("icmplen (%d) < 8", icmplen);

		if (icmp->icmp_type == ICMP_ECHOREPLY) // if received packet is of icmp echo reply  

		{

		if (icmp->icmp_id != pid)

		return; /* not a response to our ECHO_REQUEST */

		if (icmplen < 16)

		printf("icmplen (%d) < 16", icmplen);

		struct timeval* tvsend = (struct timeval *) icmp->icmp_data; //storing sent time of the sent packet(icmp echo request)

		tv_sub(tvrecv, tvsend); // subtracting recv time and sent time

		double rtt = tvrecv->tv_sec * 1000.0 + tvrecv->tv_usec / 1000.0;//calculating rtt in millisec

		char * message = (char *)(icmp->icmp_data) + sizeof(struct timeval); //storing the message in the received packet
		
		printf("%d bytes from %s: seq=%u, ttl=%d, rtt=%.3f ms\n", icmplen, host,icmp->icmp_seq, ip->ip_ttl, rtt);
		
		printf("Message Returned : %s\n", message);//print the message along the information regarding the received packet as above
		
		}
}


//send function to send the packet

void send_v4(void)
{

char sendbuf[4096]; //send buffer of size 4096

struct icmp *icmp;

icmp = (struct icmp *) sendbuf;

icmp->icmp_type = ICMP_ECHO;//type of the packet icmp echo request

icmp->icmp_code = 0;//code for icmp echo request and reply is zero

icmp->icmp_id = pid;//process id of process sending the packet

icmp->icmp_seq = nsent++;//increasing sequence number by one since sendbuff is occupied by the sent packet

//getting time and storing in the icmp packet that is being sent 
//so as to process the received time in the received packet and calculated RTT 
gettimeofday((struct timeval *) icmp->icmp_data, NULL);

char * message = (char* )("Hello\0");

strcpy(((char *)icmp->icmp_data) + sizeof(struct timeval), message);

int len = 8 + datalen; /* checksum ICMP header and data */

icmp->icmp_cksum = 0;//initialising checksum to zero

icmp->icmp_cksum = in_cksum((u_short *) icmp, len);//calculate check sum

//if less than zero our socket is of non_socket type which means invalid socket
if (sendto(sockfd, sendbuf, len, 0, (struct sockaddr *) &sasend, sizeof(sasend)) < 0)
    {
        perror("sendto error");

    }

sleep(1);

sendto(sockfd, sendbuf, len, 0, (struct sockaddr *) &sasend,sizeof(sasend));//sending the packet

}

//setting an alarm

void sig_alrm(int signo)

{

	send_v4();//send the icmp packet

	alarm(1);//set the alarm for every one sec (i.e sending the packet for every one sec )

	return;

}


void readloop(char *host)

{

char recvbuf[4096];//receive buffer of 4096 size

sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);//created raw socket on root (as it can only be created on root)

//if u get this error use root privileges i.e sudo while running the program
if(sockfd<0)
	
	{

        perror("socket error");

        exit(1);

    }

setuid(getuid()); /* don't need special permissions any more */

sig_alrm(SIGALRM); /* send first packet */

struct sockaddr_in sarecv;//recv structure

int n;socklen_t size;// size of socklen_t type since argument in recvfrom should be of socklen_t type instead of int in cpp

struct timeval tval;
		
		for ( ; ; ) 
			{
				size = sizeof(sarecv);

				n =recvfrom(sockfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*) &sarecv, &size); //receive the packet

				if (n < 0) 
					{
						if (errno == EINTR) continue;   // interrupt due to alarm is possible (EINTR)
						
						else printf("recvfrom error");
					}

				sleep(1);

				gettimeofday(&tval, NULL);//getting time when packet is received and storing in tval (used further to evaluate RTT)

				proc_v4(recvbuf, n, &tval, host);//process the icmp received packet
			}
}



int main(int argc, char **argv)

{

//if u did not pass an ip address as 2nd argument 
if(argc<2)
	
	{
		printf("Please enter host address as arg \n");
		
		exit(0);
	}

char* host = argv[1];//ex: google.com (pinging google.com)

pid = getpid();//pid of process executing the program

signal(SIGALRM, sig_alrm);//registering signal handler function

printf("PING %s : %d data bytes\n", argv[1], datalen);

bzero(&sasend, sizeof(sasend));//setting zero as default 

sasend.sin_family = AF_INET; // family ipv4

sasend.sin_addr.s_addr = inet_addr(argv[1]); // storing network byte order addr of say google.com (or anything else)

readloop(argv[1]);//calling readloop by passing some host (ex:google.com) as the argument

exit(0);

}


/*
Compilation and Execution instructions :
1.g++ ping.cpp -o ping
2.sudo ./ping 8.8.8.8 (in case of google.com)

if u pass host name instead of ip you will get the following error : "sendto error: Permission denied" since DNS is not used

*/