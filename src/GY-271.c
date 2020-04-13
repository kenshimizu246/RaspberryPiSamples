
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
    //printf("new data is ready!\n");
    return 0;
  } else {
    printf("no new data!\n");
    return -1;
  }

  // OVL: “0”: normal, “1”: data overflow
  if((status & 0x02) > 0){
    printf("data overflow!\n");
    return -2;
  } else {
    //printf("normal!\n");
  }

  // DOR: “0”: normal, “1”: data skipped for reading
  if((status & 0x04) > 0){
    printf("data skipped for reading!\n");
    return -3;
  } else {
    //printf("normal!\n");
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

  ret = wiringPiI2CWriteReg8(fd, 0x0a, 0x80);
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

  int lowX, highX, lowY, highY, lowZ, highZ;
  lowX = lowY = lowZ = 0xffff;
  highX = highY = highZ = 0;

  while(1){
    //printf("-----------------------------------------\n");

    int status;
    do{
      status = wiringPiI2CReadReg8(fd, 0x06);
      //printf("%d ", status);
    } while((status & 0x01) != 1);
    //printf("\n");
    check_status(fd);
    //int x = wiringPiI2CReadReg8(fd, 0x00) | (wiringPiI2CReadReg8(fd, 0x01) << 8);
    //int y = wiringPiI2CReadReg8(fd, 0x02) | (wiringPiI2CReadReg8(fd, 0x03) << 8);
    //int z = wiringPiI2CReadReg8(fd, 0x04) | (wiringPiI2CReadReg8(fd, 0x05) << 8);
    int lsbX = wiringPiI2CReadReg8(fd, 0x00);
    int msbX = wiringPiI2CReadReg8(fd, 0x01);
    int lsbY = wiringPiI2CReadReg8(fd, 0x02);
    int msbY = wiringPiI2CReadReg8(fd, 0x03);
    int lsbZ = wiringPiI2CReadReg8(fd, 0x04);
    int msbZ = wiringPiI2CReadReg8(fd, 0x05);
    int x = lsbX | (msbX << 8);
    int y = lsbY | (msbY << 8);
    int z = lsbZ | (msbZ << 8);
    printf("xyz: %6d:%4x, %6d:%4x, %6d:%4x ", x, x, y, y, z, z);
    if(x > 0 && x < lowX) lowX = x;
    if(x > highX) highX = x;
    if(y > 0 && y < lowY) lowY = y;
    if(y > highY) highY = y;
    if(z > 0 && z < lowZ) lowZ = z;
    if(z > highZ) highZ = z;
    if(lowX == highX || lowY == highY){
      printf("Not enough data is available!\n");
      continue;
    }
    printf("; high: %d, %d, %d ", highX, highY, highZ);
    printf("; low: %d, %d, %d ", lowX, lowY, lowZ);

    int ofsX = (highX + lowX)/2;
    int ofsY = (highY + lowY)/2;
    int ofsZ = (highZ + lowZ)/2;

    x -= ofsX;
    y -= ofsY;
    z -= ofsZ;

    float fx = (float)x/(highX - lowX);
    float fy = (float)y/(highY - lowY);
    float fz = (float)z/(highZ - lowZ);

    float heading = atan2(fy, fx) * 180.0 /3.1415926535;
    if(heading <= 0){heading += 360;}

    printf("; %-3.8f\n", heading);
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
