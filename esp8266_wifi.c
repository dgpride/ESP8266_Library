/*
 ******************************************************************************
 * Copyright (C) 2015-2016 Gokul Sriram P.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 ******************************************************************************
 */

/*
 ******************************************************************************
 *			ESP8266 - WiFi Configuration Library
 *			(esp8266_wifi.c)
 ******************************************************************************
 */

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ioctl.h>
#include "esp8266.h"

static char read_buf[RECV_BUF_SIZE] = {'\0'};

/*static char esp8266_getchar(int esp_fd)
{
    char var = '\0';

    serial_read_char(esp_fd, &var, 0);
    return var;
    }*/

static void esp8266_getline(char *line_buf)
{
	int i = 0;

	do {
		serial_read_char(esp_fd, &line_buf[i++], 0);
//		printf("%c", line_buf[i - 1]);
    } while ((line_buf[i - 1] != '\r') && (line_buf[i - 1] != '\n'));
    
//	line_buf[i - 1] = '\n';
	line_buf[i] = '\0';
	strcat(read_buf, line_buf);
}

static int esp8266_status(int esp_fd, char *line_buf)
{
	if (line_buf == NULL) {
		char temp[RECV_BUF_SIZE];
		line_buf = temp;
	}
    
	do {
		esp8266_getline(line_buf);
	} while ((strncmp(line_buf, "OK", strlen("OK"))) &&
		 (strncmp(line_buf, "ERROR", strlen("ERROR"))) &&
		 (strncmp(line_buf, "FAIL", strlen("FAIL"))));
    
	if (!strncmp(line_buf, "OK", strlen("OK")))
		return 0; /* Command written to esp8266 successfully */
	else if (!strncmp(line_buf, "ERROR", strlen("ERROR")))
		return -1; /* Command write to esp8266 error */
	else if (!strncmp(line_buf, "FAIL", strlen("FAIL")))
		return -2; /* Failed to execute command */
}

static int esp8266_open(char *esp_port, unsigned int baudrate)
{
	int fd;

	fd = serial_open(esp_port, baudrate);

	if (fd == -1) {
		printf("Cannot open port '%s'..\n", esp_port);
	} else if (fd == -2) {
		printf("Baudrate error..\n");
	} else if (fd > 0) {
		printf("Port '%s' opened successfully..\n", esp_port);
		return fd;
	}
	exit(1);
}

static void esp8266_close(int esp_fd)
{
	serial_close(esp_fd);
}

static char esp8266_write(int esp_fd, const void *buffer, unsigned int nbytes)
{
	char err_no;

	err_no = serial_write(esp_fd, buffer, nbytes);

	if (err_no == 1)
		return 0;

	printf("Serial write to port failed.\n");
	return err_no;
}

static void esp8266_parse_line(char *line_buf, apdata_t *ptr)
{
	int i;
	int j;
	char field[73] = {'\0'};

	ptr->ecn = atoi(&line_buf[8]);

	for (i = 0, j = 11; line_buf[j] != '"' && j < 84; i++, j++)
		field[i] = line_buf[j];
	if (j < 84 && line_buf[j] == '"') {
		field[i] = '\0';
		strcpy(ptr->ssid, field);
	} else {
		printf("SSID exceeds allowable string length.\n");
		exit(1);
	}
	j += 2;
	ptr->rssi = atoi(&line_buf[j]);
    
	for (;atoi(&line_buf[j]); j++);
    
	j += 2;

	for (i = 0; line_buf[j] != '"'; i++, j++) {
		ptr->mac[i] = line_buf[j];
	}
	ptr->mac[i] = '\0';
	j += 2;

	ptr->channel = atoi(&line_buf[j]);

}

/* 
 * Opens serial port with baudrate specified by ESP_BAUDRATE
 *
 * WARNING: esp8266_init(port) should be called before calling any other API.
 */

int esp8266_init(char *esp_port)
{
	/*
	 * Opens serial port with the baudrate specified and provides a file
	 * handle
	 */
	return esp8266_open(esp_port, ESP_BAUDRATE);
}

/*
 * esp8266_start disables the echo mode and configures as STATION by default
 * with DHCP client enabled.
 */
void esp8266_start(int esp_fd)
{
	int err_no;

	/* Echo mode disabled */
	err_no = esp8266_write(esp_fd, ESP_ECHO_OFF, strlen(ESP_ECHO_OFF));

	if (!err_no)
		err_no = esp8266_status(esp_fd, NULL);

	if (err_no)
		printf("ERROR %d: ECHO: Write Failed..\n", err_no);
    
	/* Configure ESP8266 in STATION mode */
	err_no = esp8266_write(esp_fd, WIFI_MODE_SET_STA,
			       strlen(WIFI_MODE_SET_STA));

	if (!err_no)
		err_no = esp8266_status(esp_fd, NULL);

	if (!err_no)
		printf("Started as STATION..\n");
	else {
		printf("ERROR %d: Cannot start as STATION..\n", err_no);
		return -1;
	}

	/* Enable DHCP Client in STATION mode */
	err_no = esp8266_write(esp_fd, ESP_DHCP_CLIENT_ENABLE,
			       strlen(ESP_DHCP_CLIENT_ENABLE));

	if (!err_no)
		err_no = esp8266_status(esp_fd, NULL);

	if (!err_no)
		printf("DHCP client enabled..\n");
	return -1;
}

void esp8266_stop(int esp_fd)
{
	esp8266_close(esp_fd);
	printf("Connection closed.\n");
}

void esp8266_get_info(char *ssid)
{
	int i, j;
	char line_buf[RECV_BUF_SIZE] = {'\0'};
	char field[73] = {'\0'};
	char *command = malloc(sizeof(GET_AP_INFO)
			       + sizeof(ssid)
			       + sizeof("\"\r\n"));

	printf("Connection details..\n");

	/* Gets SSID, RSSI, MAC, Channel, Encryption */
	command[0] = '\0';

	strcat(command, GET_AP_INFO);
	strcat(command, ssid);
	strcat(command, "\"\r\n");

	esp8266_write(esp_fd, command, strlen(command));

	do {
		esp8266_getline(line_buf);
	} while (strncmp(line_buf, "+CWLAP:(", strlen("+CWLAP:(")));
    
	esp8266_parse_line(line_buf, &ap_current);
	free(command);

	/* Get STATION IP */
	command[0] = '\0';

	strcat(command, GET_STA_IP);
	esp8266_write(esp_fd, command, strlen(command));

	do {
		esp8266_getline(line_buf);
	} while (strncmp(line_buf, "+CIPSTA:\"", strlen("+CIPSTA:\"")));

	for (i = 0, j = 9; line_buf[j] != '"' && j < 84; i++, j++)
		field[i] = line_buf[j];

	if (j < 24 && line_buf[j] == '"') {
		field[i] = '\0';
		strcpy(ap_current.ip, field);
	}
    
	printf("SSID: %s, "
		"AP MAC: %s\n"
		"RSSI: %d, "
		"Channel: %d, "
		"Encryption: %s\n"
		"STA IP: %s, \n",
		ap_current.ssid, ap_current.mac, ap_current.rssi,
		ap_current.channel, ecn_type[ap_current.ecn], ap_current.ip);
}

/*
  esp8266_write(esp_fd, GET_STA_IP, strlen(GET_STA_IP));
  esp8266_write(esp_fd, GET_STA_MAC, strlen(GET_STA_MAC));
  esp8266_write(esp_fd, AP_SSID_GET, strlen(AP_SSID_GET));
*/

void esp8266_scan(int esp_fd)
{
	char line_buf[RECV_BUF_SIZE] = {'\0'};
	int error;

	read_buf[0] = '\0';

	printf("Scanning for APs in range..\n");
	esp8266_write(esp_fd, SCAN_AP, strlen(SCAN_AP));

	/* Checks whether the write operation is successful or failed */
	error = esp8266_status(esp_fd, line_buf);
	if (!error) {
		printf("APs in range..\n");
	//	esp8266_parse_list();
		puts(read_buf);
	} else if (error == -1) {
		printf("Error: Serial write error.");
		exit(1);
	} else if (error == -2) {
		printf("Error: Scan Failed.");
		exit(1);
	}
//	esp8266_close(esp_fd);
}

void esp8266_connect(char *ssid, char *pwd)
{
	int err_no;
	char *command = malloc(sizeof(AP_CONNECT)
			       + sizeof(ssid)
			       + sizeof("\",\"")
			       + sizeof(pwd)
			       + sizeof("\"\r\n"));
    
	command[0]='\0';
	strcat(command, AP_CONNECT);
	strcat(command, ssid);
	strcat(command, "\",\"");
	strcat(command, pwd);
	strcat(command, "\"\r\n");

	printf("Connecting to '%s'.. \n", ssid);
	esp8266_write(esp_fd, command, strlen(command));
	err_no = esp8266_status(esp_fd, NULL);

	if (!err_no) { /* Prints Connection details */
		printf("\rDONE.\n");
		free(command);
		strcpy(ap_current.pwd, pwd);
		esp8266_get_info(ssid);
	} else if (err_no == -2) {
		printf("\rFAILED.\n");
	} else if (err_no == -1) {
		printf("\rERROR.\n");
	}
}

void esp8266_disconnect () {
	printf("Disconnecting from '%s'.. ", ap_current.ssid);
  
	esp8266_write(esp_fd, AP_DISCONNECT, strlen(AP_DISCONNECT));
	printf("DONE.\n");
}
