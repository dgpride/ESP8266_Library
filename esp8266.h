/*
 ******************************************************************************
 * Copyright (C) 2015-2016 Gokul Sriram P. All rights reserved.
 * Author: Gokul Sriram P <dgpride@gmail.com>
 ******************************************************************************
 *			ESP8266 - WiFi Configuration Library
 *			(esp8266.h)
 ******************************************************************************
 */

#ifndef __ESP8266_H
#define __ESP8266_H

#define CMD_BUF_SIZE                    100
#define RECV_BUF_SIZE                   512
#define ESP_BAUDRATE                    115200
#define MAX_SSID_LEN                    73

/* ENCRYPTION TYPES */
#define ECN_OPEN                        0
#define ECN_WEP                         1
#define ECN_WPA_PSK                     2
#define ECN_WPA2_PSK                    3
#define ECN_WPA_WPA2_PSK                4

/* ESP8266 AT Instruction Set - Generic */
#define ESP_RST                         "AT+RST\r\n"
#define ESP_ECHO_ON                     "ATE1\r\n"
#define ESP_ECHO_OFF                    "ATE0\r\n"

/* ESP8266 AT Instruction Set - DHCP */
#define ESP_DHCP_CLIENT_ENABLE          "AT+CWDHCP=1,1\r\n"
#define ESP_DHCP_CLIENT_DISABLE         "AT+CWDHCP=1,0\r\n"
#define ESP_DHCP_SERVER_ENABLE          "AT+CWDHCP=0,1\r\n"
#define ESP_DHCP_SERVER_DISABLE         "AT+CWDHCP=0,0\r\n"
#define ESP_DHCP_BOTH_ENABLE            "AT+CWDHCP=2,1\r\n"
#define ESP_DHCP_BOTH_DISABLE           "AT+CWDHCP=2,0\r\n"

/* AT Instruction Set - WiFi Functions */
#define WIFI_MODE_GET                   "AT+CWMODE?\r\n"
#define WIFI_MODE_SET_STA               "AT+CWMODE=1\r\n"
#define WIFI_MODE_SET_AP                "AT+CWMODE=2\r\n"
#define WIFI_MODE_SET_STA_AP            "AT+CWMODE=3\r\n"
#define WIFI_MODE_SET_DEFAULT           "AT+CWMODE_DEF\r\n"
#define WIFI_MODE_SET_SESSION           "AT+CWMODE_CUR\r\n"

#define SCAN_AP                         "AT+CWLAP\r\n"
#define GET_AP_INFO                     "AT+CWLAP=\""
#define GET_AP                          "AT+CWJAP?\r\n"

#define AP_CONNECT                      "AT+CWJAP=\""
#define AP_DISCONNECT                   "AT+CWQAP\r\n"

#define AP_AUTOCONNECT_ON               "AT+CWAUTOCONN=1\r\n"
#define AP_AUTOCONNECT_OFF              "AT+CWAUTOCONN=0\r\n"

#define SET_STA_IP                      "AT+CIPSTA="
#define GET_STA_IP                      "AT+CIPSTA?\r\n"

#define SET_STA_MAC                     "AT+CIPSTAMAC="
#define GET_STA_MAC                     "AT+CIPSTAMAC?\r\n"

#define AP_SSID_GET                     "AT+CWJAP?\r\n"

/* Global VARIABLES */

char *ecn_type[] = {"OPEN",
		    "WEP",
		    "WPA_PSK",
		    "WPA2_PSK",
		    "WPA_WPA2_PSK"};

typedef struct ap_data {
	char ssid[MAX_SSID_LEN]; /*ssid of AP */
	char mac[18]; //[6][2]; /* MAC address of AP */
	char pwd[65]; /* Login Password */
	char ip[16]; /* Station IP */
	int rssi; /* Received Signal Strength Indication */
	unsigned int ecn; /* Encryption type */
	unsigned int channel;
	struct ap_data *prev;
	struct ap_data *next;
} apdata_t;

apdata_t ap_current, ap_node[10];

static void esp8266_getline(char *line_buf);

static int esp8266_status(int esp_fd, char *line_buf);

static int esp8266_open(char *esp_port, unsigned int baudrate);

static void esp8266_close(int esp_fd);

static void esp8266_write(int esp_fd, const void *buffer, unsigned int nbytes);

static void esp8266_parse_line(char *line_buf, apdata_t *ptr);

/* 
 * Opens serial port with baudrate specified by ESP_BAUDRATE
 *
 * WARNING: esp8266_init(port) should be called before calling any other API.
 */
int esp8266_init(char *esp_port);

void esp8266_start();

void esp8266_stop(int esp_fd);

void esp8266_get_info(char *ssid);


/*
  esp8266_write(esp_fd, GET_STA_IP, strlen(GET_STA_IP));
  esp8266_write(esp_fd, GET_STA_MAC, strlen(GET_STA_MAC));
  esp8266_write(esp_fd, AP_SSID_GET, strlen(AP_SSID_GET));
*/

void esp8266_scan();

void esp8266_connect(char *ssid, char *pwd);

void esp8266_disconnect ();

#endif /* __ESP8266_H */
