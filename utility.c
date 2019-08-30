#include "if02shell12.h"


/*
	mengeksekusi external program pada child process itu sendiri
	Argument:
    - argv: arguments
*/
void extprog(char *argv[]) {
    pid_t pid;                      /* child process ID */
    int stat_loc;                   /* me-return nilai dari forking child */
    FILE *infile;                   /* input file, jika input redirected */
    FILE *outfile;                  /* output file, jika output redirected */
    
    switch(pid = fork()) {
        case -1:    /* forking gagal, exit process */
            systemerror("\nif02shell12: fork gagal");
        case 0:     /* forking sukses, mengeksekusi child process */
            /* jika input file ditentukan maka redirect input from it */
            if (infilename != NULL) {
                infile = freopen(infilename, "r", stdin);
            }
        
            /* jika output file ditentukan maka redirect output to it */
            if (outfilename != NULL) {
                if (appendto_outfile)
                    outfile = freopen(outfilename, "a", stdout);
                else
                    outfile = freopen(outfilename, "w", stdout);
            }
        
            execvp(argv[0], argv);
            exit(0);                    /* keluar dari child process */
        default:    /* parent process */
            /*  
				jika child bukan background process, maka tunggu sampai child selesai sebelum melanjutkan parent */
            if (!bg) {
                waitpid(pid, &stat_loc, WUNTRACED);
            }
    }
}

void eksekusi(char *argv[]) {
    /* tidak mengeksekusi apapun */
    if (argv[0] == NULL) {
        
    }
    /* ubah directory ke <directory> */
    else if (strcmp(argv[0], "ud") == 0) {
        udir(argv[1]);
    }
    /* clear screen */
    else if (strcmp(argv[0], "bersih") == 0) {
        system("clear");
    }
    /* daftar isi <directory> */
    else if (strcmp(argv[0], "dir") == 0) {
        listdir(argv[1]);
    }
    /* list semua environment variables */
    else if (strcmp(argv[0], "skrt") == 0) {
        printenv();
    }
    /* tampilkan <argument> */
    else if (strcmp(argv[0], "gema") == 0) {
        gema(argv);
    }
    /* tampilkan user manual */
    else if (strcmp(argv[0], "bantuan") == 0) {
        bantuan();
    }
    /* stop operasi shell sampai Enter sudah di-klik */
    else if (strcmp(argv[0], "stop") == 0) {
        stopshell();
    }
    /* quit shell */
    else if (strcmp(argv[0], "keluar") == 0) {
        exit(0);
    }
    /* external commands */
    else {
	fprintf(stderr, "perintah tidak ditemukan...\n");
        //extprog(argv);
    }
}


/*
    mengubah ke direktori yg sudah ditentukan
    Argumen: 
    - dir: directory
    Returns: 0 jika sukses, -1 jika gagal
*/
int udir(char *dir) {
    int dir_exists;             /* 1 jika <directory> ada dan 0 jika tidak ada */

    /* <directory> tidak ada, menampilkan directory saat ini */
    if (dir == NULL) {
        getcwd(cwd, max_buffer);
        fprintf(stdout, "%s\n", cwd);
        return 0;
    }
    /* argumen <directory> ada */
    else {
        dir_exists = chdir(dir);    /* merubah menjadi <directory> */

        /* perubahan ke <directory> gagal (dir tidak ada), tampilkan error */
        if (dir_exists == -1) {
            fprintf(stderr, "%s: ud: %s: tidak ada file atau directory\n",
                "if02shell12", dir);
            return -1;
        }
        /* perubahan ke <directory> sukses (dir ada) */
        else {
          /* update PWD environment variable menjadi new cwd */
          getcwd(cwd, max_buffer);
          setenv("PWD", cwd, 1);    /* 1 berarti meng-overwrite nilai PWD yang telah ada */

          /* mereconstruksi prompt dengan new cwd */
          startprompt();
          return 0;
        }
    }
}



/**
	cetak semua strings setelah gema command ke standart output
	Argumen:
    - argv: array of command line arguments
*/
void gema(char *argv[]) {
    pid_t pid;                      /* child process ID */
    int stat_loc;                   /* me-return value dari forking child */
    FILE *outfile;                  /* output file, jika output redirected */
    char **ptargv;
    
    switch(pid = fork()) {
        case -1:    /* forking gagal, exit proses */
            systemerror("\nif02shell12: fork gagal");
        case 0:     /* forking sukses, jalankan child process */
            /* jika output file ditentukan meredirect output to it */
            if (outfilename != NULL) {
                if (appendto_outfile)
                    outfile = freopen(outfilename, "a", stdout);
                else
                    outfile = freopen(outfilename, "w", stdout);
            }

            /* print argv[1] ,argv[0] adalah perintah jadi tidak di-print*/  
            ptargv = argv;
            ++ptargv;
            if (*ptargv != NULL) fprintf(stdout, "%s", *ptargv++);

            /* print argv[2] to argv[n-1] */
            while (*ptargv != NULL) {
                fprintf(stdout, " %s", *ptargv);
                ++ptargv;
            }

            fprintf(stdout, "\n");
            exit(0);                    /* keluar child process */
        default:
            /*	jika child bukan background process, maka tunggu sampai child selesai sebelum melanjutkan parent */
            if (!bg) {
                waitpid(pid, &stat_loc, WUNTRACED);
            }
    }
}





/**	stop operasi shell sampai Enter sudah di-klik
    fungsi ini mematikan semua keyboard gema sampai shell udah tidak di stop
	
*/ 
void stopshell(void) {
    fprintf(stdout, "Tekan Enter untuk melanjutkan...");
    
    /* mematikan semua keyboard gema */
    tcgetattr(fileno(stdout), &loud);
    mute = loud;
    mute.c_lflag &= ~ECHO;
    tcsetattr(fileno(stdout), TCSAFLUSH, &mute);
    
    /*  nyalakan shell jika sudah di-klik Enter */
    while(getchar() != '\n');
    fprintf(stdout, "\n");
    
    /* menghidupkan keyboard gema */
    tcsetattr(fileno(stdout), TCSAFLUSH, &loud);
}



/*
     prompt.
*/
void startprompt(void) {
    system("clear");
    prompt[0] = '\0';                 /* prompt kosong */
    getcwd(cwd, max_buffer);          /* get current working directory (CWD)*/
    
    /* membuat prompt */
    strcat(prompt, "if02shell12");      
    strcat(prompt, "> ");
}




/**
	membagi command line dan masukkan pada array of string, setting input dan output file, 
	amati jika command dieksekusi pada background dan apakah output file dikurangi atau ditambah.
		argumen:
		- argv : array of string
		- cmd : command untuk parsing
	return 0 jika parsing tidak menghasilkan error, -1 jika tidak ada parsing atau sintaks error terdeteksi selama parsing
*/

int parse(char *argv[], char *cmd) {
    int argc;
    char **ptargv;
    
    if (cmd == NULL) {
        /* tak ada yang diparsing */
        return -1;
    }
    /* simpan hasil kedalam argv */
    else {
        ptargv = argv;
        *ptargv = strtok(cmd, delimiters);        /* extract token pertama */
        ++ptargv;
        argc = 1;                       /* inisialisasi jumlah track argumen cmd */
        
        /* extract token yang tersisa */
        while ((*ptargv = strtok(NULL, delimiters)) != NULL) {
            ++argc;
        
            /* jika token < maka ekstrak input filename  */
            if (strcmp(*ptargv, "<") == 0) {
                *ptargv = NULL;
                
                /* jika tak ada input filename terspesifikasi maka return error*/
                if (inputfilename() == -1) return -1;
            }
            /* jika token <, maka ekstrak output filename*/
            else if (strcmp(*ptargv, ">") == 0) {
                *ptargv = NULL;
                
                /* jika tak ada input filename terspesifikasi maka return error */
                if (outputfilename() == -1) return -1;
            }
            /* jika token >>, maka extract output filename dan set untuk menyisipkan otput ke output */
            else if (strcmp(*ptargv, ">>") == 0) {
                appendto_outfile = 1;
                *ptargv = NULL;
                
                /* jika tak ada output filename terspesifikasi maka return error */
                if (outputfilename() == -1) return -1;
            }
            /* jika tandanya &, maka set proses untuk dieksekusi di background. tanda setelah & diabaikan */
            else if (strcmp(*ptargv, "&") == 0) {
                bg = 1;
                *ptargv = NULL;
                return 0;
            }
            /*  jika maksimum argumen yang diperbolehkan diperoleh,stop parsing*/
            else if (argc >= max_args) {
                return 0;
            }
            
            else {
                ++ptargv;
            }            
        }
        
        return 0;
    }
}



/*
	
	laporkan semua file/folder kedalam directory.
	Argument:
	- dir: directory
    Returns: 0 jika sukses, -1 jika tidak
*/
int listdir(char *dir) {
    pid_t pid;                      /* proses child ID */
    int stat_loc;                   /* nilai balik dari percabangan child*/
    FILE *outfile;                  /* output file, jika output terhubung*/
    
    /* jika dir tidak dispesifikasikan, report error */
    if (dir == NULL) {
        fprintf(stderr, "format: dir <directory>\n");
        return -1;
    }
    /* if dir dispesifikasikan, list contentnya */
    else {
        switch(pid = fork()) {
            case -1:    /* forking gagal, exit process */
                systemerror("\nif02shell12: fork gagal");
            case 0:     /* forking sukses, eksekusi proses child*/
                set_parenv();       /* set 'parent' environment variable */
                
                /* jika output file dispesifikasikan maka, hubungkan dengan output */
                if (outfilename != NULL) {
                    if (appendto_outfile)
                        outfile = freopen(outfilename, "a", stdout);
                    else
                        outfile = freopen(outfilename, "w", stdout);
                }
                
                execlp("ls", "ls", "-al", dir, NULL);     /* list directory */
            default:
                /*  
					jika child bukan merupakan background proses,tunggu sampai child selesai sebelum melanjutkan ke parent
				*/
                if (!bg) {
                    waitpid(pid, &stat_loc, WUNTRACED);
                }
        }
        
        return 0;
    }
}





/**
    print semua evirontment variable ke standar output
*/
void printenv(void) {
    pid_t pid;                  /* proses child ID */
    int stat_loc;               /* nilai balik dari percabangan child */
    FILE *outfile;              /* output file, jika terhubung */
    char **env;                 /* pointer to array of environment variables */

    switch(pid = fork()) {
        case -1:    /* forking gagal, exit process */
            systemerror("\nif02shell12: fork gagal");
        case 0:     /* forking sukses, eksekusi child process */
            set_parenv();       /* set 'parent' environment variable */
            env = environ;
            
            /* jika output file dispesifikasikan maka hubungkan dengan output */
            if (outfilename != NULL) {
                if (appendto_outfile)
                    outfile = freopen(outfilename, "a", stdout);
                else
                    outfile = freopen(outfilename, "w", stdout);
            }
            
            /* print environment variable */
            while (*env != '\0') {
                fprintf(stdout, "%s\n", *env);
                ++env;
            }
            
            exit(0);                    /* exit child process */
        default:
            /* jika child bukan background process, maka tunggu sampai child selesai sebelum melanjutkan parent */
            if (!bg) {
                waitpid(pid, &stat_loc, WUNTRACED);
            }
    }
}




/*
    Print manual.
*/
void bantuan(void) {
    pid_t pid;                  /* proses child ID */
    int stat_loc;               /* nilai balik dari percabangan child */
    FILE *outfile;              /* output file, jika terhubung*/
    char **env;                 /* pointer to array of environment variables */

    switch(pid = fork()) {
        case -1:    /* forking gagal, exit process */
            systemerror("\nif02shell12: fork gagal");
        case 0:     /* forking sukses, eksekusi child process */
            set_parenv();       /* set 'parent' environment variable */
            env = environ;
            
            /* jika output file dispesifikasikan maka hubungkan dengan output */
            if (outfilename != NULL) {
                if (appendto_outfile)
                    outfile = freopen(outfilename, "a", stdout);
                else
                    outfile = freopen(outfilename, "w", stdout);
            }
            
            execlp("more", "more", getenv("readme"), NULL);
            exit(0);                    /* exit child process */
        default:
            /* jika child bukan background process, maka tunggu sampai child selesai sebelum melanjutkan parent */
            if (!bg) {
                waitpid(pid, &stat_loc, WUNTRACED);
            }
    }
}



/*
    Sets output filename jika dispesifikasikan.
    Returns: 0 jika filename diberikan, -1 jika tidak
*/
int outputfilename(void) {
    if ((outfilename = strtok(NULL, delimiters)) == NULL) {
        return -1;
    }
    
    return 0;
}


/**
    Set 'manual' environment variable.
*/
void set_manenv(void) {
    char *manual_env;
    
    manual_env = malloc((strlen(cwd) + 8) * sizeof(char));

    /* jika malloc gagal, print error dan exit function */
    if (manual_env == NULL) {
        fprintf(stderr, "memory tidak mencukupi!\n");
        return;
    }
    
    manual_env[0] = '\0';
    
    /* manual = "<current working directory>/readme" */
    strcat(manual_env, cwd);
    strcat(manual_env, "/");
    strcat(manual_env, "readme");
    setenv("readme", manual_env, 1);         /* 1 berarti overwrite */

    /* free memory dan assign NULL untuk mencegah double free attack  */
    free(manual_env);
    manual_env = NULL;
}

/*
    Set 'parent' environment variable.
*/
void set_parenv(void) {
    setenv("parent", getenv("shell"), 1);   /* 1 berarti overwrite */
}


/*
    Sets input filename jika dispesifikasikan.
    Returns: 0 jika filename diberikan, -1 jika tidak
*/

int inputfilename(void) {
    if ((infilename = strtok(NULL, delimiters)) == NULL) {
        return -1;
    }
    
    return 0;
}




/*
    Set 'shell' environment variable.
*/
void set_shellenv(void) {
    char *shell_env;

    shell_env = malloc((strlen(cwd) + strlen("if02shell12") + 2) *
        sizeof(char));

    /* jika malloc gagal, print error dan exit function */
    if (shell_env == NULL) {
        fprintf(stderr, "memory tidak mencukupi!\n");
        return;
    }
        
    shell_env[0] = '\0';
    
    /* shell = "<current working directory>/SHELL_NAME" */
    strcat(shell_env, cwd);
    strcat(shell_env, "/");
    strcat(shell_env, "if02shell12");
    setenv("shell", shell_env, 1);          /* 1 berarti overwrite */

    /* free memory dan assign NULL untuk mencegah double free attack */
    free(shell_env);
    shell_env = NULL;
}


/*
    Print error message dan exit process.
    Argument:
    - errmsg: error message
*/
void systemerror(char *errmsg) {
    fprintf(stderr, "%s: %s", strerror(err), errmsg);
    exit(-1);
}

/*
    Reset nilai parameter yang digunakan untuk eksekusi shell command ke default.
*/
void parameterreset(void) {
    outfilename= NULL;
    bg = 0;
    appendto_outfile = 0;
    infilename = NULL;
    
}

