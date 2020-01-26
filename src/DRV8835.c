
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>


#define PIN_MODE 17
#define PIN_AIN1 27
#define PIN_AIN2 22
#define PIN_BIN1 23
#define PIN_BIN2 24

int main(int argc, char *argv[])
{
	printf("DRV8835\n");

	wiringPiSetupGpio();

	pinMode(PIN_MODE, OUTPUT);
	pinMode(PIN_AIN1, OUTPUT);
	pinMode(PIN_AIN2, OUTPUT);
	pinMode(PIN_BIN1, OUTPUT);
	pinMode(PIN_BIN2, OUTPUT);

	printf("DRV8835 mode done!\n");

	digitalWrite(PIN_MODE, LOW);

	printf("DRV8835 mode low!\n");

	double intval = 0.005 * 1000000; // microsecond... 1sec = 1 * 1000000
	//double intval = 0.01 * 1000000; // microsecond... 1sec = 1 * 1000000
	//double intval = 1 * 1000000; // microsecond... 1sec = 1 * 1000000

	digitalWrite(PIN_AIN1, HIGH);

	for(int i = 0; i < 512; i++){
		printf("DRV8835 %d 1!\n", i);
		digitalWrite(PIN_BIN2, LOW);
		digitalWrite(PIN_BIN1, HIGH);
		usleep(intval);

		printf("DRV8835 %d 2!\n", i);
		digitalWrite(PIN_AIN1, LOW);
		digitalWrite(PIN_AIN2, HIGH);
		usleep(intval);

		printf("DRV8835 %d 3!\n", i);
		digitalWrite(PIN_BIN1, LOW);
		digitalWrite(PIN_BIN2, HIGH);
		usleep(intval);

		printf("DRV8835 %d 4!\n", i);
		digitalWrite(PIN_AIN2, LOW);
		digitalWrite(PIN_AIN1, HIGH);
		usleep(intval);
	}
	digitalWrite(PIN_MODE, LOW);

	digitalWrite(PIN_AIN1, LOW);
	digitalWrite(PIN_AIN2, LOW);
	digitalWrite(PIN_BIN1, LOW);
	digitalWrite(PIN_BIN2, LOW);

	printf("DRV8835 done!\n");
}
	
