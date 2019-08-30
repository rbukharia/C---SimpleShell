#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#define delimiters " \t\n\""        /* input token delimiter */
#define max_args 64                 /* maximum argumen*/
#define max_buffer 1024             /* maximum line buffer */

/*Variabel Global*/

struct termios loud, mute;          /* variables for toggling keyboard echo */
extern char **environ;              /* environment variable */
char cmd[max_buffer];               /* standard input line buffer */    
char cwd[max_buffer];               /* current working directory */
char prompt[max_buffer];            /* shell prompt */
extern int err;                   /* error number */
char *args[max_args];               /* array of argument strings */
/* setelah parsing input line, variabel di bawah ini akan di-set */
char *infilename;                   /* input filename */
char *outfilename;                  /* output filename */
int bg;                        /* 1 jika process dieksekusi di background,
                                        0 jika tidak  */
int appendto_outfile;              /* 1 jika output  di tambahkan ke file,
                                        0 jika output menimpa file  */

/*Header Fungsi dan Prosedur*/


/* print file readme */
void bantuan(void);

/* set input filename saat parsing */
int inputfilename(void);

/* set output filename saat parsing */
int outputfilename(void);

/* set 'manual' variable  environment */
void set_manenv(void);

/* set 'parent' variable environment  */
void set_parenv(void);

/* sets 'shell'  variable environment*/
void set_shellenv(void);

/* changes directory */
int udir(char *dir);

/* gema string */
void gema(char *argv[]);
    
/* execute external program */
void extprog(char *argv[]);
    
/* mengeksekusi command dengan parameter yang diberikan */
void eksekusi(char *argv[]);
    
/* memulai prompt */
void startprompt(void);

/* lists directory */
int listdir(char *dir);

/*membagi command line dan dimasukkan pada array of string, setting input dan output file, amati jika command dieksekusi pada background dan apakah output file dikurangi atau ditambah.*/
int parse(char *argv[], char *cmd);

/* pause shell dengan mematikan keyboard echo */
void stopshell(void);

/* print variable environment */
void printenv(void);

/* mengeset value ke default*/
void parameterreset(void);

/* print  pesan system error */
void systemerror(char *errmsg);
