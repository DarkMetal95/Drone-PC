#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <math.h>

#define OFFSET_SAMPLE 	500
#define GYRO_SENS 	65.5
#define FREQ 		30.0f

void timer_int();

int s, status;
int accx, accy, accz;
int gyrox, gyroy, gyroz;
int gsens;
double gyrox_offset, gyroy_offset, gyroz_offset;
double ax = 0, ay = 0, az = 0;
double gx = 0, gy = 0, gz = 0;

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

	gyrox = (sign(gyrox) - (int) gyrox_offset) / GYRO_SENS;
	gyroy = (sign(gyroy) - (int) gyroy_offset) / GYRO_SENS;
	gyroz = (sign(gyroz) - (int) gyroz_offset) / GYRO_SENS;
}

void get_offset()
{
	char buf[6];
	int i;	

	gyrox_offset = 0;
	gyroy_offset = 0;
	gyroz_offset = 0;

	for(i = 0; i < OFFSET_SAMPLE; i++)
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

		gyrox_offset += (double) gyrox;
		gyroy_offset += (double) gyroy;
		gyroz_offset += (double) gyroz;
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

void display()
{
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	//gluLookAt(10.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f ,1.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f, 0.0f, -5.0f);
	glRotatef(gx, 1.0f, 0.0f, 0.0f);
	glRotatef(gy, 0.0f, 1.0f, 0.0f);

	//glRotatef((float) gx, 1.0f, 0.0f, 0.0f);
	//glRotatef((float) gy, 0.0f, 1.0f, 0.0f);
	glutSolidCube(2.0f);

	glFlush();
	//glutSwapBuffers();
}

void timer_int()
{		
	get_sensor_data();	

	ay = atan2(accx, sqrt( pow(accy, 2) + pow(accz, 2))) * 180 / M_PI;
	ax = atan2(accy, sqrt( pow(accx, 2) + pow(accz, 2))) * 180 / M_PI;

	gx = gx + gyrox / FREQ;
	gy = gy - gyroy / FREQ;

	// complementary filter
	// tau = DT*(A)/(1-A)
	// = 0.48sec
	gx = gx * 0.96 + ax * 0.04;
	gy = gy * 0.96 + ay * 0.04;

	glutTimerFunc(30, timer_int, 0);
	glutPostRedisplay();
}

void idle()
{
	glutPostRedisplay();
}

void reshape (int width, int height) 
{  
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity();
	gluPerspective(60, (GLfloat)width / (GLfloat)height, 1.0, 100.0); 
	glMatrixMode(GL_MODELVIEW);
} 

int main(int argc, char **argv)
{
	setup_bt();	

	get_offset();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_SINGLE);
	glutInitWindowSize(800,600);
	glutCreateWindow("P0lyDr0n3 Remote Debug");

	glutDisplayFunc(display);
	glutTimerFunc(30, timer_int, 0);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);

	glEnable(GL_DEPTH_TEST);

	glutMainLoop();
	
	end();
	return 0;
}

