#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <sys/ioctl.h> 
#include <net/if.h> 
#include <arpa/inet.h> 
#include <linux/netlink.h> 
#include <linux/rtnetlink.h> 


int get_local_net_info(  const char* lpszEth,  char* szIpAddr,  char* szNetmask,  char* szMacAddr  ) 
{ 
    int ret = 0; 
    struct ifreq req; 
    struct sockaddr_in* host = NULL; 
 
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
    if ( -1 == sockfd ) 
        return -1; 
    bzero(&req, sizeof(struct ifreq)); 
    strcpy(req.ifr_name, lpszEth); 
    if ( ioctl(sockfd, SIOCGIFADDR, &req) >= 0 ) 
    { 
        host = (struct sockaddr_in*)&req.ifr_addr; 
        strcpy(szIpAddr, inet_ntoa(host->sin_addr)); 
    }  else {  
        ret = -1; 
    } 
 
    bzero(&req, sizeof(struct ifreq)); 
    strcpy(req.ifr_name, lpszEth); 
    if ( ioctl(sockfd, SIOCGIFNETMASK, &req) >= 0 ) 
    { 
        host = (struct sockaddr_in*)&req.ifr_addr; 
        strcpy(szNetmask, inet_ntoa(host->sin_addr)); 
    }  else  { 
        ret = -1; 
    } 
 
    bzero(&req, sizeof(struct ifreq)); 
    strcpy(req.ifr_name, lpszEth); 
    if ( ioctl(sockfd, SIOCGIFHWADDR, &req) >= 0 ) 
    { 
        sprintf( 
            szMacAddr, "%02x:%02x:%02x:%02x:%02x:%02x", 
            (unsigned char)req.ifr_hwaddr.sa_data[0], 
            (unsigned char)req.ifr_hwaddr.sa_data[1], 
            (unsigned char)req.ifr_hwaddr.sa_data[2], 
            (unsigned char)req.ifr_hwaddr.sa_data[3], 
            (unsigned char)req.ifr_hwaddr.sa_data[4], 
            (unsigned char)req.ifr_hwaddr.sa_data[5] 
        ); 
    }  else  { 
        ret = -1; 
    } 
 
    if ( sockfd != -1 ) 
    { 
        close(sockfd); 
        sockfd = -1; 
    } 
    return ret; 
} 
 
// ------------------------------------------------------ 
 
#define GATEWAY_BUFSIZE 1024
 
struct route_info  
{ 
    char if_name[16]; 
    uint32_t gateway; 
    uint32_t src_addr; 
    uint32_t dst_addr; 
}; 
 
static int read_routes(int fd,  char* buf,  int seq,  int pid)
{ 
    struct nlmsghdr* hdr = NULL; 
    int read_len = 0, msg_len = 0; 
 
    while (1) 
    { 
        if((read_len = recv(fd, buf, GATEWAY_BUFSIZE - msg_len, 0)) < 0)
            return -1; 

        hdr = (struct nlmsghdr *)buf; 
        if((NLMSG_OK(hdr, (unsigned int)read_len) == 0) || (hdr->nlmsg_type == NLMSG_ERROR))
            return -1;

        if(hdr->nlmsg_type == NLMSG_DONE){ 
            break;
        } else {
            buf += read_len;
            msg_len += read_len;
        }
 
        if((hdr->nlmsg_flags & NLM_F_MULTI) == 0) 
            break; 
        if((hdr->nlmsg_seq != (unsigned int)seq) || (hdr->nlmsg_pid != (unsigned int)pid))
            break; 
    }
    return msg_len; 
} 
 
static int parse_routes(struct nlmsghdr* hdr, struct route_info* rt_info)
{ 
    int rt_len = 0; 
    struct rtmsg* rt_msg = NULL; 
    struct rtattr* rt_attr = NULL; 
 
    rt_msg = (struct rtmsg*)NLMSG_DATA(hdr); 
 
    if ((rt_msg->rtm_family != AF_INET)  || (rt_msg->rtm_table != RT_TABLE_MAIN))
        return -1;

    rt_attr = (struct rtattr*)RTM_RTA(rt_msg); 
    rt_len = RTM_PAYLOAD(hdr); 
    for (; RTA_OK(rt_attr, rt_len); rt_attr = RTA_NEXT(rt_attr, rt_len) ) 
    { 
        switch (rt_attr->rta_type) 
        { 
        case RTA_OIF: 
            if_indextoname(*(int*)RTA_DATA(rt_attr), rt_info->if_name); 
            break; 
        case RTA_GATEWAY: 
            rt_info->gateway = *(uint32_t*)RTA_DATA(rt_attr); 
            break; 
        case RTA_PREFSRC: 
            rt_info->src_addr = *(uint32_t*)RTA_DATA(rt_attr); 
            break; 
        case RTA_DST: 
            rt_info->dst_addr = *(uint32_t*)RTA_DATA(rt_attr); 
            break; 
        } 
    } 
    return 0; 
} 

int get_default_gateway(const char* eth_name, char* gateway)
{ 
    char msg_buf[GATEWAY_BUFSIZE] = {0}; 
    int sock =0, len = 0, msg_seq = 0, ret = -1; 

    struct in_addr addr; 
    struct route_info ri; 
    struct route_info* rt_info = &ri; 
    struct nlmsghdr* nlmsg = NULL; 
 
    sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE); 
    if (sock < 0) 
        return -1; 
 
    nlmsg = (struct nlmsghdr*)msg_buf; 
    nlmsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)); 
    nlmsg->nlmsg_type = RTM_GETROUTE; 
    nlmsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST; 
    nlmsg->nlmsg_seq = msg_seq++; 
    nlmsg->nlmsg_pid = getpid(); 
 
    if(send(sock, nlmsg, nlmsg->nlmsg_len, 0) < 0){
        close(sock);
        return -1;
    } 

    if((len = read_routes(sock, msg_buf, msg_seq, getpid())) < 0){
        close(sock);
        return -1;
	}

    for(; NLMSG_OK(nlmsg, (unsigned int)len); nlmsg = NLMSG_NEXT(nlmsg, len))
	{ 
        memset(rt_info, 0, sizeof(struct route_info)); 
        if(0 == parse_routes(nlmsg, rt_info)) 
        { 
			if((strcmp(rt_info->if_name, eth_name) == 0) && (rt_info->dst_addr == 0)  && (rt_info->gateway != 0))
            { 
				addr.s_addr = rt_info->gateway;
                strcpy(gateway, inet_ntoa(addr)); 
				ret = 0;
				break;
            }  
        } 
    } 

    close(sock); 
    return ret; 
}

#define ETH_NAME "wlan0"
int main(void)
{
    char buf[32];
	char ip[32], netmask[32], mac[32];
    int ret = 0;

    ret = get_default_gateway(ETH_NAME, buf);
    if(ret == 0){
        printf("default gateway:%s\n", buf);
    } else {
        printf("no gateway\n");
    }

	ret = get_local_net_info(ETH_NAME, ip, netmask, mac);
	if(ret == 0) {
		printf("ip:%s, netmask:%s, mac:%s\n", ip, netmask, mac);
	} else {
		printf("no ip info\n");
	}

    return 0;
}
