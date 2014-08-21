#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <assert.h>

typedef struct
{
	int start;
	char lastInData;
	char buf[12];
	int n;
	char sum;
	int ID;
	int Flag;
	int Index;
	int SubIndex;
	int Data;
}serialObj;

serialObj serial;

void serialObjInit( void )
{
	serial.start = 0;
	serial.lastInData = 0;
	serial.n = 0;
	serial.sum = 0xaa+0xfe;
	serial.ID = 0;
	serial.Index = 0;
	serial.SubIndex = 0;
	serial.Data = 0;
}

void error(const char *msg)
{
	perror(msg);
	exit(0);
}
void set_nonblock(int socket)
{
	int flags;
	flags = fcntl(socket,F_GETFL,0);
	assert(flags != -1);
	fcntl(socket, F_SETFL, flags | O_NONBLOCK);
}
int elCommInit(char *portName, int baud)
{
		struct termios options;
		int fd;
		char *ip;
		char *tcpPortNumString;
		long int tcpPortNum;
		int sockfd;
		struct sockaddr_in serv_addr;
		struct hostent *server;
		int rv;
		if (*portName == '/') {	// linux serial port names always begin with /dev
			printf("Opening serial port %s\n", portName);
		fd = open(portName, O_RDWR | O_NOCTTY | O_NDELAY);
		if (fd == -1){
			//Could not open the port.
			perror("init(): Unable to open serial port - ");
		}
	else{
	fcntl(fd, F_SETFL, FNDELAY); // Sets the read() function to return NOW and not wait for data to enter buffer if there isn't anything there.
	//Configure port for 8N1 transmission
	tcgetattr(fd, &options);	//Gets the current options for the port
	cfsetispeed(&options, B115200);	//Sets the Input Baud Rate
	cfsetospeed(&options, B115200);	//Sets the Output Baud Rate
	options.c_cflag |= (CLOCAL | CREAD);	//? all these set options for 8N1 serial operations
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_cflag &= ~CRTSCTS;
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);	// set raw mode
	options.c_iflag &= ~(IXON | IXOFF | IXANY);	// disable SW flow control
	options.c_oflag &= ~OPOST;
	tcsetattr(fd, TCSANOW, &options);	//Set the new options for the port "NOW"
	}
	return fd;
	} 
	return -1;
}
int elCommRead(int fd)
{
	unsigned char c;
	unsigned int i;
	int rv;
	rv = read(fd, &c, 1);	// read one byte
	i = c;	// convert byte to an int for return
	if (rv > 0)
	return i;	// return the character read
	return rv;	// return -1 or 0 either if we read nothing, or if read returned negative
}
int elCommWrite(int fd, uint8_t* data, int len)
{
	int rv;
	int length = len;
	int totalsent = 0;
	while (totalsent < length) {
	rv = write(fd, data+totalsent, length);
	if (rv < 0)
	perror("write(): error writing - trying again - ");
	else
	totalsent += rv;
	}
	return rv;
}


void sendMessage(int fd, int ID, int flag, int Index, int Subindex, int Data )
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
	int n;
	int id;
	int fd;
	char port[] = "/dev/ttyUSB0";
	char r[20];
	char inData;
	int i;
	serialObjInit();
	fd = elCommInit(port,115200);
	if(fd>0)
	while(1)
	{
		n=read(fd,&r[0],sizeof(r));
		if(n>0)
		{
			r[n]='\0';
//			for(i=0;i<n;i++)			
//			{
//				printf("%x\n",r[i]);
//			}
//			printf("%d%s\n",n,r);
			for(i=0;i<n;i++)
			{
				inData = r[i];
//printf("inData=%x\n",inData&0xff);
//printf("lastData=%x\n",serial.lastInData&0xff);
				if( ((inData&0xff) == 0xaa)&&((serial.lastInData&0xff) == 0xfe) )
				{
					serial.start |= 0x01;
					serial.sum = 0xaa+0xfe;
				}
				serial.lastInData = inData;
//printf("ok=%x\n",serial.start);
				// Start receive data
				if( serial.start != 0 )
				{
					//printf("ok=\n");
					serial.buf[serial.n]=inData&0xff;
					serial.n++;
					
//printf("n is: %x\n",serial.n);
					if( serial.n >10 )
					{
						for(serial.n=1;serial.n<10;serial.n++)
						{
							serial.sum += serial.buf[serial.n];
//printf("buf[%d]=%x\n",serial.n,serial.buf[serial.n]);
						}
//printf("sum is: %x\n",serial.sum);
//printf("buf[10] is: %x\n",serial.buf[10]);
						if( serial.sum == serial.buf[10] )
						{
							serial.ID = serial.buf[1];
							serial.Flag = serial.buf[2];
							serial.Index = (serial.buf[3]<<8) + serial.buf[4];
							serial.SubIndex = serial.buf[5];
							serial.Data = (serial.buf[6]<<24) + (serial.buf[7]<<16) + (serial.buf[8]<<8) + serial.buf[9];
							//printf("ID=%d\n",serial.ID);
							//printf("Flag=%d\n",serial.Flag);
							//printf("Index=%d\n",serial.Index);
							//printf("SubIndex=%d\n",serial.SubIndex);
							printf("Data=%d\n",serial.Data);
						}

//						CAN_send(serial.ID,serial.Flag,serial.Index,serial.SubIndex,serial.Data);
						serial.n = 0;
						serial.start = 0;
					}
				}
			}
		}
		usleep(10000);
	}
	return 0;
}
