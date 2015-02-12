#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

int s, status;
int accx, accy, accz;
int gyrox, gyroy, gyroz;

void setup_bt()
{
	struct sockaddr_rc addr = { 0 };
	char dest[18] = "00:1A:7D:DA:71:13";

	// allocate a socket
	s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

	// set the connection parameters (who to connect to)
	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = (uint8_t) 1;
	str2ba( dest, &addr.rc_bdaddr );

	// connect to server
	status = connect(s, (struct sockaddr *)&addr, sizeof(addr));
}

void end()
{
	close(s);
}

int sign(int value)
{
	if(value > 0x8000)
		value = -((65535 - value) + 1);
	return value;
}

int main(int argc, char **argv)
{
	setup_bt();	

	char buf[6];
	while(1)
	{
		read(s, buf, sizeof(buf));
		accx = atoi(buf);
		read(s, buf, sizeof(buf));
		accy = atoi(buf);
		read(s, buf, sizeof(buf));
		accz = atoi(buf);

		read(s, buf, sizeof(buf));
		gyrox = atoi(buf);
		read(s, buf, sizeof(buf));
		gyroy = atoi(buf);
		read(s, buf, sizeof(buf));
		gyroz = atoi(buf);

		accx = sign(accx);
		accy = sign(accy);
		accz = sign(accz);

		gyrox = sign(gyrox);
		gyroy = sign(gyroy);
		gyroz = sign(gyroz);

		//printf("Dronex = %f\t Droney = %f\t Dronez = %f\n", dronex, droney, dronez);
		printf("AccX  = %d\t AccY  = %d\t AccZ  = %d\nGyroX = %d\t GyroY = %d\t GyroZ = %d\n\n",accx, accy, accz, gyrox, gyroy, gyroz);
	}
	
	end();
	return 0;
}

