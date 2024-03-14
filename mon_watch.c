#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdnoreturn.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SYS_ERR 1
#define ARGS_ERR 2
#define EXEC_ERR 4
#define ECHEC_FILS_ERR 8

#define CHK(op) do { if ((op) == -1) raler (1, #op); } while (0)
noreturn void raler (int syserr, const char *msg, ...)
{
    va_list ap;

    va_start (ap, msg);
    vfprintf (stderr, msg, ap);
    fprintf (stderr, "\n");
    va_end (ap);

    if (syserr == 1)
        perror ("");

    exit (EXIT_FAILURE);
}

volatile sig_atomic_t end = 0; //reception de SIGINT
volatile sig_atomic_t alarm_sent = 0;
volatile sig_atomic_t child_end =0;
volatile sig_atomic_t nbr_echec =0;

void alarmhdl (int signo){
    (void) signo;
    alarm_sent = 1;
}
void inthdl (int signo){
    (void) signo;
    end = 1;
}
void chldhdl (int signo){
    (void) signo;
    child_end = 1;
}
void attendre_signal(sigset_t *empty){
	errno = 0;
	sigsuspend(empty);
	if (errno != EINTR) raler(1, "Erreur sigsuspend");

}
void fils ( char *com, char *options[]){
    if((execvp(com, options))==-1){
        fprintf(stderr,"Problème exec\n");
        exit(127);
    }
}
void attendre_fils(){
	int reason;
	CHK(wait(&reason));

    if(WEXITSTATUS(reason) == 127 ){
        exit(EXEC_ERR);
    }
	else if (WIFSIGNALED(reason) || WEXITSTATUS(reason)){
        nbr_echec++;
    } 
    //reinitialisation du nombre d'echecs si exec reussi
    else if (!WEXITSTATUS(reason)){
        nbr_echec = 0;
    }
    
}

int main(int argc, char *argv[]){
    if (argc < 3) {
        fprintf(stderr,"Utilisation : %s <interval> <commande>\n",argv[0]);
        exit(ARGS_ERR);
    }
    char *interval_str=argv[1];
    int interval;
    sscanf(interval_str, "%d", &interval);
    if(interval<=0){
        fprintf(stderr,"INTERVAL should be a strictly positive number\n");
        exit(ARGS_ERR);
    }
    struct sigaction s;
    s.sa_flags = 0;
    s.sa_handler = inthdl;
    CHK (sigemptyset (&s.sa_mask));
    CHK (sigaction (SIGINT, &s, NULL));

    s.sa_handler = alarmhdl;
    CHK (sigemptyset (&s.sa_mask));
    CHK (sigaction (SIGALRM, &s, NULL));

    s.sa_handler = chldhdl;
    CHK (sigemptyset (&s.sa_mask));
    CHK (sigaction (SIGCHLD, &s, NULL));

    sigset_t vide;
    CHK (sigemptyset (&vide));

    sigset_t mask;
    CHK (sigemptyset (&mask));
    CHK (sigaddset (&mask, SIGALRM));
    CHK (sigaddset (&mask, SIGKILL));
    CHK (sigaddset (&mask, SIGCHLD));

    sigset_t mask2;
    CHK (sigemptyset (&mask2));
    CHK (sigaddset (&mask2, SIGALRM));
    CHK (sigaddset (&mask2, SIGINT));
    CHK (sigaddset (&mask2, SIGCHLD));

    sigset_t mask3;
    CHK (sigemptyset (&mask3));
    CHK (sigaddset (&mask3, SIGALRM));
    CHK (sigaddset (&mask3, SIGINT));

    sigset_t mask4;
    CHK (sigemptyset (&mask4));
    CHK (sigaddset (&mask4, SIGCHLD));

    CHK (sigprocmask (SIG_BLOCK, &mask, NULL));
    pid_t pid;
    while(!end){
        switch (pid = fork ()) {
        case -1:
            raler (1, "fork");
            break;
        case 0:
            fils(argv[2], argv+2);
            break;
        }
        alarm(interval);
        CHK (sigprocmask (SIG_BLOCK, &mask2, NULL));
        if(!child_end || !alarm_sent || !end){
            attendre_signal(&vide);
        }
        CHK (sigprocmask (SIG_UNBLOCK, &mask2, NULL));

        if(child_end){
            attendre_fils();
              if(nbr_echec == 3){
                fprintf(stderr,"Echecs repetes\n");
                exit(ECHEC_FILS_ERR);
            } 
            CHK (sigprocmask (SIG_BLOCK, &mask3, NULL));
            if(!alarm_sent || !end){
                attendre_signal(&vide);
            }
            CHK (sigprocmask (SIG_UNBLOCK, &mask3, NULL));

        }
        else if(alarm_sent || end){
            CHK (kill (pid, SIGKILL));
            CHK (sigprocmask (SIG_BLOCK, &mask4, NULL));
            if(!child_end){
                attendre_signal(&vide);
            }
            CHK (sigprocmask (SIG_UNBLOCK, &mask4, NULL));
            attendre_fils();
              if(nbr_echec == 3){
                fprintf(stderr,"Echecs repetes\n");
                exit(ECHEC_FILS_ERR);
            } 
        }
        alarm_sent = 0;
        child_end = 0;
    }
    CHK (sigprocmask (SIG_UNBLOCK, &mask, NULL));
    exit(0);
}


