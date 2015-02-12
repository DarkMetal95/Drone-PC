#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glu.h>

int s, status;
int accx, accy, accz, gyrox, gyroy, gyroz;
float i;

int sign(int value)
{
	if(value > 0x8000)
		value = -((65535 - value) + 1);
	return value;
}

void setup()
{
	glClearColor(255.0f, 255.0f, 255.0f, 255.0f);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50.0, 1.0, 1.0, 20.0);
	glMatrixMode(GL_MODELVIEW);
	gluLookAt(10.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f ,1.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	
	glTranslatef(0.0f, -2.0f, 0.0f);
	//glRotatef( 45, 0.0f, 1.0f, 0.0f);
	//glRotatef(accy, 0.0f, 1.0f, 0.0f);
	//glRotatef(accz, 0.0f, 0.0f, 1.0f);
	glutSolidCube(1.0f);

	glTranslatef(0.0f, 4.0f, 0.0f);
	//glRotatef(accx, 1.0f, 0.0f, 0.0f);
	//glRotatef(accy, 0.0f, 1.0f, 0.0f);
	//glRotatef(accz, 0.0f, 0.0f, 1.0f);
	glutSolidCube(1.0f);

	//glFlush();
	glutSwapBuffers();
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

void timer_int()
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

	gyrox = sign(gyrox);
	gyroy = sign(gyroy);
	gyroz = sign(gyroz);

	glutPostRedisplay();
}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	i = 0;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(800,600);
	glutCreateWindow("P0lyDr0n3 Remote Debug");
	glEnable(GL_DEPTH_TEST);

	const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
	const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat light_position[] = { 1.0f, 1.0f, 1.0f, 0.0f };

	const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
	const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
	const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat high_shininess[] = { 50.0f };

	glClearColor(1,1,1,1);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	//glEnable(GL_COLOR_MATERIAL);

	glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	//glutTimerFunc(50, timer_int, 0);

	setup();

	//setup_bt();	

	glutMainLoop();
	
	//end();
	return 0;
}

