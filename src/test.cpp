#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
char buf[12];
int fd; /* port file descriptor */
char port[20] = "/dev/ttyUSB0"; /* port to connect to */
speed_t baud = B230400; /* baud rate */

int serialPortInit( void )
{
	fd = open(port, O_RDWR); /* connect to port */
	if (fd < 0)
	{
		perror("open_port: Cannot open serialport");		
		return 0;
	}
	else
	{
		printf("Port Opened successfully!\n");
		struct termios settings;
		tcgetattr(fd, &settings);
		cfsetospeed(&settings, baud); /* baud rate */
		settings.c_cflag &= ~PARENB; /* no parity */
		settings.c_cflag &= ~CSTOPB; /* 1 stop bit */
		settings.c_cflag &= ~CSIZE;
		settings.c_cflag |= CS8 | CLOCAL; /* 8 bits */
		settings.c_lflag = ICANON; /* canonical mode */
		settings.c_oflag &= ~OPOST; /* raw output */

		tcsetattr(fd, TCSANOW, &settings); /* apply the settings */
		tcflush(fd, TCOFLUSH);
	}
}

void sendMessage(int ID, int flag, int Index, int Subindex, int Data )
{
	char buf[20];
	char sum=0;
	int i=0;
	buf[0]=0xff;
	buf[1]=0x55;
	// Id
	buf[2]=ID;
	// Flag
	buf[3]=flag;
	// Index
	buf[4]=(Index>>8);
	buf[5]=(Index&0xff);
	// Subindex
	buf[6]=Subindex;
	// Data
	buf[7]=(Data>>24)&0xff;
	buf[8]=(Data>>16)&0xff;
	buf[9]=(Data>>8)&0xff;
	buf[10]=(Data&0xff);
	// Sum
	for(i=0;i<11;i++)
	{
		sum += buf[i];
	}
	buf[11]=sum;
	write(fd,buf,12);
}
int main()
{
	int id = 0x01;
	int flag = 0x05;
	int index =0x6060;
	int subindex = 0x00;
	int data = 0x1;	
	serialPortInit();
	if( fd >0 )		
		while(1)	
		{
			sleep(3);
			//data++;
			sendMessage( id, flag, index, subindex, data );
			printf("data=%x\n",data);
		}
	return 0;
}
