#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<sys/wait.h>
#include<X11/Xlib.h>
#define LENGTH(X)               (sizeof(X) / sizeof (X[0]))
#define CMDLENGTH		50

#ifdef __OpenBSD__
#define SIGPLUS			SIGUSR1+1
#define SIGMINUS		SIGUSR1-1
#else
#define SIGPLUS			SIGRTMIN
#define SIGMINUS		SIGRTMIN
#endif

typedef struct {
	char* icon;
	char* command;
	unsigned int interval;
	unsigned int signal;
} Block;
void getcmds(int time);
#ifndef __OpenBSD__
void getsigcmds(int signal);
void setupsignals();
void sighandler(int signum, siginfo_t *si, void *ucontext);
#endif
int getstatus(char *str, char *last);
void setroot();
void statusloop();
void termhandler(int signum);
void chldhandler();


#include "blocks.h"

static Display *dpy;
static int screen;
static Window root;
static char statusbar[LENGTH(blocks)][CMDLENGTH] = {0};
static char statusstr[2][256];
static int statusContinue = 1;
static void (*writestatus) () = setroot;

//opens process *cmd and stores output in *output
void getcmd(const Block *block, char *output)
{
  if (block->signal) {
    *output++ = block->signal;
  }

	strcpy(output, block->icon);
	char *cmd = block->command;
	FILE *cmdf = popen(cmd,"r");
	if (!cmdf)
		return;
	char c;
	int i = strlen(block->icon);
	fgets(output+i, CMDLENGTH-i, cmdf);
	i = strlen(output);
	if (delim != '\0' && --i)
		output[i++] = delim;
	output[i++] = '\0';
	pclose(cmdf);
}

void getcmds(int time)
{
	const Block* current;
	for(int i = 0; i < LENGTH(blocks); i++)
	{	
		current = blocks + i;
		if ((current->interval != 0 && time % current->interval == 0) || time == -1)
			getcmd(current,statusbar[i]);
	}
}

#ifndef __OpenBSD__
void getsigcmds(int signal)
{
	const Block *current;
	for (int i = 0; i < LENGTH(blocks); i++)
	{
		current = blocks + i;
		if (current->signal == signal)
			getcmd(current,statusbar[i]);
	}
}

void setupsignals()
{
	struct sigaction sa = { .sa_sigaction = sighandler, .sa_flags = SA_SIGINFO };
 #ifndef __OpenBSD__
  /* initialize all real time signals with dummy handler */
  for (int i = SIGRTMIN; i <= SIGRTMAX; i++) {
         //signal(i, dummysighandler);
         signal(i, sighandler);
		sigaddset(&sa.sa_mask, i);
	}
 #endif
 
 	for (unsigned int i = 0; i < LENGTH(blocks); i++) {
 		if (blocks[i].signal > 0)
			sigaction(SIGMINUS+blocks[i].signal, &sa, NULL);
 	}

}
#endif

int getstatus(char *str, char *last)
{
	strcpy(last, str);
	str[0] = '\0';
	for(int i = 0; i < LENGTH(blocks); i++)
		strcat(str, statusbar[i]);
	str[strlen(str)-1] = '\0';
	return strcmp(str, last);//0 if they are the same
}

void setroot()
{
	if (!getstatus(statusstr[0], statusstr[1]))//Only set root if text has changed.
		return;
	Display *d = XOpenDisplay(NULL);
	if (d) {
		dpy = d;
	}
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
	XStoreName(dpy, root, statusstr[0]);
	XCloseDisplay(dpy);
}

void pstdout()
{
	if (!getstatus(statusstr[0], statusstr[1]))//Only write out if text has changed.
		return;
	printf("%s\n",statusstr[0]);
	fflush(stdout);
}


void statusloop()
{
#ifndef __OpenBSD__
	setupsignals();
#endif
	int i = 0;
	getcmds(-1);
	while(statusContinue)
	{
		getcmds(i);
		writestatus();
		sleep(1.0);
		i++;
	}
}

#ifndef __OpenBSD__
void sighandler(int signum, siginfo_t *si, void *ucontext)
{
	if (si->si_value.sival_int) {
		pid_t parent = getpid();
		if (fork() == 0) {
#ifndef NO_X
			if (dpy)
				close(ConnectionNumber(dpy));
#endif
    int i;
    for (i = 0; i < LENGTH(blocks) && blocks[i].signal != signum-SIGRTMIN; i++);

			char shcmd[1024];
			sprintf(shcmd, "%s; kill -%d %d", blocks[i].command, SIGRTMIN+blocks[i].signal, parent);
			char *cmd[] = { "/bin/sh", "-c", shcmd, NULL };
			char button[2] = { '0' + si->si_value.sival_int, '\0' };
			setenv("BUTTON", button, 1);
			setsid();
			execvp(cmd[0], cmd);
			perror(cmd[0]);
			exit(EXIT_SUCCESS);
		}
	} else {
		getsigcmds(signum-SIGPLUS);
		writestatus();
	}
}
#endif

void termhandler(int signum)
{
	statusContinue = 0;
	exit(0);
}

void chldhandler()
{
	while (0 < waitpid(-1, NULL, WNOHANG));
}

int main(int argc, char** argv)
{
	for(int i = 0; i < argc; i++)
	{	
		if (!strcmp("-d",argv[i]))
			delim = argv[++i][0];
		else if(!strcmp("-p",argv[i]))
			writestatus = pstdout;
	}
	signal(SIGTERM, termhandler);
	signal(SIGINT, termhandler);
  signal(SIGCHLD, chldhandler);
	statusloop();
}
