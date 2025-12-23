//#ifdef notdef

/***************************************************************************
| File: monitor.c
|
| Autor: Carlos Almeida (IST), from work by Jose Rufino (IST/INESC), 
|        from an original by Leendert Van Doorn
| Data:  Nov 2002
***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "mbed.h"
#include "mytasks.h"


/*-------------------------------------------------------------------------+
| Headers of command functions
+--------------------------------------------------------------------------*/
void        cmd_sos(int, char **);
extern void cmd_test(int, char **);
extern void cmd_rc(int, char **);
extern void cmd_sd(int, char **);
extern void cmd_rdt(int, char **);
extern void cmd_sc(int, char **);
extern void cmd_rt(int, char **);
extern void cmd_rmm(int, char **);
extern void cmd_cmm(int, char **);
extern void cmd_rp(int, char **);
extern void cmd_mmp(int, char **);
extern void cmd_rai(int, char **);
extern void cmd_sac(int, char **);
extern void cmd_sat(int, char **);
extern void cmd_adac(int, char **);
extern void cmd_adat(int, char **);
extern void cmd_rts(int, char **);
extern void cmd_adbl(int, char **);
extern void cmd_adhb(int, char **);
extern void cmd_adcs(int, char **);

/*-------------------------------------------------------------------------+
| Variable and constants definition
+--------------------------------------------------------------------------*/ 
const char TitleMsg[] = "\n Application Control Monitor\n";
const char InvalMsg[] = "\nInvalid command!";

struct  command_d {
  void  (*cmd_fnct)(int, char**);
  char* cmd_name;
  char* cmd_help;
} const commands[] = {
  {cmd_sos,     "sos","                     help"},
  {cmd_test,    "test"," <arg1> ... <argn>  test command"},
  {cmd_rdt,     "rdt","                     read date/time (dd/MM/YYYY hh:mm:ss)"},
  {cmd_sd,      "sd","   d m y              set date (day, month, year)"},
  {cmd_rc,      "rc","                      read clock"},
  {cmd_sc,      "sc","   h m s              set clock (hours, minutes, seconds)"},
  {cmd_rt,      "rt","                      read temperature"},
  {cmd_rmm,     "rmm","                     read maximum and minimum of temperature"},
  {cmd_cmm,     "cmm","                     clear maximum and minimum of temperature"},
  {cmd_rp,      "rp","                      read parameters (pmon, tala)"},
  {cmd_mmp,     "mmp","  p                  modify monitoring period"},
  {cmd_rai,     "rai","                     read alarm info (alarm clock, tlow, thigh, active/inactive)"},
  {cmd_sac,     "sac","  h m s              set alarm clock (hours, minutes, seconds)"},
  {cmd_sat,     "sat","  tl th              set alarm temperature thresholds (tlow, thigh)"},
  {cmd_adac,    "adac"," 1/0                activate/deactivate alarm clock"},
  {cmd_adat,    "adat"," 1/0                activate/deactivate alarm temperature"},
  {cmd_rts,     "rts","                     read task state (Bubble Level, Hit Bit, Config Sound)"},
  {cmd_adbl,    "adbl"," 1/0                activate/deactivate Bubble Level task"},
  {cmd_adhb,    "adht"," 1/0                activate/deactivate Hit Bit task"},
  {cmd_adcs,    "adcs"," 1/0                activate/deactivate Config Sound operation"}
};

#define NCOMMANDS  (sizeof(commands)/sizeof(struct command_d))
#define ARGVECSIZE 4
#define MAX_LINE   50

/*-------------------------------------------------------------------------+
| Function: my_fgets        (called from my_getline / monitor) 
+--------------------------------------------------------------------------*/ 
char* my_fgets (char* ln, int sz, FILE* f, Serial *pc)
{
//  fgets(line, MAX_LINE, stdin);
//  pc.gets(line, MAX_LINE);
  int i; char c;
  for(i=0; i<sz-1; i++) {
      c = pc->getc();
      ln[i] = c;
      if ((c == '\n') || (c == '\r')) break;
  }
  ln[i] = '\0';

  return ln;
}

/*-------------------------------------------------------------------------+
| Function: cmd_sos - provides a rudimentary help
+--------------------------------------------------------------------------*/ 
void cmd_sos (int argc, char **argv)
{
  int i;

  printf("%s\n", TitleMsg);
  for (i=0; i<NCOMMANDS; i++)
    printf("%s %s\n", commands[i].cmd_name, commands[i].cmd_help);
}

/*-------------------------------------------------------------------------+
| Function: my_getline        (called from monitor) 
+--------------------------------------------------------------------------*/ 
int my_getline (char** argv, int argvsize, Serial *pc)
{
  static char line[MAX_LINE];
  char *p;
  int argc;

//  fgets(line, MAX_LINE, stdin);
  my_fgets(line, MAX_LINE, stdin, pc);

  /* Break command line into an o.s. like argument vector,
     i.e. compliant with the (int argc, char **argv) specification --------*/

  for (argc=0,p=line; (*line != '\0') && (argc < argvsize); p=NULL,argc++) {
    p = strtok(p, " \t\n");
    argv[argc] = p;
    if (p == NULL) return argc;
  }
  argv[argc] = p;
  return argc;
}

/*-------------------------------------------------------------------------+
| Function: monitor        (called from main) 
+--------------------------------------------------------------------------*/ 
void monitor (Serial *pc)
{
  static char *argv[ARGVECSIZE+1], *p;
  int argc, i;

  printf("%s Type sos for help\n", TitleMsg);
  for (;;) {
    printf("\nCmd> ");
    /* Reading and parsing command line  ----------------------------------*/
    if ((argc = my_getline(argv, ARGVECSIZE, pc)) > 0) {
        xEventGroupClearBits(xAlarmEvents, ALARM_ACTIVE);
        for (p=argv[0]; *p != '\0'; *p=tolower(*p), p++);
        for (i = 0; i < NCOMMANDS; i++) 
    if (strcmp(argv[0], commands[i].cmd_name) == 0) 
      break;
      /* Executing commands -----------------------------------------------*/
      if (i < NCOMMANDS)
    commands[i].cmd_fnct (argc, argv);
      else  
    printf("%s", InvalMsg);
    } /* if my_getline */
  } /* forever */
}

//#endif //notdef
