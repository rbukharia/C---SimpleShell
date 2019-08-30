#include "if02shell12.h"

/* Main program. */
int main(int argc, char *argv[]) {
    int curr_linenum;                   /* parsing current line number dari file */
    FILE *file;                         /* batch file*/
    getcwd(cwd, max_buffer);            /* set current working directory */
    parameterreset();                     /* set initial parameters */
    set_manenv();                   /* set 'manual' environment variable */
    set_shellenv();                    /* set 'shell' environment variable */
    startprompt();                       /* memulai prompt */
    
    /* start shell jika tidak ada command yang diberikan */
    if (argc == 1) {
        while (!feof(stdin)) {
            fputs(prompt, stdout);                      /* write prompt */

            if (fgets(cmd, max_buffer, stdin)) {        /* membaca line */
                /* jika parsing error maka baca dari file dihentikan dan terminate */
                if (parse(args, cmd)) {
                    fprintf(stderr, "if02shell12: syntax error\n");
                    parameterreset();
                    continue;
                }
                /* jika tidak, eksekusi command */
                else {
                    eksekusi(args);
                    parameterreset();
                }
            }
        }
    }
    /* jika ada command line, baca line dari file*/
    else if (argc == 2) {
        curr_linenum = 1;
        file = freopen(argv[1], "r", stdin);       /* open file untuk dibaca */

        /* baca dan eksekusi setiap line sampai EOF */
        while (fgets(cmd, max_buffer, file)) {
            /* jika parsing error maka baca dari file dihentikan dan terminate */
            if (parse(args, cmd)) {
                fprintf(stderr, "if02shell12: syntax error on line %d of %s\n", 
                    curr_linenum, argv[1]);
                parameterreset();
                break;
            }
            /* jika tidak, eksekusi command */
            else {
                eksekusi(args);
                parameterreset();
            }
            ++curr_linenum;
        }
    }
    /* terlalu banyak command line yang diberikan; exit program */
    else {
        fprintf(stderr, "Format options:\n- if02shell12\n- if02shell12 <file>\n");
        return 0;
    }
  
    return 0;
}


