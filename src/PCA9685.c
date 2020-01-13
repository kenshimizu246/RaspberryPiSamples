
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


#define PCA9685_MODE1 0x0
#define PCA9685_PRESCALE 0xFE

#define LED0_ON_L 0x6
#define LEDALL_ON_L 0xFA

#define PIN_ALL 16


	//wiringPiSetupGpio();
void PWMFreq(int fd, float freq);
int baseReg(int pin);
void FullOff(int fd, int pin, int tf);

void Setup(const int i2cAddress/* = 0x40*/, float freq/* = 50*/){
	// Check i2c address
	int fd = wiringPiI2CSetup(i2cAddress);
	if (fd < 0){
		printf("wiringPiI2CSetup error!");
		exit(-1);
	}

	// Setup the chip. Enable auto-increment of registers.
	int settings = wiringPiI2CReadReg8(fd, PCA9685_MODE1) & 0x7F;
	int autoInc = settings | 0x20;

	wiringPiI2CWriteReg8(fd, PCA9685_MODE1, autoInc);

	// Set frequency of PWM signals. Also ends sleep mode and starts PWM output.
	if (freq > 0)
		PWMFreq(fd, freq);
}

void PWMFreq(int fd, float freq){
	// Cap at min 40 and max 1000
	freq = (freq > 1000 ? 1000 : (freq < 40 ? 40 : freq));

	// http://www.nxp.com/documents/data_sheet/PCA9685.pdf Page 24
	int prescale = (int)(25000000.0f / (4096 * freq) - 0.5f);

	int settings = wiringPiI2CReadReg8(fd, PCA9685_MODE1) & 0x7F;
	int sleep       = settings | 0x10;
	int wake	= settings & 0xEF;
	int restart = wake | 0x80;

	wiringPiI2CWriteReg8(fd, PCA9685_MODE1, sleep);
	wiringPiI2CWriteReg8(fd, PCA9685_PRESCALE, prescale);
	wiringPiI2CWriteReg8(fd, PCA9685_MODE1, wake);

	delay(1);
	wiringPiI2CWriteReg8(fd, PCA9685_MODE1, restart);
}

void PWMReset(int fd){
	wiringPiI2CWriteReg16(fd, LEDALL_ON_L    , 0x0);
	wiringPiI2CWriteReg16(fd, LEDALL_ON_L + 2, 0x1000);
}

void PWMWrite(int fd, int pin, int on, int off){
	int reg = baseReg(pin);

	wiringPiI2CWriteReg16(fd, reg    , on  & 0x0FFF);
	wiringPiI2CWriteReg16(fd, reg + 2, off & 0x0FFF);
}

void PWMRead(int fd, int pin, int *on, int *off){
	int reg = baseReg(pin);

	if (on)
		*on  = wiringPiI2CReadReg16(fd, reg);
	if (off)
		*off = wiringPiI2CReadReg16(fd, reg + 2);
}

void FullOn(int fd, int pin, int tf){
	int reg = baseReg(pin) + 1;	     // LEDX_ON_H
	int state = wiringPiI2CReadReg8(fd, reg);

	// Set bit 4 to 1 or 0 accordingly
	state = tf ? (state | 0x10) : (state & 0xEF);

	wiringPiI2CWriteReg8(fd, reg, state);

	// For simplicity, we set full-off to 0 because it has priority over full-on
	if (tf)
		FullOff(fd, pin, 0);
}

void FullOff(int fd, int pin, int tf){
	int reg = baseReg(pin) + 3;	     // LEDX_OFF_H
	int state = wiringPiI2CReadReg8(fd, reg);

	// Set bit 4 to 1 or 0 accordingly
	state = tf ? (state | 0x10) : (state & 0xEF);

	wiringPiI2CWriteReg8(fd, reg, state);
}

int baseReg(int pin)
{
	return (pin >= PIN_ALL ? LEDALL_ON_L : LED0_ON_L + 4 * pin);
}

int main(int argc, char *argv[])
{
	printf("PCA9685\n");
	wiringPiSetupGpio();
}
