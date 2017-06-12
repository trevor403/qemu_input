#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include <linux/input.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include <pthread.h>
#include <signal.h>
#include <errno.h>

enum {
	VERBOSE    = true,
  VALUE_REPEATED = 2,
	VALUE_PRESSED  = 1,
	VALUE_RELEASED = 0,
};

struct monitor_t {
  struct sockaddr_in serv_addr;
  struct hostent *server;
  int sockfd;
  int portno;
  char buffer[256];
};

struct modifiers_t {
  bool ctrl;
  bool shift;
  bool meta;
  bool alt;
};

const char *keyboard_dev = "/dev/input/by-id/usb-Logitech_USB_Receiver-event-kbd"; //"/dev/input/by-path/platform-i8042-serio-0-event-kbd";
const char *mouse_dev = "/dev/input/by-id/usb-Logitech_USB_Receiver-if01-event-mouse";

const char *mod_noop = "";
const char *mod_ctrl = "ctrl-";
const char *mod_shift = "shift-";
const char *mod_meta = "0xdb-";
const char *mod_alt = "alt-";

const char *ev_type[EV_CNT] = {
	[EV_SYN]       = "EV_SYN",
	[EV_KEY]       = "EV_KEY",
	[EV_REL]       = "EV_REL",
	[EV_ABS]       = "EV_ABS",
	[EV_MSC]       = "EV_MSC",
	[EV_SW]        = "EV_SW",
	[EV_LED]       = "EV_LED",
	[EV_SND]       = "EV_SND",
	[EV_REP]       = "EV_REP",
	[EV_FF]        = "EV_FF",
	[EV_PWR]       = "EV_PWR",
	[EV_FF_STATUS] = "EV_FF_STATUS",
	[EV_MAX]       = "EV_MAX",
};

const char *ev_code_syn[SYN_CNT] = {
	[SYN_REPORT]    = "SYN_REPORT",
	[SYN_CONFIG]    = "SYN_CONFIG",
	[SYN_MT_REPORT] = "SYN_MT_REPORT",
	[SYN_DROPPED]   = "SYN_DROPPED",
	[SYN_MAX]       = "SYN_MAX",
};

const char *ev_code_rel[REL_CNT] = {
	[REL_X]      = "REL_X",
	[REL_Y]      = "REL_Y",
	[REL_Z]      = "REL_Z",
	[REL_RX]     = "REL_RX",
	[REL_RY]     = "REL_RY",
	[REL_RZ]     = "REL_RZ",
	[REL_HWHEEL] = "REL_WHEEL",
	[REL_DIAL]   = "REL_DIAL",
	[REL_WHEEL]  = "REL_WHEEL",
	[REL_MISC]   = "REL_MISC",
	[REL_MAX]    = "REL_MAX",
};

const char *ev_code_key[KEY_CNT] = {
	[BTN_LEFT]           = "BTN_LEFT",
	[BTN_RIGHT]          = "BTN_RIGHT",
	[BTN_MIDDLE]         = "BTN_MIDDLE",
	[BTN_SIDE]           = "BTN_SIDE",
	[BTN_EXTRA]          = "BTN_EXTRA",
	[BTN_FORWARD]        = "BTN_FORWARD",
	[BTN_BACK]           = "BTN_BACK",
	[BTN_TASK]           = "BTN_TASK",
	[KEY_MAX]            = "KEY_MAX",
//keyboard
  [KEY_ESC]            = "0x01",
  [KEY_1]              = "1",
  [KEY_2]              = "2",
  [KEY_3]              = "3",
  [KEY_4]              = "4",
  [KEY_5]              = "5",
  [KEY_6]              = "6",
  [KEY_7]              = "7",
  [KEY_8]              = "8",
  [KEY_9]              = "9",
  [KEY_0]              = "0",
  [KEY_MINUS]          = "minus",
  [KEY_EQUAL]          = "equal",
  [KEY_BACKSPACE]      = "backspace",
  [KEY_TAB]            = "tab",
  [KEY_Q]              = "q",
  [KEY_W]              = "w",
  [KEY_E]              = "e",
  [KEY_R]              = "r",
  [KEY_T]              = "t",
  [KEY_Y]              = "y",
  [KEY_U]              = "u",
  [KEY_I]              = "i",
  [KEY_O]              = "o",
  [KEY_P]              = "p",
  [KEY_LEFTBRACE]      = "0x1a",
  [KEY_RIGHTBRACE]     = "0x1b",
  [KEY_ENTER]          = "ret",
  [KEY_LEFTCTRL]       = "Q_KEY_CODE_CTRL",
  [KEY_A]              = "a",
  [KEY_S]              = "s",
  [KEY_D]              = "d",
  [KEY_F]              = "f",
  [KEY_G]              = "g",
  [KEY_H]              = "h",
  [KEY_J]              = "j",
  [KEY_K]              = "k",
  [KEY_L]              = "l",
  [KEY_SEMICOLON]      = "0x27",
  [KEY_APOSTROPHE]     = "0x28",
  [KEY_GRAVE]          = "0x29",
  [KEY_LEFTSHIFT]      = "Q_KEY_CODE_SHIFT",
  [KEY_BACKSLASH]      = "0x2b",
  [KEY_102ND]          = "less",
  [KEY_Z]              = "z",
  [KEY_X]              = "x",
  [KEY_C]              = "c",
  [KEY_V]              = "v",
  [KEY_B]              = "b",
  [KEY_N]              = "n",
  [KEY_M]              = "m",
  [KEY_COMMA]          = "comma",
  [KEY_DOT]            = "dot",
  [KEY_SLASH]          = "slash",
  [KEY_RIGHTSHIFT]     = "Q_KEY_CODE_SHIFT_R",
  [KEY_LEFTALT]        = "Q_KEY_CODE_ALT",
  [KEY_SPACE]          = "spc",
  [KEY_CAPSLOCK]       = "caps_lock",
  [KEY_F1]             = "f1",
  [KEY_F2]             = "f2",
  [KEY_F3]             = "f3",
  [KEY_F4]             = "f4",
  [KEY_F5]             = "f5",
  [KEY_F6]             = "f6",
  [KEY_F7]             = "f7",
  [KEY_F8]             = "f8",
  [KEY_F9]             = "f9",
  [KEY_F10]            = "f10",
  [KEY_NUMLOCK]        = "0x45",
  [KEY_SCROLLLOCK]     = "0x46",
  [KEY_KP0]            = "kp_0",
  [KEY_KP1]            = "kp_1",
  [KEY_KP2]            = "kp_2",
  [KEY_KP3]            = "kp_3",
  [KEY_KP4]            = "kp_4",
  [KEY_KP5]            = "kp_5",
  [KEY_KP6]            = "kp_6",
  [KEY_KP7]            = "kp_7",
  [KEY_KP8]            = "kp_8",
  [KEY_KP9]            = "kp_9",
  [KEY_KPMINUS]        = "kp_subtract",
  [KEY_KPPLUS]         = "kp_add",
  [KEY_KPDOT]          = "kp_decimal",
  [KEY_KPENTER]        = "kp_enter",
  [KEY_KPSLASH]        = "kp_divide",
  [KEY_KPASTERISK]     = "kp_multiply",
  [KEY_F11]            = "f11",
  [KEY_F12]            = "f12",
  [KEY_RIGHTCTRL]      = "Q_KEY_CODE_CTRL_R",
  [KEY_SYSRQ]          = "sysrq",
  [KEY_RIGHTALT]       = "Q_KEY_CODE_ALT_R",
  [KEY_HOME]           = "home",
  [KEY_UP]             = "up",
  [KEY_PAGEUP]         = "pgup",
  [KEY_LEFT]           = "left",
  [KEY_RIGHT]          = "right",
  [KEY_END]            = "end",
  [KEY_DOWN]           = "down",
  [KEY_PAGEDOWN]       = "pgdn",
  [KEY_INSERT]         = "insert",
  [KEY_DELETE]         = "delete",
  [KEY_LEFTMETA]       = "Q_KEY_CODE_META_L",
  [KEY_RIGHTMETA]      = "Q_KEY_CODE_META_R",
  [KEY_MENU]           = "0xdd",
};

static volatile int keepRunning = 1;

struct {
  int keyboard_fd;
  int mouse_fd;
  int sockfd;
} fds;

void intHandler(int dummy) {
  keepRunning = 0;
  
  close(fds.keyboard_fd);
  close(fds.mouse_fd);

  close(fds.sockfd);

  printf("\nBye bitch\n");

	exit(EXIT_SUCCESS);

}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void init_keyboard(int *fd)
{
	if ((*fd = open(keyboard_dev, O_RDONLY)) == -1)
		error("opening device");

  bool state = false;
  if (ioctl(*fd, EVIOCGRAB, &state) < 0) error("ERROR could not grab Keyboard");
  else printf("Grabbed Keyboard!\n");
}


void init_mouse(int *fd)
{
	if ((*fd = open(mouse_dev, O_RDONLY)) == -1)
		error("opening device");

  bool state = false;
  if (ioctl(*fd, EVIOCGRAB, &state) < 0) error("ERROR could not grab Mouse");
  else printf("Grabbed Mouse!\n");
}

void init_socket(struct monitor_t *mon)
{
  mon->sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (mon->sockfd < 0) 
      error("ERROR opening socket");

  mon->server = gethostbyname("10.51.146.70");
  if (mon->server == NULL)
    error("ERROR, no such host");

  bzero((char *) &mon->serv_addr, sizeof(mon->serv_addr));
  mon->serv_addr.sin_family = AF_INET;

  bcopy((char *)mon->server->h_addr, (char*)&mon->serv_addr.sin_addr.s_addr, mon->server->h_length);
  mon->serv_addr.sin_port = htons(mon->portno);

  if (connect(mon->sockfd,(struct sockaddr*) &mon->serv_addr,sizeof(mon->serv_addr)) < 0) 
    error("ERROR connecting");

  bzero(mon->buffer,256);
}

void print_event(struct input_event *ie)
{
	switch (ie->type) {
	case EV_SYN:
		fprintf(stderr, "time:%ld.%06ld\ttype:%s\tcode:%s\tvalue:%d\n",
			ie->time.tv_sec, ie->time.tv_usec, ev_type[ie->type],
			ev_code_syn[ie->code], ie->value);
		break;
	case EV_REL:
		fprintf(stderr, "time:%ld.%06ld\ttype:%s\tcode:%s\tvalue:%d\n",
			ie->time.tv_sec, ie->time.tv_usec, ev_type[ie->type],
			ev_code_rel[ie->code], ie->value);
		break;
	case EV_KEY:
		fprintf(stderr, "time:%ld.%06ld\ttype:%s\tcode:%s\tvalue:%d\n",
			ie->time.tv_sec, ie->time.tv_usec, ev_type[ie->type],
			ev_code_key[ie->code], ie->value);
		break;
	default:
		break;
	}
}

void cursor(struct input_event *ie, struct monitor_t *mon)
{
  int dx = 0, dy = 0, dz = 0;

	if (ie->code == REL_X)
		dx = ie->value;

	if (ie->code == REL_Y)
		dy = ie->value;

	if (ie->code == REL_WHEEL)
		dz = ie->value;

  sprintf(mon->buffer, "mouse_move %d %d %d\n", dx, dy, dz);
  if (send(mon->sockfd,mon->buffer,strlen(mon->buffer), 0) < 0) 
    error("ERROR writing to socket");
}

void mouse_button(struct input_event *ie, struct monitor_t *mon)
{
	int button;
  if (ie->value == VALUE_RELEASED) button = 0;
  else switch (ie->code) {
    case BTN_LEFT: button = 1; break;
    case BTN_RIGHT: button = 2; break;
    case BTN_MIDDLE: button = 4; break;
  }
  sprintf(mon->buffer, "mouse_button %d\n", button);
  if (send(mon->sockfd,mon->buffer,strlen(mon->buffer), 0) < 0) 
    error("ERROR writing to socket");  
}

void keyboard_button(struct input_event *ie, struct modifiers_t *mod, struct monitor_t *mon)
{
  switch(ie->code) {
    case KEY_LEFTCTRL:
    case KEY_RIGHTCTRL:
      mod->ctrl = ie->value;
      break;
    case KEY_LEFTSHIFT:
    case KEY_RIGHTSHIFT:
      mod->shift = ie->value;
      break;
    case KEY_LEFTALT:
    case KEY_RIGHTALT:
      mod->alt = ie->value;
      break;
    case KEY_LEFTMETA:
    case KEY_RIGHTMETA:
      mod->meta = ie->value;
      break;
  }
  
  const char * ctrl = mod->ctrl ? mod_ctrl : mod_noop;
  const char * shift = mod->shift ? mod_shift : mod_noop;
  const char * meta = mod->meta ? mod_meta : mod_noop;
  const char * alt = mod->alt ? mod_alt : mod_noop;

  if (ie->value == VALUE_PRESSED || ie->value == VALUE_REPEATED) {
    bzero(mon->buffer,256);
    sprintf(mon->buffer, "sendkey %s%s%s%s%s\n", ctrl, meta, alt, shift, ev_code_key[ie->code]);
    if(ev_code_key[ie->code][0] == 'Q') {
      if (mod->ctrl && mod->alt) intHandler(0);
      else return;
    }
    printf(mon->buffer);
    if (send(mon->sockfd,mon->buffer,strlen(mon->buffer), 0) < 0) 
      error("ERROR writing to socket");
  }
}

/*
void sync(struct input_event *ie, struct mouse_t *mouse)
{
	if (ie->code != SYN_REPORT)
		return;
}
*/

void (*keyboard_event_handler[EV_CNT])(struct input_event *ie, struct modifiers_t *mod, struct monitor_t *mon) = {
	[EV_KEY] = keyboard_button, 
	[EV_MAX] = NULL, 
};

void (*mouse_event_handler[EV_CNT])(struct input_event *ie, struct monitor_t *mon) = {
	//[EV_SYN] = sync,
	[EV_REL] = cursor,
	[EV_KEY] = mouse_button, 
	[EV_MAX] = NULL, 
};

// https://cboard.cprogramming.com/linux-programming/116184-best-solution-inter-thread-communication.html
// https://github.com/LnxPrgr3/message_queue

void *keyboard_evdev(void *ptr)
{
  int keyboard_fd;
  init_keyboard(&keyboard_fd);
  fds.keyboard_fd = keyboard_fd;

  struct monitor_t *mon = (struct monitor_t *)(ptr);

  struct modifiers_t mod;
  bzero((char *) &mod, sizeof(mod));

	struct input_event ie;
  while(keepRunning) {
    read(keyboard_fd, &ie, sizeof(struct input_event));
    //print_event(&ie);

		if (keyboard_event_handler[ie.type])
			keyboard_event_handler[ie.type](&ie, &mod, mon);
  }

  return NULL;
}

void *mouse_evdev(void *ptr)
{
  int mouse_fd;
  init_mouse(&mouse_fd);
  fds.mouse_fd = mouse_fd;

  struct monitor_t *mon = (struct monitor_t *)(ptr);

	struct input_event ie;
  while(keepRunning) {
    read(mouse_fd, &ie, sizeof(struct input_event));
    //print_event(&ie);
		
		if (mouse_event_handler[ie.type])
			mouse_event_handler[ie.type](&ie, mon);
  }

  return NULL;
}

int main(int argc, char *argv[])
{
  //int keyboard_fd;
  struct monitor_t mon;
  mon.portno = atoi(argv[1]);
  init_socket(&mon);
  fds.sockfd = mon.sockfd;

  sleep(3);  

  signal(SIGINT, intHandler);

  pthread_t keyboard_thread;
  pthread_t mouse_thread;

  if(pthread_create(&keyboard_thread, NULL, keyboard_evdev, &mon))
    error("Error creating keyboard thread");

  if(pthread_create(&mouse_thread, NULL, mouse_evdev, &mon))
    error("Error creating mouse thread");

  if(pthread_join(keyboard_thread, NULL))
    error("Error joining keyboard thread");

  if(pthread_join(mouse_thread, NULL))
    error("Error joining mouse thread");

  printf("womp?\n");

  return -1;
}

