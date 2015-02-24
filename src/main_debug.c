#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <ncurses.h>
#include <math.h>

#define OFFSET_SAMPLE 	500
#define GYRO_SENS 		65.5
#define FREQ 			30
#define PRINT_OFFSET_Y	4

int s, status;
int accx, accy, accz;
int gyrox, gyroy, gyroz;
int gsens;
int xconsi, yconsi;
double errorx, sum_errorx = 0, prev_errorx = 0;
double errory, sum_errory = 0, prev_errory = 0;
double gyrox_offset, gyroy_offset, gyroz_offset;

void get_sensor_data()
{
	char buf[6];

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

	gyrox = (sign(gyrox) - (int)gyrox_offset) / GYRO_SENS;
	gyroy = (sign(gyroy) - (int)gyroy_offset) / GYRO_SENS;
	gyroz = (sign(gyroz) - (int)gyroz_offset) / GYRO_SENS;
}

void get_offset()
{
	char buf[6];
	int i;

	gyrox_offset = 0;
	gyroy_offset = 0;
	gyroz_offset = 0;

	for (i = 0; i < OFFSET_SAMPLE; i++)
	{
		read(s, buf, sizeof(buf));
		read(s, buf, sizeof(buf));
		read(s, buf, sizeof(buf));

		read(s, buf, sizeof(buf));
		gyrox = atoi(buf);
		read(s, buf, sizeof(buf));
		gyroy = atoi(buf);
		read(s, buf, sizeof(buf));
		gyroz = atoi(buf);

		gyrox = sign(gyrox);
		gyroy = sign(gyroy);
		gyroz = sign(gyroz);

		gyrox_offset += (double)gyrox;
		gyroy_offset += (double)gyroy;
		gyroz_offset += (double)gyroz;
	}

	gyrox_offset = gyrox_offset / OFFSET_SAMPLE;
	gyroy_offset = gyroy_offset / OFFSET_SAMPLE;
	gyroz_offset = gyroz_offset / OFFSET_SAMPLE;
}

void setup_bt()
{
	struct sockaddr_rc addr = { 0 };
	char dest[18] = "00:1A:7D:DA:71:13";

	// allocate a socket
	s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

	// set the connection parameters (who to connect to)
	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = (uint8_t) 1;
	str2ba(dest, &addr.rc_bdaddr);

	// connect to server
	status = connect(s, (struct sockaddr *)&addr, sizeof(addr));
}

void end()
{
	close(s);
}

int sign(int value)
{
	if (value > 0x8000)
		value = -((65535 - value) + 1);
	return value;
}

void motors_draw(int motor1, int motor2, int motor3, int motor4)
{
	int i, j;

	for (i = 0; i < 4; i++)
		for (j = 0; j <= (motor1 * 30 / 100); j++)
			mvprintw(j + PRINT_OFFSET_Y, i + 1, "-");

	for (i = 8; i < 12; i++)
		for (j = 0; j <= (motor2 * 30 / 100); j++)
			mvprintw(j + PRINT_OFFSET_Y, i + 1, "-");

	for (i = 16; i < 20; i++)
		for (j = 0; j <= (motor3 * 30 / 100); j++)
			mvprintw(j + PRINT_OFFSET_Y, i + 1, "-");

	for (i = 24; i < 28; i++)
		for (j = 0; j <= (motor4 * 30 / 100); j++)
			mvprintw(j + PRINT_OFFSET_Y, i + 1, "-");

	mvprintw(3, 0, "Motor1");
	mvprintw(3, 8, "Motor2");
	mvprintw(3, 16, "Motor3");
	mvprintw(3, 24, "Motor4");
}

int main(int argc, char **argv)
{
	double ax = 0, ay = 0, az = 0;
	double gx = 0, gy = 0, gz = 0;
	double commandex, kpx, kdx, kix;
	int motor1, motor2, motor3, motor4;

	motor1 = 0;
	motor2 = 0;
	motor3 = 0;
	motor4 = 0;

	kpx = 300;
	kdx = 6;
	kix = 500;

	double commandey, kpy, kdy, kiy;

	kpy = 300;
	kdy = 6;
	kiy = 500;

	int key;

	xconsi = 0;
	yconsi = 0;

	initscr();
	mvprintw(0, 0, "Waiting for Bluetooth connection...");
	setup_bt();
	nodelay(stdscr, TRUE);
	noecho();

	get_offset();

	while (1)
	{
		get_sensor_data();

		ay = atan2(accx, sqrt(pow(accy, 2) + pow(accz, 2))) * 180 / M_PI;
		ax = atan2(accy, sqrt(pow(accx, 2) + pow(accz, 2))) * 180 / M_PI;

		gx = gx + gyrox / FREQ;
		gy = gy + gyroy / FREQ;

		// complementary filter
		// tau = DT*(A)/(1-A)
		// = 0.48sec
		gx = gx * 0.96 + ax * 0.04;
		gy = gy * 0.96 + ay * 0.04;

		errorx = (double)xconsi - gx;
		sum_errorx += errorx;
		commandex =
			kpx * errorx + kix * sum_errorx + kdx * (errorx - prev_errorx);
		prev_errorx = errorx;

		errory = (double)yconsi - gx;
		sum_errory += errory;
		commandey =
			kpy * errory + kiy * sum_errory + kdy * (errory - prev_errory);
		prev_errory = errory;

		mvprintw(0, 0,
				 "Xvalue  = %f\t Yvalue  = %f\nXconsi = %d\t Yconsi = %d\nXcomma = %f\t Ycomma = %f",
				 gx, gy, xconsi, yconsi, commandex, commandey);

		/* motor1 -= commandex; motor2 -= commandex; motor3 += commandex;
		   motor4 += commandex;

		   motor1 -= commandey; motor3 -= commandey; motor2 += commandey;
		   motor4 += commandey; */

		// motors_draw(motor1, motor2, motor3, motor4);
		key = getch();

		if (key == 'z')
			if (xconsi < 90)
				xconsi++;
			else if (key == 's')
				if (xconsi > -90)
					xconsi--;
				else if (key == 'q')
					if (yconsi > -90)
						yconsi--;
					else if (key == 'd')
						if (yconsi < 90)
							yconsi++;

		refresh();
	}

	endwin();
	end();
	return 0;
}
