
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
#include <math.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>

#define GY_271_ADDR 0x0d

#define REG_CTRL 0x09

// mode 00:Standby 01:Continuous
#define MODE_STANDBY    0x00
#define MODE_CONTINUOUS 0x01

// ODR (output Data Rate) 00:10Hz 01:50Hz 10:100Hz 11:200Hz
#define ODR_10HZ  0x00
#define ODR_50HZ  0x04
#define ODR_100HZ 0x08
#define ODR_200HZ 0x0C

// RNG (Full Scale) 00:2G 01:8G
#define RNG_2G 0x00
#define RNG_8G 0x10

// OSR (Over Sample Ratio)  00:512 01:256 10:128 11:64
#define OSR_512 0x00
#define OSR_256 0x40
#define OSR_128 0x80
#define OSR_64  0xC0

int check_status(int fd){
  int status = wiringPiI2CReadReg8(fd, 0x06);

  // DRDY: “0”: no new data, “1”: new data is ready
  if((status & 0x01) > 0){
    printf("new data is ready!\n");
  } else {
    printf("no new data!\n");
    //return -1;
  }

  // OVL: “0”: normal, “1”: data overflow
  if((status & 0x02) > 0){
    printf("data overflow!\n");
    //return -2;
  } else {
    printf("normal!\n");
  }

  // DOR: “0”: normal, “1”: data skipped for reading
  if((status & 0x04) > 0){
    printf("data skipped for reading!\n");
    //return -3;
  } else {
    printf("normal!\n");
  }

  return 0;
}


int main(int argc, char *argv[])
{
  int ret = 0;

  printf("GY-271\n");

  wiringPiSetupGpio();

  // Check i2c address
  int fd = wiringPiI2CSetup(GY_271_ADDR);
  if (fd < 0){
    printf("wiringPiI2CSetup error!");
    exit(-1);
  }

  ret = wiringPiI2CWriteReg8(fd, 0x0b, 0x01);
  if(ret < 0){
    printf("Error Reset %d\n", ret);
    return ret;
  }

  int ctrl = MODE_CONTINUOUS | ODR_200HZ | RNG_8G | OSR_512;
  printf("ctrl %x\n", ctrl);
  ret = wiringPiI2CWriteReg8(fd, 0x09, ctrl);
  if(ret < 0){
    printf("Error Control %d\n", ret);
    return ret;
  }

  unsigned int xlow, xhigh, ylow, yhigh, zlow, zhigh;
  xlow = ylow = zlow = 0xffff;
  xhigh = yhigh = zhigh = 0;

  while(1){
    printf("-----------------------------------------\n");

    int status;
    do{
      status = wiringPiI2CReadReg8(fd, 0x06);
      printf("--- %d\n", status);
    } while((status & 0x01) != 1);
    check_status(fd);
    int x = wiringPiI2CReadReg8(fd, 0x00) | (wiringPiI2CReadReg8(fd, 0x01) << 8);
    int y = wiringPiI2CReadReg8(fd, 0x02) | (wiringPiI2CReadReg8(fd, 0x03) << 8);
    int z = wiringPiI2CReadReg8(fd, 0x04) | (wiringPiI2CReadReg8(fd, 0x05) << 8);
    printf("xyz: %d %d %d\n", x, y, z);
    if(x < xlow) xlow = x;
    if(x > xhigh) xhigh = x;
    if(y < ylow) ylow = y;
    if(y > yhigh) yhigh = y;
    if(z < zlow) zlow = z;
    if(z > zhigh) zhigh = z;
    if(xlow == xhigh || ylow == yhigh){
            printf("Not enough data is available!\n");
      continue;
    }
    x -= (xhigh + xlow)/2;
    y -= (yhigh + ylow)/2;
    z -= (zhigh + zlow)/2;

    float fx = (float)x/(xhigh - xlow);
    float fy = (float)y/(yhigh - ylow);
    float fz = (float)z/(zhigh - zlow);

    int heading = 180.0 * atan2(fy, fx)/3.1415926535;
    if(heading <= 0){heading += 360;}

    printf(" -> %d\n", heading);
    //usleep(1000000); //1sec
  }

  printf("GY-271 done!\n");
}
/*
time:765511072
time:765512072
time:765513072
time:765514072
time:765515072
*/
