/*
 * $Id: wmbatppc.c 32 2004-10-13 17:43:38Z julien $
 *
 * wmbatppc, originally written by Lou <titelou@free.fr>
 *
 * ChangeLog :
 *
 * * 20021016 v1.2 Julien BLACHE <jb@jblache.org>
 *    + do not segfault when the pmud stops or crashes
 *    + added unix socket support
 *    + added /proc/pmu support
 * * 20021105 v1.3 Julien BLACHE <jb@jblache.org>
 *    + added xosd support
 * * 20021127 v2.0 Julien BLACHE <jb@jblache.org>
 *    + added tty output support
 * * 20030122 v2.1 Julien BLACHE <jb@jblache.org>
 *    + display OSD in case curtime - last_osd < 0
 * * 20030410 v2.2 Julien BLACHE <jb@jblache.org>
 *    + /proc/pmu parsing rewritten
 * * 20030723 v2.3 Julien BLACHE <jb@jblache.org>
 *    + /proc/pmu/battery_X has changed the meaning of its flags: member,
 *      now the second byte indicates the battery type.
 *    + when parsing /proc, we get the time in seconds, we want minutes !
 *    + removed SIGINT/SIGTERM handler. (caused wmbatppc to hang when receiving
 *      these signals and built with xosd ; works fine without the handlers too)
 * * 20040211 v2.4 Julien BLACHE <jb@jblache.org>
 *    + patch from Sebastian Henschel to fix a segfault when using OSD and a
 *      locale not supported by XLib (can't find an appropriate font).
 *    + you can now specify the path to the UNIX domain socket with the -u switch.
 *    + some code cleanup at last, eliminating most of the global variables.
 *    + changed the way OSD display works.
 * * 20041013 v2.5 Julien BLACHE <jb@jblache.org>
 *    + fixed compilation without ENABLE_XOSD
 *    + added runtime switch to disable OSD
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

#include <netinet/in.h> /* INET sockets */
#include <sys/un.h> /* UNIX domain sockets */
#include <netdb.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#ifdef ENABLE_XOSD
# include <xosd.h>
#endif /* ENABLE_XOSD */


#include "wmbatppc.h"
#include "wmgeneral.h"
#include "wmbatppc-master.xpm"
#include "xgbatppc-master.xpm"

pmud_info sys_pmu;

gui_info gui;
gui_xpm xpm;

void showGUIelement (coords img, coords size, coords dest)
{
  copyXPMArea(img.x, img.y, size.x, size.y, dest.x, dest.y);
}

void updatePlug (int state)
{
  showGUIelement(xpm.plugImg[state], xpm.plugSize, gui.plug);
}

void updateCharg (int state)
{
  showGUIelement(xpm.chargImg[state], xpm.chargSize, gui.charg);
}

void drawBatteryState (int bat_side, int bat_state)
{
  showGUIelement(xpm.battImg[bat_side*4+bat_state], xpm.battSize, gui.batt[bat_side]);
}

void initXIMGUIelements (void)
{
  int i = 0;

  gui.voltage[V].x = 20;
  gui.voltage[V].y = 6;
  gui.voltage[MV].x = 34;
  gui.voltage[MV].y = 6;
  gui.symbols[COMMA].x = 32;
  gui.symbols[COMMA].y = 11;
  gui.percent[LEFT_BATTERY].x = 65;
  gui.percent[LEFT_BATTERY].y = 6;
  gui.percent[RIGHT_BATTERY].x = 91;
  gui.percent[RIGHT_BATTERY].y = 6;
  gui.timeleft[HOURS].x = 119;
  gui.timeleft[HOURS].y = 6;
  gui.timeleft[MINUTES].x = 137;
  gui.timeleft[MINUTES].y = 6;
  gui.jukeBox[0].x = 23;
  gui.jukeBox[0].y = 1;
  gui.jukeBox[1].x = 122;
  gui.jukeBox[1].y = 1;
  gui.batt[LEFT_BATTERY].x = 60;
  gui.batt[LEFT_BATTERY].y = 4;
  gui.batt[RIGHT_BATTERY].x = 87;
  gui.batt[RIGHT_BATTERY].y = 4;
  gui.plug.x = 3;
  gui.plug.y = 2;
  gui.charg.x = 158;
  gui.charg.y = 2;
  gui.jukeBoxParts = 2;
  gui.jukeBoxSize[0].x = 28;
  gui.jukeBoxSize[0].y = 4;
  gui.jukeBoxSize[1].x = 28;
  gui.jukeBoxSize[1].y = 4;
  
  for (i=0; i < 10; i++)
    xpm.jukeBoxImg[i].x += 14;
}

void initWMGUIelements (void)
{
  gui.voltage[V].x = 15;
  gui.voltage[V].y = 2;
  gui.voltage[MV].x = 29;
  gui.voltage[MV].y = 2;
  gui.symbols[COMMA].x = 27;
  gui.symbols[COMMA].y = 7;
  gui.percent[LEFT_BATTERY].x = 11;
  gui.percent[LEFT_BATTERY].y = 49;
  gui.percent[RIGHT_BATTERY].x = 36;
  gui.percent[RIGHT_BATTERY].y = 49;
  gui.timeleft[HOURS].x = 15;
  gui.timeleft[HOURS].y = 31;
  gui.timeleft[MINUTES].x = 33;
  gui.timeleft[MINUTES].y = 31;
  gui.jukeBox[0].x = 4;
  gui.jukeBox[0].y = 13;
  gui.jukeBox[1].x = 0;
  gui.jukeBox[1].y = 0;
  gui.batt[LEFT_BATTERY].x = 6;
  gui.batt[LEFT_BATTERY].y = 47;
  gui.batt[RIGHT_BATTERY].x = 33;
  gui.batt[RIGHT_BATTERY].y = 47;
  gui.plug.x = 2;
  gui.plug.y = 2;
  gui.charg.x = 50;
  gui.charg.y = 2;
  gui.jukeBoxParts = 1;
  gui.jukeBoxSize[0].x = 56;
  gui.jukeBoxSize[0].y = 30;
  gui.jukeBoxSize[1].x = 0;
  gui.jukeBoxSize[1].y = 0;
}

void initXPMelements (int xoff, int yoff)
{
  xpm.offset.x = xoff;
  xpm.offset.y = yoff;

  xpm.timeleftIndSize.x = 36 ;
  xpm.timeleftIndSize.y = 13 ;
  xpm.timeleftIndImg.x = 15;
  xpm.timeleftIndImg.y = 29 + xpm.offset.y;
  xpm.voltIndSize.x = 34;
  xpm.voltIndSize.y = 10;
  xpm.voltIndImg.x = 2;
  xpm.voltIndImg.y = 84 + xpm.offset.y; 
  xpm.battImg[LEFT_BATTERY*4+HIGH_BATT].x = 146;
  xpm.battImg[LEFT_BATTERY*4+HIGH_BATT].y = 6 + xpm.offset.y;
  xpm.battImg[LEFT_BATTERY*4+LOW_BATT].x = 173;
  xpm.battImg[LEFT_BATTERY*4+LOW_BATT].y = 6 + xpm.offset.y;
  xpm.battImg[LEFT_BATTERY*4+MED_BATT].x = 200;
  xpm.battImg[LEFT_BATTERY*4+MED_BATT].y = 6 + xpm.offset.y;
  xpm.battImg[LEFT_BATTERY*4+NO_BATT].x = 108;
  xpm.battImg[LEFT_BATTERY*4+NO_BATT].y = 85 + xpm.offset.y;
  xpm.battImg[RIGHT_BATTERY*4+HIGH_BATT].x = 146;
  xpm.battImg[RIGHT_BATTERY*4+HIGH_BATT].y = 21 + xpm.offset.y;
  xpm.battImg[RIGHT_BATTERY*4+LOW_BATT].x = 173;
  xpm.battImg[RIGHT_BATTERY*4+LOW_BATT].y = 21 + xpm.offset.y;
  xpm.battImg[RIGHT_BATTERY*4+MED_BATT].x = 200;
  xpm.battImg[RIGHT_BATTERY*4+MED_BATT].y = 21 + xpm.offset.y;
  xpm.battImg[RIGHT_BATTERY*4+NO_BATT].x = 135;
  xpm.battImg[RIGHT_BATTERY*4+NO_BATT].y = 85 + xpm.offset.y;
  xpm.battSize.x = 25;
  xpm.battSize.y = 13;
  xpm.plugSize.x = 12;
  xpm.plugSize.y = 16;
  xpm.chargSize.x = 12;
  xpm.chargSize.y = 16;
  xpm.plugImg[0].x = 158;
  xpm.plugImg[0].y = 53 + xpm.offset.y;
  xpm.plugImg[1].x = 158;
  xpm.plugImg[1].y = 36 + xpm.offset.y;
  xpm.chargImg[0].x = 134;
  xpm.chargImg[0].y = 53 + xpm.offset.y;
  xpm.chargImg[1].x = 134;
  xpm.chargImg[1].y = 36 + xpm.offset.y;
  xpm.jukeBoxImg[0].x = 173; 
  xpm.jukeBoxImg[0].y = 68 + xpm.offset.y;
  xpm.jukeBoxImg[1].x = 5;
  xpm.jukeBoxImg[1].y = 101 + xpm.offset.y;
  xpm.jukeBoxImg[2].x = 126;
  xpm.jukeBoxImg[2].y = 135 + xpm.offset.y;
  xpm.jukeBoxImg[3].x = 4;
  xpm.jukeBoxImg[3].y = 168 + xpm.offset.y;
  xpm.jukeBoxImg[4].x = 127;
  xpm.jukeBoxImg[4].y = 168 + xpm.offset.y;
  xpm.jukeBoxImg[5].x = 65;
  xpm.jukeBoxImg[5].y = 168 + xpm.offset.y;
  xpm.jukeBoxImg[6].x = 127;
  xpm.jukeBoxImg[6].y = 101 + xpm.offset.y;
  xpm.jukeBoxImg[7].x = 4;
  xpm.jukeBoxImg[7].y = 135 + xpm.offset.y;
  xpm.jukeBoxImg[8].x = 65;
  xpm.jukeBoxImg[8].y = 135 + xpm.offset.y;
  xpm.jukeBoxImg[9].x = 66;
  xpm.jukeBoxImg[9].y = 101 + xpm.offset.y;
  xpm.jukeBoxImg[10].x = 173;
  xpm.jukeBoxImg[10].y = 36 + xpm.offset.y;
}

void initGUIelements (wm_info *wm)
{
  if (wm->type == WMAKER)
    {
      initXPMelements(0, 0);
      initWMGUIelements();
    }
  else
    {
      initXPMelements(0, 30);
      initXIMGUIelements();
    }
}

void drawJukeBox (int state)
{
  showGUIelement (xpm.jukeBoxImg[state], gui.jukeBoxSize[0], gui.jukeBox[0]) ;
  if (gui.jukeBoxParts > 1)
    showGUIelement (xpm.jukeBoxImg[state], gui.jukeBoxSize[1], gui.jukeBox[1]) ;
}

int open_pmud_socket (void)
{
  int ret;
  int sa_size;

  struct sockaddr *sa;
  
  struct sockaddr_in sin;
  struct sockaddr_un sun;
  
  if (sys_pmu.pmud_domain == PF_INET)
    {
      /* INET part */
      sin.sin_family = PF_INET;
      sin.sin_port = htons (PMUD_PORT);
      sin.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
      
      sa = (struct sockaddr *) &sin;
      sa_size = sizeof (struct sockaddr_in);
    }
  else
    {
      /* UNIX part */
      sun.sun_family = PF_UNIX;
      strcpy (sun.sun_path, sys_pmu.socket_path);
      
      sa = (struct sockaddr *) &sun;
      sa_size = sizeof (struct sockaddr_un);
    }
  
  if ((ret = socket (sys_pmu.pmud_domain, SOCK_STREAM, 0)) >= 0)
    {
      if (connect (ret, sa, sa_size) >= 0)
	return ret;
      else
	close (ret);
    }

  return -1;
}


static mactype keylargo_identify (void)
{
  FILE *fd;
  char buf[25];
  if (!(fd = fopen ("/proc/device-tree/pci/mac-io/media-bay/compatible", "ro")))
    return KL_IBOOK;
  /* 
   * no media-bay.  definately an iBook...
   * or a Pismo running a kernel without OF-devtree support...
   */
  fscanf (fd, "%s", buf);
  fclose (fd);
  if (!strcmp ("keylargo-media-bay", buf))	// only the pismo should have one
    return KL_PISMO;	// has one, its a pismo
  else
    return KL_UNKNOWN;
  /*
   * has a media bay, IDs as a Keylargo, but not a
   * "keylargo-media-bay"... what the...
   */
}

void read_pmu_proc (void)
{
  static int info_fd = -1;
  static int batt_fd[2] = { -1, -1 };
  int nb_batt, flags;
  int charge, maxcharge;
  int time_left[2];

  int i;

  char buf[128];
  char *s;

  if (info_fd < 0)
    {
      info_fd = open(PMU_PROC_INFO, O_RDONLY);
      
      if (info_fd < 0)
	{
	  sys_pmu.time_left = -2;
	  return;
	}
    }
  
  if (read(info_fd, buf, 127) < 0)
    {
      sys_pmu.time_left = -2;
      return;
    }

  /* Skip PMU driver version */
  s = strchr(buf, '\n');

  /* Skip PMU firmware version */
  s = strchr(s + 1, '\n');

  /* Get the AC status */
  s = strchr(s + 1, ':');
  sys_pmu.ac_connected = atoi(s + 2);
  
  /* Get the number of batteries */
  s = strchr(s + 1, ':');
  nb_batt = atoi(s + 2);

  close(info_fd);
  info_fd = -1;

  sys_pmu.b[LEFT_BATTERY].state = NO_BATT;
  sys_pmu.b[RIGHT_BATTERY].state = NO_BATT;
  sys_pmu.time_left = 0;
  time_left[0] = 0;
  time_left[1] = 0;

  for (i = 0; i < 2; i++)
    {
      if (batt_fd[i] < 0)
	{
	  sprintf(buf, PMU_PROC_BATTERY_BASE "%d", i);

	  if ((batt_fd[i] = open(buf, O_RDONLY)) < 0)
	    {
	      sys_pmu.b[i].available = NO_BATT;
	    }
	}

      if (read(batt_fd[i], buf, 127) < 0)
	{
	  sys_pmu.b[i].available = NO_BATT;
	  flags = 0;
	}
      else
	{
	  /* Get the flags */
	  s = strchr(buf, ':');
	  /* 
	   * We want only the 1st digit (byte, actually) 
	   * other bytes are not relevant yet (2nd byte indicates the battery type)
	   */
	  flags = atoi(s + 2 + 7);
	}

      if ((flags & PMU_PROC_PRESENT) && (i < nb_batt))
	{
	  sys_pmu.b[i].available = IS_BATT;
	  sys_pmu.b[i].charging = flags & PMU_PROC_CHARGING;
	  /* Get the charge */
	  s = strchr(s + 1, ':');
	  charge = atoi(s + 2);
	  /* Get the max_charge */
	  s = strchr(s + 1, ':');
	  maxcharge = atoi(s + 2);
	  /* Get the current */
	  s = strchr(s + 1, ':');
	  sys_pmu.b[i].current = atoi(s + 2);
	  /* Get the voltage */
	  s = strchr(s + 1, ':');
	  sys_pmu.b[i].voltage = atoi(s + 2);
	  /* Get the time remaining */
	  s = strchr(s + 1, ':');
	  time_left[i] = atoi(s + 2) / 60; /* We get the time in seconds, we want minutes */
	  sys_pmu.b[i].percentage = charge * 100 / maxcharge;
	  sys_pmu.b[i].state = IS_BATT;
	}
      else
	{
	  sys_pmu.b[i].available = NO_BATT;
	  sys_pmu.b[i].percentage = 0;
	  sys_pmu.b[i].current = 0;
	  sys_pmu.b[i].voltage = 0.0;
	  sys_pmu.b[i].charging = 0;
	}
      
      close(batt_fd[i]);
      batt_fd[i] = -1;
    }

  sys_pmu.current = sys_pmu.b[LEFT_BATTERY].current + sys_pmu.b[RIGHT_BATTERY].current;

  if (sys_pmu.b[0].charging && sys_pmu.b[1].charging)
    sys_pmu.time_left = (time_left[0] > time_left[1]) ? time_left[0] : time_left[1];
  else if (sys_pmu.b[0].charging)
    sys_pmu.time_left = time_left[0];
  else if (sys_pmu.b[1].charging)
    sys_pmu.time_left = time_left[1];
  else
    sys_pmu.time_left = time_left[0] + time_left[1];
}

void read_pmu_socket (void)
{
  /* smart battery systems are the same */
  unsigned char par;
  int current, charge, maxcharge, ac_hold = -1;
  char *b;
  char c[150];
  static int sock = -1;
  int i;
#ifdef PMUD_REFRESH
  int ret;
#endif
  
  if (sock < 0)
    if ((sock = open_pmud_socket ()) < 0)
      {
	sys_pmu.time_left = -2;
	return;
      }

#ifdef PMUD_REFRESH
  ret = read (sock, c, 150);
  shutdown (sock, SHUT_RDWR);
  close (sock);
  sock = -1;
  
  if (ret <= 0)
    {
      sys_pmu.time_left = 0;

      sys_pmu.ac_connected = 0;
      sys_pmu.show_charge_time = 0;
      sys_pmu.current = 0;

      for (i = 0; i < 2; i++)
	{
	  sys_pmu.b[i].available = IS_BATT;
	  
	  sys_pmu.b[i].current = 0;
	  sys_pmu.b[i].percentage = 0;
	  sys_pmu.b[i].charging = 0;
	  sys_pmu.b[i].voltage = 0;
	  
	  sys_pmu.b[i].state = NO_BATT;
	}

      return;
    }
#else /* !PMUD_REFRESH */
  if ((write(sock, "\n", 1) <= 0) || (read (sock, c, 150) <= 0))
    {
      shutdown (sock, SHUT_RDWR); 
      close (sock); 
      sock = -1; 
      
      sys_pmu.time_left = 0;

      sys_pmu.ac_connected = 0;
      sys_pmu.show_charge_time = 0;
      sys_pmu.current = 0;

      for (i = 0; i < 2; i++)
	{
	  sys_pmu.b[i].available = IS_BATT;
	  
	  sys_pmu.b[i].current = 0;
	  sys_pmu.b[i].percentage = 0;
	  sys_pmu.b[i].charging = 0;
	  sys_pmu.b[i].voltage = 0;
	  
	  sys_pmu.b[i].state = NO_BATT;
	}

      return;
    }
#endif /* PMUD_REFRESH */
      
  b = strtok (c, PMUD_INPUT_DELIM);
  if (b[0] != 'S')
    return;
  
  sys_pmu.b[LEFT_BATTERY].state = NO_BATT;
  sys_pmu.b[RIGHT_BATTERY].state = NO_BATT;
  
  sys_pmu.time_left = 0;
  for (par = 0; par < 2; par++)
    {
      ac_hold = atoi (strtok (NULL, PMUD_INPUT_DELIM));
      if (ac_hold % 10)
	{
	  sys_pmu.b[par].available = IS_BATT;
	  sys_pmu.b[par].charging = (((ac_hold / 10) % 10) == 1);
	  charge = atoi (strtok (NULL, PMUD_INPUT_DELIM));
	  maxcharge = atoi (strtok (NULL, PMUD_INPUT_DELIM)) - 2 ;
	  sys_pmu.b[par].current = atoi (strtok (NULL, PMUD_INPUT_DELIM));
	  sys_pmu.b[par].voltage = atoi (strtok (NULL, PMUD_INPUT_DELIM));
	  sys_pmu.b[par].percentage = charge * 100 / maxcharge;
	  sys_pmu.time_left += ((ac_hold / 100) ? maxcharge - charge : charge);
	  sys_pmu.b[par].state = IS_BATT;
	}
      else
	{
	  sys_pmu.b[par].available = NO_BATT;
	  sys_pmu.b[par].percentage = 0;
	  sys_pmu.b[par].current = 0;
	  sys_pmu.b[par].voltage = 0;
	  sys_pmu.b[par].charging = 0;
	}
    }
  
  sys_pmu.ac_connected = ac_hold / 100;
  current = sys_pmu.b[LEFT_BATTERY].current + sys_pmu.b[RIGHT_BATTERY].current;
  if (current < 0)
    sys_pmu.time_left = sys_pmu.time_left * 3552 / (current * -60);
  else if (sys_pmu.show_charge_time && (current > 0))
    sys_pmu.time_left = sys_pmu.time_left * 3552 / (current * 60);
  else
    sys_pmu.time_left = -1;
  
  sys_pmu.current = current ;
}


void DisplayBat(void)
{	
  int keylargo;
  unsigned int par = 0;
  int hour = 0;
  int min = 0;
  int volts = 0;
  int millivolts = 0;
  int i;
  int state;
  
  sys_pmu.show_charge_time = 1;
  
  keylargo = keylargo_identify(); /* TODO : use this detection */

  sys_pmu.read_pmu();

  if (sys_pmu.time_left == -1)
    drawJukeBox(0);
  else
    {
      state = (sys_pmu.current + 200) / 200 ;
      if (state < 0)
	state = state * -1 ;
      if (state > 9) 
	state = 9 ;
      drawJukeBox(state);
      if (sys_pmu.time_left == -2)
	{
	  hour = 0;
	  min = 0;
	}
      else
	{
	  hour = sys_pmu.time_left / 60;
	  min = sys_pmu.time_left % 60;
	}
      //printf("%i,%d,%d\n",sys_pmu.time_left,hour,min);
      BlitNum(hour, gui.timeleft[HOURS].x, gui.timeleft[HOURS].y, BIG, 1);  
      BlitNum(min, gui.timeleft[MINUTES].x, gui.timeleft[MINUTES].y, BIG, 1);  
    }
  
  if (sys_pmu.ac_connected)
    updatePlug(PLUGGED);
  else
    updatePlug(UNPLUGGED);
  
  if (sys_pmu.b[LEFT_BATTERY].charging || sys_pmu.b[RIGHT_BATTERY].charging)
    updateCharg(CHARGE);
  else
    updateCharg(NOCHARGE);
  
  for (par = 0; par < 2; par++)
    {
      if (sys_pmu.b[par].available == NO_BATT)
	drawBatteryState(par, NO_BATT);
      else
	{
	  if (sys_pmu.b[par].percentage > 70)
	    drawBatteryState(par, HIGH_BATT);
	  else if (sys_pmu.b[par].percentage > 25)
	    drawBatteryState(par, MED_BATT);
	  else if ((sys_pmu.b[par].percentage > 5) || (sys_pmu.b[par].charging) || (sys_pmu.b[1-par].percentage > 5))
	    drawBatteryState(par, LOW_BATT);
	  else 
	    {
	      /*
	       * This is were we blink when the connection to pmud is lost.
	       * So, we must have :
	       *   b[].percentage < 5 (0 will do)
	       *   b[].available != NO_BATT (0 will do too)
	       */
	      for (i = 0; i < (sys_pmu.update_rate / 50000); i++)
		{
		  drawBatteryState(par, NO_BATT);
		  RedrawWindow();
		  usleep(200000);
		  drawBatteryState(par, LOW_BATT);
		  RedrawWindow();
		}
	    }
	  
	  if (sys_pmu.b[par].percentage == 100)
	    BlitNum(sys_pmu.b[par].percentage, (gui.percent[par].x - 6), gui.percent[par].y, SMALL, 2);
	  else
	    BlitNum(sys_pmu.b[par].percentage, gui.percent[par].x, gui.percent[par].y, SMALL, 1);
	
	  volts = sys_pmu.b[par].voltage / 1000;
	  millivolts =  sys_pmu.b[par].voltage % 1000;
	  BlitNum(volts, gui.voltage[V].x, gui.voltage[V].y, SMALL, 1);
	  BlitNum(millivolts, gui.voltage[MV].x, gui.voltage[MV].y, SMALL, 0);
	  copyXPMArea(74, (69 + xpm.offset.y), 2, 3, gui.symbols[COMMA].x, gui.symbols[COMMA].y);
	}
    }

  if ((sys_pmu.b[LEFT_BATTERY].available == NO_BATT) &&
      (sys_pmu.b[RIGHT_BATTERY].available == NO_BATT))
    copyXPMArea(xpm.voltIndImg.x, xpm.voltIndImg.y, xpm.voltIndSize.x, xpm.voltIndSize.y, gui.voltage[V].x, gui.voltage[V].y);
}

#ifdef ENABLE_XOSD

time_t DisplayOSD (void)
{
  int osd_freq;
  int ref_bat; /* reference battery, ie the weakest one */
  char buf[128];

  if ((sys_pmu.time_left <= 0) ||
      ((sys_pmu.b[LEFT_BATTERY].available == NO_BATT) && (sys_pmu.b[RIGHT_BATTERY].available == NO_BATT)))
    return 0;

  /* mandatory ? */
  memset(buf, 0, 128);

  if ((sys_pmu.b[LEFT_BATTERY].charging) && !(sys_pmu.b[RIGHT_BATTERY].charging))
    {
      ref_bat = LEFT_BATTERY;

      sprintf(buf, "Battery level : left charging");
    }
  else if ((sys_pmu.b[RIGHT_BATTERY].charging) && !(sys_pmu.b[LEFT_BATTERY].charging))
    {
      ref_bat = RIGHT_BATTERY;

      sprintf(buf, "Battery level : right charging");
    }
  else
    {
      if (sys_pmu.b[LEFT_BATTERY].percentage > sys_pmu.b[RIGHT_BATTERY].percentage)
	ref_bat = RIGHT_BATTERY;
      else
	ref_bat = LEFT_BATTERY;

      if (sys_pmu.b[LEFT_BATTERY].charging)
	sprintf(buf, "Battery level : both charging");
      else
	sprintf(buf, "Battery level :");
    }

  if (sys_pmu.b[ref_bat].available == NO_BATT)
    {
      ref_bat = (ref_bat == LEFT_BATTERY) ? RIGHT_BATTERY : LEFT_BATTERY;
    }


  if (sys_pmu.b[ref_bat].charging)
    {
      xosd_set_colour(gui.osd, "Red");
      osd_freq = 15 * 60; /* 15 minutes */
    }
  else
    {
      if (sys_pmu.b[ref_bat].percentage > 70)
	{
	  xosd_set_colour(gui.osd, "Green");
	  osd_freq = 15 * 60;
	}
      else if (sys_pmu.b[ref_bat].percentage > 25)
	{
	  xosd_set_colour(gui.osd, "Yellow");
	  osd_freq = 10 * 60;
	}
      else if (sys_pmu.b[ref_bat].percentage > 5)
	{
	  xosd_set_colour(gui.osd, "Red");
	  osd_freq = 5 * 60;
	}
      else
	{
	  xosd_set_colour(gui.osd, "Red");
	  osd_freq = 30;
	}
    }

  xosd_display(gui.osd, 0, XOSD_string, buf);

  if (sys_pmu.b[LEFT_BATTERY].available == IS_BATT)
    xosd_display(gui.osd, 1, XOSD_percentage, sys_pmu.b[LEFT_BATTERY].percentage);

  if (sys_pmu.b[RIGHT_BATTERY].available == IS_BATT)
    xosd_display(gui.osd, 2, XOSD_percentage, sys_pmu.b[RIGHT_BATTERY].percentage);

  /* return next osd timestamp */
  return (time(NULL) + osd_freq);
}
#endif /* ENABLE_XOSD */


/*
 * Display battery state to a TTY, with colors
 */
void DisplayTTY (void)
{
  int i;
  
  /* ac connected ? */
  
  if (sys_pmu.ac_connected)
    fprintf(stdout, "%sP", BWHITE);
  else
    fprintf(stdout, "%s-", BWHITE);
  
  /* charging ? */
  
  if (sys_pmu.b[0].charging)
    fprintf(stdout, "C");
  else
    fprintf(stdout, "-");
  
  /* verify the state of the battery, colors as required */
  
  for (i = 0; i < 2; i++)
    {
      if (sys_pmu.b[i].available == NO_BATT)
	fprintf(stdout, " %s--%%", BRED);
      else
	{
	  if (sys_pmu.b[i].percentage > 70)
	    fprintf(stdout, "%s", BGREEN);
	  else if (sys_pmu.b[i].percentage > 25)
	    fprintf(stdout, "%s", BYELLOW);
	  else
	    fprintf(stdout, "%s", BRED);
	  
	  if (sys_pmu.b[i].percentage < 100)
	    fprintf(stdout, " %02d%%", sys_pmu.b[i].percentage);
	  else
	    fprintf(stdout, " %d%%", sys_pmu.b[i].percentage);
	}
    }
  
  fprintf(stdout, "%s", BWHITE);
  
  /* time left */
  
  fprintf(stdout, " %02d:%02d", sys_pmu.time_left / 60, (sys_pmu.time_left % 60 < 0) ? 0 : sys_pmu.time_left % 60);
  
  fprintf(stdout, "%s\n", BRESET);
}


/* SIGCHLD handler */
void sig_chld (int signo)
{
  waitpid((pid_t) -1, NULL, WNOHANG);
  signal(SIGCHLD, sig_chld);
}

int main(int argc, char *argv[])
{
  int i;
  wm_info wm;
  char *progname;
#ifdef ENABLE_XOSD
  int osd = 1; /* OSD activated by default */
#endif /* ENABLE_XOSD */

  /* init */
  sys_pmu.pmud_domain = PF_INET;
  strcpy(sys_pmu.socket_path, PMUD_SOCKET_PATH);

  sys_pmu.update_rate = UPDATE_RATE;
  sys_pmu.read_pmu = read_pmu_proc;

  xpm.offset.x = 0;
  xpm.offset.y = 0;

  signal(SIGCHLD, sig_chld);

  /* Parse Command Line */

  progname = argv[0];
  if (strlen(progname) > 8)
    progname += (strlen(progname) - 8);
  
  for (i=1; i<argc; i++) {
    char *arg = argv[i];
    
    if (*arg=='-') {
      switch (arg[1]) {
      case 'd' :
	if (strcmp(arg+1, "display")) {
	  usage();
	  exit(1);
	}
	break;
      case 'g' :
	if (strcmp(arg+1, "geometry")) {
	  usage();
	  exit(1);
	}
	break;
      case 'v' :
	printversion();
	exit(0);
	break;
      case 'r':
	if (argc > (i+1)) {
	  sys_pmu.update_rate = (atoi(argv[i+1]) * 1000);
	  i++;
	}
	break;
      case 'u':
	sys_pmu.pmud_domain = PF_UNIX;
	sys_pmu.read_pmu = read_pmu_socket;
	if ((argc > (i+1)) && (argv[i+1][0] != '-'))
	  {
	    i++;
	    if (strlen(argv[i]) > UNIX_PATH_MAX)
	      {
		fprintf(stderr, "%s: UNIX socket path too long!\n", progname);
		exit(1);
	      }
	    else
	      {
		strcpy(sys_pmu.socket_path, argv[i]);
	      }
	  }
	break;
      case 't':
	sys_pmu.pmud_domain = PF_INET;
	sys_pmu.read_pmu = read_pmu_socket;
	break;
      case 'w':
	wm.type = WMAKER;
	break;
      case 'x':
	wm.type = XIMIAN;
	break;
#ifdef ENABLE_XOSD
      case 'o':
	osd = 0;
	break;
#endif /* ENABLE_XOSD */
      default:
	usage();
	exit(0);
	break;
      }
    }
  }

  switch (wm.type)
    {
      case XIMIAN:
	wm.width = 176;
	wm.height = 24;
	wm.xpm = xgbatppc_master_xpm;
	break;
      case WMAKER:
      default:
	wm.type = WMAKER;
	wm.width = 64;
	wm.height = 64;
	wm.xpm = wmbatppc_master_xpm;
	break;
    }

  /*
   * If argv[0] is "wmbatppc", then user wants the WMaker dockapp ;
   * if it is "batppc", then he wants TTY output
   */

  /* use 6 in case argv[0] is only "batppc" */
  if (strncmp(progname, "wmbatppc", 6) != 0)
    {
      /* do tty output */
      sys_pmu.show_charge_time = 1;

      sys_pmu.read_pmu();

      DisplayTTY();
    }
  else
    {
#ifdef ENABLE_XOSD
      /*
       * 1 line of text
       * 1 line for the left battery
       * 1 line for the right battery
       */
      if (osd)
	{
	  gui.osd = xosd_create(3);
	}
      
      if ((osd) && (gui.osd == NULL))
	{
	  fprintf(stderr, "%s: OSD disabled after error (%s)\n", progname, xosd_error);
	}
      else
	{
	  xosd_set_font(gui.osd, "-adobe-helvetica-bold-r-*-*-14-*"); /* "-misc-fixed-medium-r-semicondensed-*-14-*-*-*-c-*-*-*"); */
	  xosd_set_timeout(gui.osd, 5);
	  xosd_set_shadow_offset(gui.osd, 2);
	  xosd_set_vertical_offset(gui.osd, 10);
	  xosd_set_horizontal_offset(gui.osd, 10);
	}
#endif /* ENABLE_XOSD */
      
      wmbatppc_routine(argc, argv, &wm);
      
#ifdef ENABLE_XOSD
      if (gui.osd != NULL)
	{
	  xosd_destroy(gui.osd);
	  gui.osd = NULL;
	}
#endif /* ENABLE_XOSD */
    }

  return 0;
}

/*
 * Main loop
 */
void wmbatppc_routine (int argc, char **argv, wm_info *wm)
{
#ifdef ENABLE_XOSD
  time_t curtime;
  time_t next_osd = 0;
#endif /* ENABLE_XOSD */
  XEvent Event;
  char *mask_bits;
  int i ;

  mask_bits = malloc(wm->width * wm->height * sizeof(char));

  if (mask_bits == NULL)
    {
      fprintf(stderr, "%s: cannot allocate memory !\n", argv[0]);
      return;
    }

  createXBMfromXPM(mask_bits, wm->xpm, wm->width, wm->height);
  openXwindow(argc, argv, wm->xpm, mask_bits, wm->width, wm->height);

  free(mask_bits);

  RedrawWindow();
  
  initGUIelements(wm);

  if (wm->type == WMAKER)
    {
      for (i = 0; i < 10; i++)
	{
	  drawJukeBox(i);
	  updatePlug(UNPLUGGED);
	  updateCharg(NOCHARGE);
	  RedrawWindow();
	  usleep(100000);
	}
      for (i = 9; i >= 0; i--)
	{
	  drawJukeBox(i);
	  updatePlug(UNPLUGGED);
	  updateCharg(NOCHARGE);
	  RedrawWindow();
	  usleep(100000);
	}
    }

  while (1)
    {
      /*
       * Update display
       */
      
      DisplayBat();
      
      RedrawWindow();
      
#ifdef ENABLE_XOSD
      curtime = time(NULL);

      if ((gui.osd != NULL) &&
	  ((curtime >= next_osd) || (next_osd == 0)))
	next_osd = DisplayOSD();
#endif /* ENABLE_XOSD */
    
      /*
       * X Events
       */
      while (XPending(display))
	{
	  XNextEvent(display, &Event);
	  switch (Event.type) 
	    {
	      case Expose:
		RedrawWindow();
		break;
	      case DestroyNotify:
		XCloseDisplay(display);
		exit(0);
	      case ButtonPress:
#ifdef ENABLE_XOSD
		DisplayOSD();
#endif /* ENABLE_XOSD */
		break;
	    }
	}
      
      usleep(sys_pmu.update_rate);
    }
}

/*
 * Blits a string at given co-ordinates
 */
void BlitString (char *name, int x, int y, int is_big)
{
  int i;
  int c;
  int k;
  
  k = x;

  for (i=0; name[i]; i++)
    {
      c = toupper(name[i]); 
      if (c >= 'A' && c <= 'Z') /* its a letter */
	{
	  c -= 'A';
	  copyXPMArea(c * 6, 74+xpm.offset.y, 6, 8, k, y);
	  k += 6;
	} 
      else
	{
	  if (c>='0' && c<='9') /* its a number or symbol */
	    {
	      c -= '0';
	      if (is_big == BIG)
		{
		  copyXPMArea(68 + c * 7 , 22+xpm.offset.y, 7, 10, k, y);
		  k += 7 ;
		}
	      else
		{
		  copyXPMArea(c * 6, 64+xpm.offset.y, 6, 8, k, y);
		  k += 6;
		}
	    }
	  else
	    {
	      copyXPMArea(1, 84+xpm.offset.y, 6, 8, k, y);
	      k += 6;
	    }
	}
    }
}

void BlitNum (int num, int x, int y, int is_big, int two_digits)
{
  char buf[1024];
  int newx=x;
  int temp=0 ;
  
  if (two_digits == 1) 
    sprintf(buf, "%02i", num);
  else
    {
      if (two_digits == 2)
	{
	  temp = num - 100;
	  copyXPMArea(10, 64+xpm.offset.y, 2, 8, x+4, y); /* print the hundreds unit */
	  newx = x + 6;
	  sprintf(buf, "%02i", temp);
	}
      else
	{
	  sprintf(buf, "%03i", num);
	} 
    }
  BlitString(buf, newx, y, is_big);
}

/*
 * Usage
 */
void usage (void)
{
  fprintf(stderr, "\n");
  fprintf(stderr, "wmbatppc %s - Lou <titelou@free.fr>  http://titelou.free.fr\n", WMBATPPC_VERSION);
  fprintf(stderr, "              Julien BLACHE <jb@jblache.org>\n\n");
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "    -display <display name>\n");
  fprintf(stderr, "    -geometry <X11 geometry string>\n");
  fprintf(stderr, "    -r	      update rate in milliseconds (default:100)\n");
  fprintf(stderr, "    -w             WindowMaker/AfterStep GUI\n");
#ifdef ENABLE_XOSD
  fprintf(stderr, "    -o             disable On-Screen Display\n");
#endif /* ENABLE_XOSD */
  fprintf(stderr, "    -x             swallowed application GUI (for GNOME Panel)\n");
  fprintf(stderr, "    -t             use pmud on TCP port 879\n");
  fprintf(stderr, "    -u [PATH]      use pmud via UNIX domain socket\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "When invoked as \'batppc\', a line similar to the following is\n");
  fprintf(stderr, "printed on the console :\n");
  fprintf(stderr, "     PC 100%% 80%% 01:20\n");
  fprintf(stderr, "See man batppc for details about this output.\n");
  fprintf(stderr, "\n");
}

/*
 * printversion
 */
void printversion (void)
{
  fprintf(stderr, "wmbatppc v%s\n", WMBATPPC_VERSION);
}
