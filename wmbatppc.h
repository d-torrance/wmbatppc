/*
 * Revision 0.2  24 Nov 2000 Lou
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>

#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netdb.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#define WMBATPPC_VERSION "0.1"

#define LEFT_BATTERY 0
#define RIGHT_BATTERY 1

#define HIGH_BATT 0	// color green
#define LOW_BATT 1  // color red
#define MED_BATT 2  // color yellow

#define NO_BATT 3
#define IS_BATT 4

#define NOCHARGE 0
#define CHARGE 1
#define UNPLUGGED 0
#define PLUGGED 1

#define LIMITED 0
#define INFINITE 1

#define SMALL 0
#define BIG 1

#define HOURS 0
#define MINUTES 1

#define V 0
#define MV 1

#define COMMA 0
#define PERCENT 1

#define WMAKER 10
#define XIMIAN 20

#define PMU_VERSION_KEYLARGO	12

#define PMU_CONFIG_KEYWORD "pmu"
#define PMU_STYLE_NAME "pmu"

#define PMUD_INPUT_DELIM " \t\n{}" 

#define PMUD_HOST "localhost"
#define PMUD_PORT 879
#define PMUD_SERVICE		"pmud"

#define PMUD_REFRESH


typedef struct {
	int available;
     	int current;
     	int percentage;
     	int charging;
     	int voltage;
} battery ;

typedef struct {
	int pmud_version ;
	battery b [2] ;
	int batt_state [2] ;
	int time_left ;
	int ac_connected ;
	int show_charge_time ;
	int current ;
} pmud_info ;

typedef struct {
	int x ;
	int y ;
} coords ;

typedef struct {
	coords plug ;
	coords charg ;
	coords timeleft[2] ;
	coords voltage[2] ;
	coords batt[2] ;
	coords percent[2] ;
	coords jukeBox[2] ;
	coords symbols[2] ;
} interface ;

void usage(void);
void printversion(void);
void BlitString(char *name, int x, int y, int is_big);
void BlitNum(int num, int x, int y, int is_big, int two_digits);
void wmbatppc_routine(int, char **);


