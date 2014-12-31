/*
 * code file for wmbatppc
 * by Lou
 */


#include "wmbatppc.h"

#include "wmgeneral.h"

#include "wmbatppc-master.xpm"

#include "xgbatppc-master.xpm"

pmud_info sys_pmu ;

unsigned int wm = WMAKER ;

char wmbatppc_mask_bits[64*64];
char xgbatppc_mask_bits[24*176];
int wmbatppc_mask_width = 64;
int xgbatppc_mask_width= 176;
int wmbatppc_mask_height = 64;
int xgbatppc_mask_height = 24;

coords jukeBoxImg[12] ;
coords plugImg[2];
coords chargImg[2];
coords battImg[8];
coords plugSize ;
coords chargSize ;
coords battSize ;
coords voltIndSize ;
coords voltIndImg ;
coords timeleftIndSize ;
coords timeleftIndImg ;
coords jukeBoxSize[2] ;
int jukeBoxParts ;
int XOFFSET = 0;
int YOFFSET = 0;
interface gui ;

int update_rate=100000;

char *ProgName;

time_t curtime;
time_t prevtime;

int mode = 0 ; // default: no info
int screen = 0;  // default: Quality screen is displayed

void showGUIelement(coords img, coords size, coords dest){
	copyXPMArea(img.x,img.y,size.x,size.y,dest.x,dest.y) ;
}

void updatePlug (int state){
	showGUIelement(plugImg[state],plugSize,gui.plug);
}

void updateCharg (int state){
	showGUIelement(chargImg[state],chargSize,gui.charg);
}

void drawBatteryState(int bat_side, int bat_state){
	showGUIelement(battImg[bat_side*4+bat_state],battSize,gui.batt[bat_side]);
}

void initXIMGUIelements (){
	int i = 0;
	gui.voltage[V].x = 20 ;
	gui.voltage[V].y = 6 ;
	gui.voltage[MV].x = 34 ;
	gui.voltage[MV].y = 6 ;
	gui.symbols[COMMA].x = 32 ;
	gui.symbols[COMMA].y = 11 ;
	gui.percent[LEFT_BATTERY].x = 65 ;
	gui.percent[LEFT_BATTERY].y = 6 ;
	gui.percent[RIGHT_BATTERY].x = 91 ;
	gui.percent[RIGHT_BATTERY].y = 6 ;
	gui.timeleft[HOURS].x = 119 ;
	gui.timeleft[HOURS].y = 6 ;
	gui.timeleft[MINUTES].x = 137 ;
	gui.timeleft[MINUTES].y = 6 ;
	gui.jukeBox[0].x = 23 ;
	gui.jukeBox[0].y = 1 ;
	gui.jukeBox[1].x = 122 ;
	gui.jukeBox[1].y = 1 ;
	gui.batt[LEFT_BATTERY].x = 60 ;
	gui.batt[LEFT_BATTERY].y = 4 ;
	gui.batt[RIGHT_BATTERY].x = 87 ;
	gui.batt[RIGHT_BATTERY].y = 4 ;
	gui.plug.x = 3 ;
	gui.plug.y = 2 ;
	gui.charg.x = 158 ;
	gui.charg.y = 2 ;
	jukeBoxParts = 2 ;
	jukeBoxSize[0].x = 28 ;
	jukeBoxSize[0].y = 4 ;
	jukeBoxSize[1].x = 28 ;
	jukeBoxSize[1].y = 4  ;
	for (i=0; i < 10;i++){
	jukeBoxImg[i].x += 14 ;
	}
}

void initWMGUIelements (){
	gui.voltage[V].x = 15 ;
	gui.voltage[V].y = 2 ;
	gui.voltage[MV].x = 29 ;
	gui.voltage[MV].y = 2 ;
	gui.symbols[COMMA].x = 27 ;
	gui.symbols[COMMA].y = 7 ;
	gui.percent[LEFT_BATTERY].x = 11 ;
	gui.percent[LEFT_BATTERY].y = 49 ;
	gui.percent[RIGHT_BATTERY].x = 36 ;
	gui.percent[RIGHT_BATTERY].y = 49 ;
	gui.timeleft[HOURS].x = 15 ;
	gui.timeleft[HOURS].y = 31 ;
	gui.timeleft[MINUTES].x = 33 ;
	gui.timeleft[MINUTES].y = 31 ;
	gui.jukeBox[0].x = 4 ;
	gui.jukeBox[0].y = 13 ;
	gui.jukeBox[1].x = 0 ;
	gui.jukeBox[1].y = 0 ;
	gui.batt[LEFT_BATTERY].x = 6 ;
	gui.batt[LEFT_BATTERY].y = 47 ;
	gui.batt[RIGHT_BATTERY].x = 33 ;
	gui.batt[RIGHT_BATTERY].y = 47 ;
	gui.plug.x = 2 ;
	gui.plug.y = 2 ;
	gui.charg.x = 50 ;
	gui.charg.y = 2 ;
	jukeBoxParts = 1 ;
	jukeBoxSize[0].x = 56 ;
	jukeBoxSize[0].y = 30 ;
	jukeBoxSize[1].x = 0 ;
	jukeBoxSize[1].y = 0 ;
}

void initXPMelements(){
	timeleftIndSize.x = 36 ;
	timeleftIndSize.y = 13 ;
	timeleftIndImg.x = 15;
	timeleftIndImg.y = 29 + YOFFSET ;
	voltIndSize.x = 34 ;
	voltIndSize.y = 10 ;
	voltIndImg.x = 2;
	voltIndImg.y = 84 + YOFFSET; 
	battImg[LEFT_BATTERY*4+HIGH_BATT].x = 146  ;
	battImg[LEFT_BATTERY*4+HIGH_BATT].y = 6 + YOFFSET  ;
	battImg[LEFT_BATTERY*4+LOW_BATT].x = 173  ;
	battImg[LEFT_BATTERY*4+LOW_BATT].y = 6  + YOFFSET ;
	battImg[LEFT_BATTERY*4+MED_BATT].x = 200 ;
	battImg[LEFT_BATTERY*4+MED_BATT].y = 6  + YOFFSET ;
	battImg[LEFT_BATTERY*4+NO_BATT].x = 108 ;
	battImg[LEFT_BATTERY*4+NO_BATT].y = 85  + YOFFSET ;
	battImg[RIGHT_BATTERY*4+HIGH_BATT].x = 146 ;
	battImg[RIGHT_BATTERY*4+HIGH_BATT].y = 21  + YOFFSET ;
	battImg[RIGHT_BATTERY*4+LOW_BATT].x = 173 ;
	battImg[RIGHT_BATTERY*4+LOW_BATT].y = 21  + YOFFSET ;
	battImg[RIGHT_BATTERY*4+MED_BATT].x = 200 ;
	battImg[RIGHT_BATTERY*4+MED_BATT].y = 21  + YOFFSET ;
	battImg[RIGHT_BATTERY*4+NO_BATT].x = 135 ;
	battImg[RIGHT_BATTERY*4+NO_BATT].y = 85  + YOFFSET ;
	battSize.x = 25 ;
	battSize.y = 13 ;
	plugSize.x = 12 ;
	plugSize.y = 16 ;
	chargSize.x = 12 ;
	chargSize.y = 16 ;
	plugImg[0].x = 158 ;
	plugImg[0].y = 53  + YOFFSET ;
	plugImg[1].x = 158 ;
	plugImg[1].y = 36  + YOFFSET ;
	chargImg[0].x = 134  ;
	chargImg[0].y = 53  + YOFFSET ;
	chargImg[1].x = 134 ;
	chargImg[1].y = 36  + YOFFSET ;
	jukeBoxImg[0].x = 173 ; 
	jukeBoxImg[0].y = 68  + YOFFSET ;
	jukeBoxImg[1].x = 5 ;
	jukeBoxImg[1].y = 101  + YOFFSET ;
	jukeBoxImg[2].x = 126 ;
	jukeBoxImg[2].y = 135  + YOFFSET ;
	jukeBoxImg[3].x = 4 ;
	jukeBoxImg[3].y = 168  + YOFFSET ;
	jukeBoxImg[4].x = 127 ;
	jukeBoxImg[4].y = 168  + YOFFSET ;
	jukeBoxImg[5].x = 65 ;
	jukeBoxImg[5].y = 168  + YOFFSET ;
	jukeBoxImg[6].x = 127 ;
	jukeBoxImg[6].y = 101  + YOFFSET ;
	jukeBoxImg[7].x = 4 ;
	jukeBoxImg[7].y = 135  + YOFFSET ;
	jukeBoxImg[8].x = 65 ;
	jukeBoxImg[8].y = 135  + YOFFSET ;
	jukeBoxImg[9].x = 66 ;
	jukeBoxImg[9].y = 101  + YOFFSET ;
	jukeBoxImg[10].x = 173 ;
	jukeBoxImg[10].y = 36  + YOFFSET ;
}

void initGUIelements(){
	if (wm == WMAKER){
		initXPMelements();
		initWMGUIelements();
	}
	else
	{ 	YOFFSET = 30 ;
		initXPMelements();
		initXIMGUIelements();
	}
}

void drawJukeBox (int state){
	showGUIelement (jukeBoxImg[state],jukeBoxSize[0],gui.jukeBox[0]) ;
	if (jukeBoxParts > 1)
		showGUIelement (jukeBoxImg[state],jukeBoxSize[1],gui.jukeBox[1]) ;
}

int open_pmud_socket() {

     	int ret;
	     struct sockaddr_in sin ;
	     sin.sin_family = PF_INET ;
	     sin.sin_port = htons (PMUD_PORT) ;
	     sin.sin_addr.s_addr = htonl (INADDR_LOOPBACK) ;

	  if ((ret = socket (PF_INET, SOCK_STREAM, 0)) >= 0) {
	       if (connect (ret, (struct sockaddr *) &sin, sizeof (sin)) >= 0) {
		    return ret;
	       } else {
		    close (ret);
	       }
	  }
     	return -1;
}


enum { KL_IBOOK, KL_PISMO, KL_UNKNOWN };

static int keylargo_identify ()
{
     FILE *fd;
     char buf[25];
     if (!(fd = fopen ("/proc/device-tree/pci/mac-io/media-bay/compatible", "ro")))
	  return KL_IBOOK;
     /* no media-bay.  definately an iBook...
      * or a Pismo running a kernel without OF-devtree support...
      */
     fscanf (fd, "%s", buf);
     fclose (fd);
     if (!strcmp ("keylargo-media-bay", buf))	// only the pismo should have one
	  return KL_PISMO;	// has one, its a pismo
     else
	  return KL_UNKNOWN;
     /* has a media bay, IDs as a Keylargo, but not a
      * "keylargo-media-bay.... what the...
      */
}

void read_g3_pmu ()
{	/* smart battery systems are the same */
     unsigned char par;
     int current, charge, maxcharge, ac_hold = -1;
     char *b;
     char c[150];
     static int sock = -1;
     
     if (sock < 0) {
	  if ((sock = open_pmud_socket ()) < 0) {
	       sys_pmu.time_left = -2;
	       return;
	  } else {
	       read (sock, c, 150);
	  }
     }
#ifdef PMUD_REFRESH
     read (sock, c, 150);
     shutdown (sock, SHUT_RDWR);
     close (sock);
     sock = -1;
#else
     write (sock, "\n", 1);
     read (sock, c, 150);
#endif
     b = strtok (c, PMUD_INPUT_DELIM);
     if (b[0] != 'S')
	  return;
     sys_pmu.batt_state[LEFT_BATTERY] = NO_BATT ;
     sys_pmu.batt_state[RIGHT_BATTERY] = NO_BATT ;

     sys_pmu.time_left = 0;
     for (par = 0; par < 2; par++) {
	  ac_hold = atoi (strtok (0, PMUD_INPUT_DELIM));
	  if (ac_hold % 10) {
	       sys_pmu.b[par].available = IS_BATT ;
	       sys_pmu.b[par].charging = (((ac_hold / 10) % 10) == 1);
	       charge = atoi (strtok (0, PMUD_INPUT_DELIM));
	       maxcharge = atoi (strtok (0, PMUD_INPUT_DELIM)) - 2 ;
	       sys_pmu.b[par].current = atoi (strtok (0, PMUD_INPUT_DELIM));
	       sys_pmu.b[par].voltage = atoi (strtok (0, PMUD_INPUT_DELIM));
	       sys_pmu.b[par].percentage = charge * 100 / maxcharge ;
	       sys_pmu.time_left += ((ac_hold / 100) ? maxcharge - charge : charge);
	       sys_pmu.batt_state[par] = IS_BATT ;
	  } else {
	       sys_pmu.b[par].available = NO_BATT ;
	       sys_pmu.b[par].percentage = 0;
	       sys_pmu.b[par].current = 0;
	       sys_pmu.b[par].voltage = 0.0;
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

void DisplayBat(void) {
	
	int keylargo ;
	unsigned int par = 0 ;
	int hour = 0 ;
	int min = 0 ;
	int volts = 0 ;
	int millivolts = 0 ;
	int i;
	int state ;

	sys_pmu.show_charge_time = 1 ;

	keylargo = keylargo_identify() ; // TODO : use this detection

	read_g3_pmu () ;
	
	if (sys_pmu.time_left == -1)
		drawJukeBox(0) ;
	else
	{	state = (sys_pmu.current + 200) / 200 ;
		if (state < 0)
			state = state * -1 ;
		if (state > 9) 
			state = 9 ;
		drawJukeBox(state);
		hour =	sys_pmu.time_left / 60 ;
		min = sys_pmu.time_left % 60 ;
		// printf("%i,%d,%d\n",sys_pmu.time_left,hour,min);
		BlitNum(hour,gui.timeleft[HOURS].x,gui.timeleft[HOURS].y,BIG,1);  
		BlitNum(min,gui.timeleft[MINUTES].x,gui.timeleft[MINUTES].y,BIG,1);  
	}

	if (sys_pmu.ac_connected)
		updatePlug(PLUGGED) ;
	else 
		updatePlug(UNPLUGGED) ;

	if (sys_pmu.b[LEFT_BATTERY].charging || sys_pmu.b[RIGHT_BATTERY].charging)
		updateCharg(CHARGE) ;
	else
		updateCharg(NOCHARGE) ;

     for (par=0; par<2; par++){
	if (sys_pmu.b[par].available == NO_BATT){
		drawBatteryState(par,NO_BATT) ;
	}else {
		if (sys_pmu.b[par].percentage > 70)
			drawBatteryState(par,HIGH_BATT) ;
		else if (sys_pmu.b[par].percentage > 25)
			drawBatteryState(par,MED_BATT) ;
		else if ((sys_pmu.b[par].percentage > 5) || (sys_pmu.b[par].charging) || (sys_pmu.b[1-par].percentage > 5))
			drawBatteryState(par,LOW_BATT) ;
		else 
			{  
			for (i=0; i<update_rate/50000; i++){
				drawBatteryState(par,NO_BATT) ;
				RedrawWindow() ;
				usleep(200000) ;
				drawBatteryState(par,LOW_BATT) ;
				RedrawWindow() ;
			}
		}

		if (sys_pmu.b[par].percentage == 100){
			BlitNum(sys_pmu.b[par].percentage,gui.percent[par].x-6,gui.percent[par].y,SMALL,2) ;
		} else {
			BlitNum(sys_pmu.b[par].percentage,gui.percent[par].x,gui.percent[par].y,SMALL,1) ;
		}
	
		volts = sys_pmu.b[par].voltage / 1000 ;
		millivolts =  sys_pmu.b[par].voltage % 1000 ;
		BlitNum(volts,gui.voltage[V].x,gui.voltage[V].y,SMALL,1) ;
		BlitNum(millivolts,gui.voltage[MV].x,gui.voltage[MV].y,SMALL,0) ;
		copyXPMArea(74,69+YOFFSET,2,3,gui.symbols[COMMA].x,gui.symbols[COMMA].y);
	}
     }

	if (sys_pmu.b[LEFT_BATTERY].available == NO_BATT 
			&& sys_pmu.b[RIGHT_BATTERY].available == NO_BATT)
		copyXPMArea(voltIndImg.x,voltIndImg.y,voltIndSize.x,voltIndSize.y,gui.voltage[V].x,gui.voltage[V].y) ;
}

/* SIGCHLD handler */
void sig_chld(int signo)
{
  waitpid((pid_t) -1, NULL, WNOHANG);
  signal(SIGCHLD, sig_chld);
}

int main(int argc, char *argv[]) {
  int i;
  
  /* Parse Command Line */
  
  signal(SIGCHLD, sig_chld);
  ProgName = argv[0];
  if (strlen(ProgName) >= 5)
    ProgName += (strlen(ProgName) - 5);
  
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
	  update_rate = (atoi(argv[i+1]) * 1000);
	  i++;
	}
	break;
      case 'w':
	wm = WMAKER ;
	break;
      case 'x':
	wm = XIMIAN ;
	break;
      default:
	usage();
	exit(0);
	break;
      }
    }
  }
  
  wmbatppc_routine(argc, argv);
  
  return 0;
}

/*
 * Main loop
 */
void wmbatppc_routine(int argc, char **argv) {
  XEvent Event;
  struct timeval tv={0,0};
  struct timeval last={0,0};
  int i ;
 
  if (wm == WMAKER){
	  printf("window maker version \n");
  	createXBMfromXPM(wmbatppc_mask_bits, wmbatppc_master_xpm, wmbatppc_mask_width, wmbatppc_mask_height);
  openXwindow(argc, argv, wmbatppc_master_xpm, wmbatppc_mask_bits, wmbatppc_mask_width, wmbatppc_mask_height);
  } else {
	  printf("ximian gnome version \n");
  	createXBMfromXPM(xgbatppc_mask_bits, xgbatppc_master_xpm, xgbatppc_mask_width, xgbatppc_mask_height);
  openXwindow(argc, argv, xgbatppc_master_xpm, xgbatppc_mask_bits, xgbatppc_mask_width, xgbatppc_mask_height);
  }

  RedrawWindow();

  initGUIelements () ;

  if (wm == WMAKER){
  for (i=0;i<10;i++){
	drawJukeBox(i) ;
	updatePlug(UNPLUGGED) ;
	updateCharg(NOCHARGE) ;
	RedrawWindow() ;
	usleep(100000) ;
  }
  for (i=9;i>=0;i--){
	drawJukeBox(i) ;
	updatePlug(UNPLUGGED) ;
	updateCharg(NOCHARGE) ;
	RedrawWindow() ;
	usleep(100000) ;
  }
  }
  
  
  while (1) {
    
    curtime = time(0);
    
    if (1) {
      memcpy(&last, &tv, sizeof(tv));
      
      /*
       * Update display
       */

      DisplayBat();
      
      RedrawWindow();
    }
    
    /*
     * X Events
     */
    while (XPending(display)) {
      XNextEvent(display, &Event);
      switch (Event.type) {
      case Expose:
	RedrawWindow();
	break;
      case DestroyNotify:
	XCloseDisplay(display);
	exit(0);
      case ButtonPress:
	switch (screen) {
	case 0: screen=1; break;
	case 1: screen=0; break;
	};
	break;
      }
    }
    
    usleep(update_rate);
  }
}

/*
 * Blits a string at given co-ordinates
 */
void BlitString(char *name, int x, int y, int is_big) {
  int	i;
  int	c;
  int	k;

  k = x;

  for (i=0; name[i]; i++){
	c = toupper(name[i]); 
	if (c >= 'A' && c <= 'Z'){   // its a letter
		c -= 'A';
		copyXPMArea(c * 6, 74+YOFFSET, 6, 8, k, y);
		k += 6;
	} else {
	  	if (c>='0' && c<='9'){   // its a number or symbol
	 		c -= '0';
	  		if (is_big == BIG){
	 			copyXPMArea(68 + c * 7 , 22+YOFFSET, 7, 10, k, y);
				k += 7 ;
			} else {
	 			copyXPMArea(c * 6, 64+YOFFSET, 6, 8, k, y);
	  			k += 6;
			}
	  	} else {
	    		copyXPMArea(1, 84+YOFFSET, 6, 8, k, y);
	    		k += 6;
	  	}
	}
   }
}

void BlitNum(int num, int x, int y, int is_big, int two_digits) {
  char buf[1024];
  int newx=x;
  int temp=0 ;

  if (two_digits == 1) 
  	sprintf(buf, "%02i", num);
  else {
	  if (two_digits == 2) {
		temp = num - 100 ;
		copyXPMArea(10, 64+YOFFSET, 2, 8, x+4, y); /* print the hundreds unit */
		newx = x + 6 ;
  		sprintf(buf, "%02i", temp);
	  } else {
  		sprintf(buf, "%03i", num);
	  } 
  }
  BlitString(buf, newx, y, is_big);
}

/*
 * Usage
 */
void usage(void) {
  fprintf(stderr, "\nwmbatppc - Lou <titelou@free.fr>  http://titelou.free.fr\n\n");
  fprintf(stderr, "usage:\n");
  fprintf(stderr, "    -display <display name>\n");
  fprintf(stderr, "    -r	update rate in milliseconds (default:100)\n");
  fprintf(stderr, "\n");
}

/*
 * printversion
 */
void printversion(void) {
  fprintf(stderr, "wmbatppc v%s\n", WMBATPPC_VERSION);
}
