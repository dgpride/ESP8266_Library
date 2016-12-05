#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include "esp8266_wifi.c"

#define MAX_LEN        73

static char ssid[MAX_LEN];
static char pwd[MAX_LEN];

void get_ssid_pwd()
{
    int i;
    
    printf("Enter SSID : ");
    for (i = 0; (ssid[i - 1] != '\n') && (i < MAX_LEN); i++) {
	ssid[i] = getchar();
    }
    ssid[i - 1] = '\0';

    if (i >= MAX_LEN) {
	printf("Invalid SSID..\n");
	exit(0);
    }

    usleep(1000);
    
    printf("Enter PASSWORD : ");
    for (i = 0; pwd[i - 1] != '\n' && (i < MAX_LEN); i++) {
	pwd[i] = getchar();
    }
    pwd[i - 1] = '\0';

    if (i >= MAX_LEN) {
	printf("Invalid PASSWORD..\n");
	exit(0);
    }
}

int main(int argc, char *argv[])
{
    char port[20] = {'\0'};
    int i;
    int j = 0;
    int fd;
    
    for (i = 0; i < argc; i++) {
	if (argv[i][0] == '-' && argv[i][1] == 'p' && argv[i + 1][0] == '/') {
	    strcpy(port, argv[i + 1]);
	    printf("Configured %s ...\n", argv[i + 1]);
	    j = 1;
	    break;
	}
    }
    
    if (!j) {
	printf("OPTION: -p <port> needed\n");
	exit(1);
    }
    
    fd = esp8266_init(port);
    esp8266_start();
    esp8266_scan();
    get_ssid_pwd();
    esp8266_connect(ssid, pwd);
    esp8266_get_info(NULL);
//    esp8266_disconnect();
//    esp8266_stop(fd);
    return 0;
}
