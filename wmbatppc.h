/*
 * Revision 0.2  24 Nov 2000 Lou
 *
 * $Id: wmbatppc.h 32 2004-10-13 17:43:38Z julien $
 */

#ifndef __WMBATPPC_H__
#define __WMBATPPC_H__

#include <sys/socket.h>
#include <sys/un.h>

#ifdef ENABLE_XOSD
# include <xosd.h>
#endif /* ENABLE_XOSD */

/* Should be defined in <sys/un.h> ... */
#ifndef UNIX_PATH_MAX
# define UNIX_PATH_MAX 108
#endif

#define WMBATPPC_VERSION       "2.5"

#define LEFT_BATTERY           0
#define RIGHT_BATTERY          1

#define HIGH_BATT              0  // color green
#define LOW_BATT               1  // color red
#define MED_BATT               2  // color yellow

#define NO_BATT                3
#define IS_BATT                4

#define NOCHARGE               0
#define CHARGE                 1
#define UNPLUGGED              0
#define PLUGGED                1

#define LIMITED                0
#define INFINITE               1

#define SMALL                  0
#define BIG                    1

#define HOURS                  0
#define MINUTES                1

#define V                      0
#define MV                     1

#define COMMA                  0
#define PERCENT                1

#define WMAKER                 10
#define XIMIAN                 20

#define PMU_VERSION_KEYLARGO   12

#define PMU_CONFIG_KEYWORD     "pmu"
#define PMU_STYLE_NAME         "pmu"

#define PMU_PROC_INFO          "/proc/pmu/info"
#define PMU_PROC_BATTERY_BASE  "/proc/pmu/battery_"
#define PMU_PROC_PRESENT       (1 << 0)
#define PMU_PROC_CHARGING      (1 << 1)

#define PMUD_INPUT_DELIM       " \t\n{}" 

#define PMUD_HOST              "localhost"
#define PMUD_PORT              879
#define PMUD_SERVICE	       "pmud"
#define PMUD_SOCKET_PATH       "/etc/power/control"

/*
 * Uncomment the following line to wait for pmud to refresh battery
 * information, otherwise we update every 100 milliseconds
 */
//#define PMUD_REFRESH
#define UPDATE_RATE            100000

/*
 * Colors for TTY output
 */
#define BWHITE                 "\e[1;37m"
#define BRED                   "\e[1;31m"
#define BYELLOW                "\e[1;33m"
#define BGREEN                 "\e[1;32m"
#define BRESET                 "\e[0m"

typedef struct
{
  int available;
  int state;
  int current;
  int percentage;
  int charging;
  int voltage;
} battery;

typedef struct
{
  int pmud_domain;
  char socket_path[UNIX_PATH_MAX];
  int update_rate;
  void (*read_pmu) (void);
  int pmud_version;
  battery b[2];
  int time_left;
  int ac_connected;
  int show_charge_time;
  int current;
} pmud_info;

typedef struct
{
  int x;
  int y;
} coords;

typedef struct
{
  coords plug;
  coords charg;
  coords timeleft[2];
  coords voltage[2];
  coords batt[2];
  coords percent[2];
  int jukeBoxParts;
  coords jukeBox[2];
  coords jukeBoxSize[2];
  coords symbols[2];
#ifdef ENABLE_XOSD
  xosd *osd;
#endif /* ENABLE_XOSD */
} gui_info;

typedef struct
{
  coords offset;
  coords jukeBoxImg[12];
  coords plugImg[2];
  coords chargImg[2];
  coords battImg[8];
  coords plugSize;
  coords chargSize;
  coords battSize;
  coords voltIndSize;
  coords voltIndImg;
  coords timeleftIndSize;
  coords timeleftIndImg;
} gui_xpm;

typedef struct
{
  int type;
  int width;
  int height;
  char **xpm;
} wm_info;

typedef enum
{
  KL_IBOOK,
  KL_PISMO,
  KL_UNKNOWN
} mactype;

void usage(void);
void printversion(void);
void BlitString(char *name, int x, int y, int is_big);
void BlitNum(int num, int x, int y, int is_big, int two_digits);
void wmbatppc_routine(int argc, char **argv, wm_info *wm);


#endif /* !__WMBATPPC_H__ */

