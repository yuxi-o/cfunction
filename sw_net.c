#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define _GNU_SOURCE

#define CMD_SIZE	256	
#define IP_SIZE		16
#define SMALL_SIZE	16

#define NET_FILE	"/etc/network/interfaces"
#define	NET_DHCP_CONF	"auto %s\niface %s inet dhcp\n" 
#define NET_STATIC_CONF	"auto %s\niface %s inet static\naddress %s\nnetwork %s\ngateway %s\n"
#define NET_DNS_FILE	"/etc/resolv.conf"
#define NET_STATIC_DNS	"nameserve %s\nnameserver %s\n"

#define NET_START_CMD	"/etc/init.d/networking restart"
#define	NET_STOP_CMD	"/etc/init.d/networking stop" 
#define NET_CONF_CMD	"ifconfig %s %s netmask %s up"

#define WIFI_START_CMD	"ifconfig wlp1s0 up"
#define WIFI_STOP_CMD	"ifconfig wlp1s0 down"
#define WIFI_CONF_CMD	"wpa_supplicant -B -Dnl80211 -iwlp1s0 -c/etc/wpa_supplicant.conf"
#define WIFI_DHCP_CMD	"dhcpcd wlp1s0"
#define WIFI_KILL_CMD	"killall wpa_supplicant"

#define WIFI_CONF_FILE	"/etc/wpa_supplicant.conf"
#define WIFI_CONF_HEAD	"ctrl_interface=/var/run/wpa_supplicant\nupdate_config=1\n\nnetwork={\n\tscan_ssid=1\n"
#define WIFI_CONF_SSID	"\tssid=\"%s\"\n"
#define WIFI_CONF_PSK	"\tpsk=\"%s\"\n"
#define WIFI_CONF_KEY_MGMT	"\tkey_mgmt=%s\n"
#define WIFI_CONF_PAIRWISE	"\tpairwise=%s\n"
#define WIFI_CONF_TAIL	"}\n"

int main(int argc, char *argv[])
{
	if(argc != 3){
		printf("Usage: %s ethX dhcp|static|wifi\n", argv[0]);		
		return -1;
	}

	char ethx[IP_SIZE] = {0};
	char cmd[CMD_SIZE]={0};	
	char address[IP_SIZE]="172.17.6.222";
	char netmask[IP_SIZE]="255.255.255.0";
	char gateway[IP_SIZE]="172.17.6.254";

	char nameserver1[IP_SIZE]="8.8.8.8";
	char nameserver2[IP_SIZE]="8.8.4.4";

	char wifi_conf[CMD_SIZE*2] = {0};
	char wifi_conf_tmp[CMD_SIZE] = {0};
	char wifi_ssid[SMALL_SIZE]="Certusnet";
	char wifi_psk[CMD_SIZE]="fv40icVY";
	char wifi_key_mgmt[SMALL_SIZE]="wpa2_psk";
	char wifi_pairwise[SMALL_SIZE]="aes";

	int fd = 0;
	int ret = 0;
	
	strcpy(ethx, argv[1]);

	strcpy(cmd, WIFI_STOP_CMD);
	ret = system(cmd);
	if(ret != 0){
		system(cmd);
	}
	sleep(1);
	strcpy(cmd, WIFI_KILL_CMD);
	ret = system(cmd);
	if(ret != 0){
		system(cmd);
	}

	if(!strcmp(argv[2], "dhcp")){
		fd = open(NET_FILE, O_WRONLY | O_CREAT | O_TRUNC, 00644);	
		if(!fd){
			perror("open");
			return -1;
		}
		snprintf(cmd, sizeof(cmd), NET_DHCP_CONF, ethx, ethx);
		write(fd, cmd, sizeof(cmd));
		//syncfs(fd);
		sync();
		close(fd);

		sleep(1);
		strcpy(cmd, NET_START_CMD);
		ret = system(cmd);
		if(ret != 0){
			system(cmd);
		}
	} else if(!strcmp(argv[2], "static")){
		fd = open(NET_FILE, O_WRONLY | O_CREAT | O_TRUNC, 00644);	
		if(!fd){
			perror("open");
			return -1;
		}
		snprintf(cmd, sizeof(cmd), NET_STATIC_CONF, ethx, ethx, address, netmask, gateway);
		write(fd, cmd, sizeof(cmd));
		//syncfs(fd);
		sync();
		close(fd);

		fd = open(NET_DNS_FILE, O_WRONLY | O_CREAT | O_TRUNC, 00644);	
		if(!fd){
			perror("open");
			return -1;
		}
		snprintf(cmd, sizeof(cmd), NET_STATIC_DNS, nameserver1, nameserver2);
		write(fd, cmd, sizeof(cmd));
		//syncfs(fd);
		sync();
		close(fd);

		sleep(1);
		sprintf(cmd, NET_CONF_CMD, ethx, address, netmask);
		ret = system(cmd);
		if(ret != 0){
			system(cmd);
		}
		strcpy(cmd, NET_START_CMD);
		ret = system(cmd);
		if(ret != 0){
			system(cmd);
		}
	} else if(!strcmp(argv[2], "wifi")){
		// configure wifi
		strcpy(wifi_conf, WIFI_CONF_HEAD);
		sprintf(wifi_conf_tmp, WIFI_CONF_SSID, wifi_ssid);
		strcat(wifi_conf, wifi_conf_tmp);
		sprintf(wifi_conf_tmp, WIFI_CONF_PSK, wifi_psk);
		strcat(wifi_conf, wifi_conf_tmp);
		if(!strcmp("wep", wifi_key_mgmt)){
			sprintf(wifi_conf_tmp, WIFI_CONF_KEY_MGMT, "WPA-EAP");
			strcat(wifi_conf, wifi_conf_tmp);
		} else if(!strcmp("wpa2_psk", wifi_key_mgmt)){
			sprintf(wifi_conf_tmp, WIFI_CONF_KEY_MGMT, "WPA-PSK");
			strcat(wifi_conf, wifi_conf_tmp);
		} else {
			// auto : none
		}
		if(!strcmp("aes", wifi_pairwise)){
			sprintf(wifi_conf_tmp, WIFI_CONF_PAIRWISE, "CCMP");
			strcat(wifi_conf, wifi_conf_tmp);
		} else if(!strcmp("tkip", wifi_pairwise)){
			sprintf(wifi_conf_tmp, WIFI_CONF_PAIRWISE, "TKIP");
			strcat(wifi_conf, wifi_conf_tmp);
		} else {
			// auto : none
		}
		strcat(wifi_conf, WIFI_CONF_TAIL);

		fd = open(WIFI_CONF_FILE, O_WRONLY | O_CREAT | O_TRUNC, 00644);	
		if(!fd){
			perror("open");
			return -1;
		}
		write(fd, wifi_conf, strlen(wifi_conf));
		//syncfs(fd);
		sync();
		close(fd);
		
		// start wifi
		strcpy(cmd, NET_STOP_CMD);
		ret = system(cmd);
		if(ret != 0){
			system(cmd);
		}

		strcpy(cmd, WIFI_START_CMD);
		ret = system(cmd);
		if(ret != 0){
			system(cmd);
		}
		sleep(1);
		strcpy(cmd, WIFI_CONF_CMD);
		ret = system(cmd);
		if(ret != 0){
			system(cmd);
		}
		sleep(1);
		strcpy(cmd, WIFI_DHCP_CMD);
		ret = system(cmd);
		if(ret != 0){
			system(cmd);
		}
		sleep(2);
	} else {
		return -1;
	}

	return 0;
}

