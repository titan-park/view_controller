#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "gpio.h"

#define BUF_LEN 128
#define MOTOR_EN 182
#define DIR_R 48
#define DIR_L 49

int main(int argc, char *argv[]){
	char buffer[BUF_LEN];
	struct sockaddr_in server_addr, client_addr;
	char temp[20];
	int server_fd, client_fd = -1;
	int len, msg_size;
	char pre_order = 'S';
	int err;

	buffer[0] = 'S';

	if(argc != 2){
		printf("usage: %s [port]\n", argv[0]);
		exit(0);
	}
	

	if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		printf("Server: Can't open stream socket\n");
		exit(0);
	}

	memset(&server_addr, 0x00, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(argv[1]));

	if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		printf("Server: Can't bind local address. \n");
		exit(0);
	}
	
	if(listen(server_fd, 5) < 0){
		printf("Server: Can't listening connect.\n");
		exit(0);
	}

	memset(buffer, 0x00, sizeof(buffer));
	printf("Server: waiting connection requset.\n");
	len = sizeof(client_addr);
	
	err = gpio_init(MOTOR_EN, 1);
	if(err != 1)	printf("MOTOR EN GPIO Init Error.\n");
	err = gpio_init(DIR_R, 1);
	if(err != 1)	printf("DIR_R GPIO Init Error.\n");
	err = gpio_init(DIR_L, 1);
	if(err != 1)	printf("DIR_L GPIO Init Error.\n");

	while(err != -1){

		if(client_fd < 0){
			client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);

			if(client_fd < 0){
				printf("Server: accpet failed.\n");
				exit(0);
			}
			inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, temp);
			printf("Server: %s client connected.\n", temp);
		}
		else{
			msg_size = read(client_fd, buffer, 1024);
			write(client_fd, buffer, msg_size);
			printf("Server: %s received data.\n", buffer);

			if(buffer[0] == 'x' || client_fd < 0){
				gpio_write(MOTOR_EN, 0);
				gpio_write(DIR_R, 0);
				gpio_write(DIR_L, 0);

				close(client_fd);
				printf("Server: %s client closed.\n", temp);
				client_fd = -1;
			}
			else{
				if(buffer[0] != pre_order){
					pre_order = buffer[0];
					if(pre_order == 'S'){
						gpio_write(MOTOR_EN, 0);
						gpio_write(DIR_R, 0);
						gpio_write(DIR_L, 0);
					}
					else{
						gpio_write(MOTOR_EN, 1);
						if(pre_order == 'F'){						
							gpio_write(DIR_R, 1);
							gpio_write(DIR_L, 1);
						}
						else if(pre_order == 'R'){
							gpio_write(DIR_R, 1);
							gpio_write(DIR_L, 0);
						}
						else if(pre_order == 'L'){
							gpio_write(DIR_R, 0);
							gpio_write(DIR_L, 1);
						}
						else if(pre_order == 'B'){
							gpio_write(DIR_R, 0);
							gpio_write(DIR_L, 0);
						}
					}

				}
				pre_order = buffer[0];
			}
		}
	}
	
	return 0;
}
