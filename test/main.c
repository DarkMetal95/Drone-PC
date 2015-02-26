#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <ncurses.h>

int s, status;

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

int main(int argc, char **argv)
{
	char key[1] = {0x00};
	char speed_c[] = "PWM_SPEED";
	char phrase[10];

	initscr();	
	mvprintw(0,0,"Waiting for Bluetooth connection...");
	nodelay(stdscr, TRUE);
	noecho();
	setup_bt();
	refresh();
	mvprintw(0,0,"Connected...");

	while((*key != 'k') && (*key != 'l'))
	{
		*key = getch();
		if( *key >= 0)
		{
			refresh();
			mvprintw(0,0,"Sending characters %c", *key);
			write(s, key, 1);
			read(s, speed_c, 10);
			mvprintw(1,0,"Motors speed: %s", speed_c);
		}
	}

	end();
	endwin();
	return 0;
}
