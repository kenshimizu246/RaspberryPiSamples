
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>


#define PIN_TRIG 21
#define PIN_ECHO 22

// 1sec
#define TIMEOUT 1000

void init()
{
	wiringPiSetupGpio();
}

void setup()
{
	pinMode(PIN_TRIG, OUTPUT);
	pinMode(PIN_ECHO, INPUT);
	digitalWrite(PIN_TRIG, LOW);
}

unsigned int getTime(){
	struct timeval tv;
	unsigned int ms;

	gettimeofday(&tv, NULL);
	ms =  (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

	return ms;
}

float mesure()
{
	unsigned int signoff, signon, prev;

	digitalWrite(PIN_TRIG, HIGH);
	usleep(0.00001 * 1000000); // microsecond... 1sec = 1 * 1000000
	digitalWrite(PIN_TRIG, LOW);
	prev = getTime();
	while(digitalRead(PIN_ECHO) == LOW){
		signoff = getTime();
		if((prev - signoff) > TIMEOUT){
			return -1;
		}
	}
	prev = getTime();
	while(digitalRead(PIN_ECHO) == HIGH){
		signon = getTime();
		if((prev - signon) > TIMEOUT){
			return -1;
		}
	}
	return ((signon - signoff) * 34)/2;
}

int main(int argc, char *argv[])
{
	//printf("HC-SR04\n");

	while(1){
		double distance;
		unsigned int signoff, signon;
		signoff = getTime();
		usleep(1 * 1000000); // microsecond... 1sec = 1 * 1000000
		signon = getTime();
		distance = ((signon - signoff) * 34) / 2;
		printf("time:%f : %d : %d \n", distance, signon, signoff);
	}

	digitalWrite(PIN_TRIG, LOW);
	digitalWrite(PIN_ECHO, LOW);

	//printf("DRV8835 done!\n");
}
/*
time:765511072
time:765512072
time:765513072
time:765514072
time:765515072
*/
