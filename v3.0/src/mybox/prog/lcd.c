/*
 (C) Copyright 2006 Mohd Nawawi Mohamad Jamili, TraceNetwork Corporation Sdn. Bhd.
 $Id: lcd.c,v 1.00 2006/11/26 13:10 PM nawawi Exp $
*/

#include "libmybox.h"

#include <termios.h>

#define LCD_CMD                 254
#define LCD_CLEAR               1
#define LCD_HOME_CURSOR		2
#define LCD_BLANK_SCREEN	8
#define LCD_HIDE_CURSOR      	12
#define LCD_ENABLE_BLOCK_CURSOR	13
#define LCD_ENABLE_UNDER_CURSOR	14
#define LCD_CURSOR_LEFT		16
#define LCD_CURSOR_RIGHT	20
#define LCD_SCROLL_LEFT		24
#define LCD_SCROLL_RIGHT	28
/*
 * 64~127 character-generator address
 * 128~144 Line 1 address
 * 192~208 Line 2 address
 */

#define LCD_CELLHGT		8
#define LCD_CELLWID		5

#define LCD_UP_KEY 'A'
#define LCD_DOWN_KEY 'B'
#define LCD_LEFT_KEY 'D'
#define LCD_RIGHT_KEY 'H'
#define LCD_ENTER_KEY 'P'

static int fd;
int dostat=0;
char cpu_data[18];
char bandwidth_in[18];
char bandwidth_out[18];

void get_cpu_data(void) {
	char buf[250], dd[250];
	int nums, x=0;
	FILE *p;
	cpu_data[0]=0;
	float k;
	if((p=popen("/bin/procinfo 2>/dev/null","r"))!=NULL) {
		while(fgets(buf,sizeof(buf),p)!=NULL) {
			if(buf[0]!='\0') {
				if(strstr(buf,"idle")) {
					nums=split_array(buf, SINGLE_SPACE, &chargv);
					if(nums > 0) {
						for(x=0;x<nums;x++) {
							if(strstr(chargv[x],"%")) {
								k=atof(stripchar(chargv[x],'%'));
								snprintf(dd,sizeof(dd),"U:%05.2f  I:%05.2f",100 - k, k);
								strcat(cpu_data,dd);
							}
						}
					}
					break;
				}
			}
		}
		buf[0]=0;
		pclose(p);
        }
}

int LCD_init() {
	struct termios portset;
	char device[256] = "/dev/lcd";
	int speed=B2400;
	fd = open (device, O_RDWR|O_NOCTTY|O_NDELAY);
	if(fd == -1){
		return -1;
	}
	tcgetattr(fd, &portset);
	cfmakeraw(&portset);
	cfsetospeed(&portset, speed);
	cfsetispeed(&portset, B0);
	tcsetattr(fd, TCSANOW, &portset);
	return fd;

}

void LCD_close() {
    close (fd);
}

void LCD_hide_cursor(){	
	char out[4];	
	snprintf(out, sizeof(out), "%c%c",LCD_CMD,LCD_HIDE_CURSOR);	
	write(fd,out,2);	
	return;
}

void LCD_show_cursor(int type) {
	char out[4];
	if(type==1){
		snprintf(out, sizeof(out), "%c%c",LCD_CMD,LCD_ENABLE_BLOCK_CURSOR);
	} else {
		snprintf(out, sizeof(out), "%c%c",LCD_CMD,LCD_ENABLE_UNDER_CURSOR);
	}
	write(fd,out,2);
	return;
}

void LCD_clear() {
	char out[4];
	snprintf(out, sizeof(out), "%c%c",LCD_CMD,LCD_CLEAR);
	write(fd,out,2);
	LCD_hide_cursor();
	return;
}


void LCD_string (int x, int y, char string[]) {
	int i;
	char c;
	char out[16];
	
	snprintf (out, sizeof(out), "%c%c", LCD_CMD,128+(64*(y))+x);
	write (fd, out, 2);
	for (i = 0; string[i]; i++) {
		c = string[i];
		switch(c) {
			case '\254': c='#'; break;
		}
		// write out string
		snprintf(out, sizeof(out), "%c",c);
		write(fd,out,1);
	}
		
}

void show_logo(void) {
	char buf[18], v[18], m[18];
	FILE *f;
	if(file_exists("/etc/version")) {
		if((f=fopen("/etc/version","r"))!=NULL) {
			fgets(buf,sizeof(buf),f);
			fclose(f);
        	}
	}
	trim(buf);
	snprintf(v,sizeof(v),"   %s",buf);
	buf[0]=0;
	if(file_exists("/etc/version_core")) {
		if((f=fopen("/etc/version_core","r"))!=NULL) {
			fgets(buf,sizeof(buf),f);
			fclose(f);
        	}
	}
	trim(buf);
	snprintf(m,sizeof(m)," MYBOX VER %s",buf);
	LCD_clear();
	LCD_string(0,0,m);
	LCD_string(0,1,v);
}

void show_logod(void) {
	show_logo();
	dostat=1;
	while(dostat) {switch_tab();usleep(500000);}
}

char *convert_double(double *value, int base, double roundlimit) {
        static char *units[] = { "", "k", "M", "G", "T", "P", "E", "Z", "Y", NULL };
        int off = 0;
        if((roundlimit <= 0.0) || (roundlimit > 1.0)) roundlimit = 0.5;
        while(units[off] != NULL) {
                if(*value < 1000 * roundlimit) break;
                off++;
                *value /= base;
        }
        return units[off];
}

void format_value(char *buff, double value, char *unit) {
        /* Convert bytes to bits, if necessary */
        if(strstr(unit, "b")) value *= 8;

        /* If units are bytes, then divide by 2^10, otherwise by 10^3 */
        char *mag = convert_double(&value, (strstr(unit, "B")) ? 1024 : 1000, 1.0f);
        if(mag[0] == 0) {
                //sprintf(buff, "%5.2lf %s", (long) value, unit);
                sprintf(buff, "%5.2lf%s", (long) value, unit);
        } else {
                //sprintf(buff, "%5.2lf %s%s", value, mag, unit);
		sprintf(buff, "%5.2lf%s", value, mag);
	}
}

int get_bandwidth(void) {
        FILE *file, *f;
        char buffer[1024];
        char *ch_pointer = NULL;
        char name[8]="eth0";
        double rx_byte;
        double tx_byte;
        double rx_pkt;
        double tx_pkt;

	bandwidth_in[0]=0;
	bandwidth_out[0]=0;
	if(file_exists("/var/sys/lcdd_eth")) {
		if((f=fopen("/var/sys/lcdd_eth","r"))!=NULL) {
			fgets(name,sizeof(name),f);
			fclose(f);
        	}
		if(name[0]==0) snprintf(name,sizeof(name),"eth0");
	}
        if((file = fopen("/proc/net/dev", "r")) != NULL) {
                /* Skip first 2 header lines of file */
                fgets(buffer, sizeof(buffer), file);
                fgets(buffer, sizeof(buffer), file);

                /* Search iface_name and scan values */
                while((fgets(buffer, sizeof(buffer), file) != NULL)) {
                        if (strstr(buffer,name)) {
                                /* search ':' and skip over it */
                                ch_pointer = strchr(buffer, ':');
                                ch_pointer++;

                                /* Now ch_pointer points to values of iface_name */
                                /* Scan values from here */
                                sscanf(ch_pointer, "%lf %lf %*s %*s %*s %*s %*s %*s %lf %lf",
                                        &rx_byte,
                                        &rx_pkt,
                                        &tx_byte,
                                        &tx_pkt);
                        }
                }
                format_value(bandwidth_in,rx_byte,"b");
                format_value(bandwidth_out,tx_byte,"b");
                fclose(file);
		return 0;
        }
	snprintf(bandwidth_in,sizeof(bandwidth_in),"00.00b");
	snprintf(bandwidth_out,sizeof(bandwidth_out),"00.00b");
	return 1;
}

void show_bandwidth(void) {
	char str[18];
	signal(SIGALRM, show_logo);
	alarm(3600);
	dostat=1;
	LCD_clear();
	LCD_string(0,0,"  WAN TRAFFIC   ");
	while(dostat) {
		switch_tab();
		get_bandwidth();
		trim(bandwidth_in);
		trim(bandwidth_out);
		snprintf(str,sizeof(str),"%s/%s",bandwidth_in,bandwidth_out);
		LCD_string(0,1,"                \0");
		LCD_string(0,1,str);
		usleep(500000);
	}	
}

void show_cpu(void) {
	char str[18];
	signal(SIGALRM, show_logo);
	alarm(3600);
	LCD_clear();
	LCD_string(0,0,"  CPU INFO (%)  \0");
	dostat=1;
	while(dostat) {
		switch_tab();
		get_cpu_data();
		snprintf(str,sizeof(str),"%s",cpu_data);
		LCD_string(0,1,"                \0");
		LCD_string(0,1,str);
		usleep(500000);
	}
}

void show_uptime(void) {
	char str[18];
	signal(SIGALRM, show_logo);
	alarm(3600);
	LCD_clear();
	LCD_string(0,0,"     UPTIME     \0");
	dostat=1;
	while(dostat) {
		switch_tab();
		const long minute = 60;
		const long hour = minute * 60;
		const long day = hour * 24;
		struct sysinfo si;
		sysinfo(&si);
		snprintf(str,sizeof(str)," %03ldd, %02ld:%02ld:%02ld", si.uptime / day, (si.uptime % day) / hour, (si.uptime % hour) / minute, si.uptime % minute);
		LCD_string(0,1,"                \0");
		LCD_string(0,1,str);
		usleep(500000);
	}
}

void show_mem(void) {
	char str[18];
	signal(SIGALRM, show_logo);
	alarm(3600);
	LCD_clear();
	LCD_string(0,0,"  MEMORY (MB)   \0");
	dostat=1;
	while(dostat) {
		switch_tab();
		const double megabyte = 1024 * 1024;
		struct sysinfo si;
		sysinfo(&si);
		snprintf(str,sizeof(str)," %5.1f / %5.1f ",si.totalram / megabyte,si.freeram / megabyte);
		LCD_string(0,1,"                \0");
		LCD_string(0,1,str);
		usleep(500000);
	}
}

void show_msg(char *title, char *str) {
	LCD_clear();
	LCD_string(0,0,title);
	LCD_show_cursor(2);
	LCD_string(0,1,str);
}

void switch_tab(void) {
	unsigned char b;
	read(fd,&b,1);
	if(b==(unsigned char)253){
		dostat=0;
		read(fd,&b,1);
		if(b & 0x1) { 
			// Key UP
			show_bandwidth();
		}
		if(b & 0x2) { 
			// Key DOWN
			show_cpu();
		} 
		if(b & 0x4) { 
			// Key LEFT
			show_mem();			
		} 
		if(b & 0x8) { 
			// Key RIGHT
			show_uptime();
		} 
		if(b & 0x10) {
			// Key ENTER
			show_logod();
		}
	}
}

int boot_lcd_msg(char *name, char *val) {
	char cmd1[17], cmd2[17];
	fd = LCD_init();
	if(fd < 0 ) return 1;
	snprintf(cmd1,sizeof(cmd1),"%s",name);
	snprintf(cmd2,sizeof(cmd2),"%s",val);
	if(!strcmp(cmd1,"show") && !strcmp(cmd2,"logo")) {
		show_logo();
	} else {
		show_msg(cmd1,cmd2);
	}
	LCD_close();
	return 0;
}

int lcd_msg(int prog, char *name, char *val) {
	if(prog==0) {
		if(file_exists("/var/sys/lcd_dev")) {
			boot_lcd_msg(name,val);
			return 0;
		}
	} else if(prog==1) {
		save_to_file("/var/sys/lcd_msg","%s\n",val);
		return 0;
	}
	return 1;
}

int lcdd_main(int argc, char **argv) {
	char cmd1[17], cmd2[17];
	//freopen("/dev/null", "w", stderr);
	if(argv[1]!=NULL && argv[2]!=NULL) {
		fd = LCD_init();
		if(fd < 0 ) exit(-1);
		snprintf(cmd1,sizeof(cmd1),"%s",argv[1]);
		snprintf(cmd2,sizeof(cmd2),"%s",argv[2]);
		if(!strcmp(cmd1,"show") && !strcmp(cmd2,"logo")) {
			show_logo();
		} else {
			show_msg(cmd1,cmd2);
		}
		LCD_close();
		exit(0);
	}
	if(daemon(1,1)==0) {
		fd = LCD_init();
		if(fd < 0 ) exit(-1);

		LCD_clear();
		show_logo();   
		dostat=0;
		while(1){
			switch_tab();
			usleep(500000);
		}
		LCD_close();
		exit(0);
	}
	exit(0);
}
