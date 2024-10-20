/*******************************************************************************************
 * Copyright (c) 2018 Ryan Hsu @ SenaoNetworks - Taiwan                                    *
 *                                                                                         *
 * This code has to be used in SenaoNetworks internally                                    *
 * Unauthorized copying of this file, via any medium is strictly prohibite                 *
 * Proprietary and confidential                                                            *
 * --------------------------------------------------------------------------------------- *
 * Project:  syseye                                                                        *
 *                                                                                         *
 * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
 *******************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/prctl.h>
#include "action.h"

int file_exist(char *fname)
{
    if( access( fname, F_OK ) != -1 ) {
        return 1;
    } else {
        return 0;
    }
}

int write_to_fname(char *fname, const char *fi, int l, const char *fn, char *str) {
    int fd;
    unsigned int pid;
    char out[1024];
    int len = 0;
    va_list args;
    char *tmp;
    if ((fd = open(fname, O_WRONLY | O_CREAT | O_APPEND)) != -1){
      pid = syscall(SYS_gettid);
      memset(out, 0, sizeof(out));
      snprintf(out, sizeof(out), "\033[1m\033[40;%dm[%5d %-10s<%5d>%10s()]\033[0m ", 30+(pid%6)+1, pid, fi, l, fn);
      len += write(fd, out, strlen(out));
      len += write(fd, str, strlen(str));
      close(fd);
    }
    return len;
}

/* Read a full `count' bytes from fd, unless end-of-file or an error
 * other than EINTR is encountered. copied from ppp source.
 */
size_t fd_read(int fd, void *buf, size_t count)
{
    size_t bytes_read;
    char *ptr = buf;
    for (bytes_read = 0; bytes_read < count; ) {
        size_t nb = read(fd, ptr, count - bytes_read);
        if (nb < 0) {
            if (errno == EINTR)
                continue;
                return -1;
            }
            if (nb == 0)
                break;
            bytes_read += nb;
            ptr += nb;
        }
        return bytes_read;
}

pid_t safe_fork(void)
{
    pid_t pid;
    int pipefd[2];
    char buf[1];
    int sig;
    if (pipe(pipefd) == -1) {
        pipefd[0] = pipefd[1] = -1;
    }
    pid = fork();
    if (pid < 0) {
        perror("fork fault:");	    
        return errno;
    }
    if (pid > 0) {
        /* parent... */
        close(pipefd[1]);
        /* this read() blocks until the close(pipefd[1]) below */
        fd_read(pipefd[0], buf, 1);
        close(pipefd[0]);
        return pid;
    }
    /* child... */
    close(pipefd[0]);
    /* this close unblocks the read() call above in the parent */
    close(pipefd[1]);

    /* Reset signal handlers set for parent process */
    for (sig = 0; sig < (_NSIG-1); sig++)
        signal(sig, SIG_DFL);
    /* Lost controlling terminal */
    ioctl(0, TIOCNOTTY, 0);
    close(STDIN_FILENO);
    setsid();
    return pid;
}

pid_t safe_fork_fn(void *(*fn) (void *), void *args)
{
    struct action_args_t *act_args = (struct action_args_t *)args;
    pid_t pid = safe_fork();
    if (pid !=0)
        return pid;
    // child ...
    prctl(PR_SET_NAME, (unsigned long) act_args->cmd, 0, 0, 0);
    fn(args);
    exit(EXIT_SUCCESS);
    return pid;
}

/* from Robbins & Robbins: Unix Systems Programming p.37*/
int makeargv(const char *s, const char *delimiters, char ***argvp) {
	int error;
	int i;
	int numtokens;
	const char *snew;
	char *t;
	char *saveptr;

	if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
		errno = EINVAL;
		return -1;
	}
	*argvp = NULL;
	snew = s + strspn(s, delimiters);         /* snew is real start of string */
	if ((t = malloc(strlen(snew) + 1)) == NULL)
		return -1;
	strcpy(t, snew);
	numtokens = 0;
	if (strtok_r(t, delimiters, &saveptr) != NULL)     /* count the number of tokens in s */
		for (numtokens = 1; strtok_r(NULL, delimiters, &saveptr) != NULL; numtokens++) ;

	/* create argument array for ptrs to the tokens */
	if ((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL) {
		error = errno;
		free(t);
		errno = error;
		return -1;
	}
	/* insert pointers to tokens into the argument array */
	if (numtokens == 0)
		free(t);
	else {
		strcpy(t, snew);
		**argvp = strtok_r(t, delimiters, &saveptr);
		for (i = 1; i < numtokens; i++)
			*((*argvp) + i) = strtok_r(NULL, delimiters, &saveptr);
	}
	*((*argvp) + numtokens) = NULL;             /* put in final NULL pointer */
	return numtokens;
}

void freemakeargv(char **argv)
{
	if (argv == NULL)
		return;
	if (*argv != NULL)
		free(*argv);
	free(argv);
}

void hexDump(char *desc, void *addr, int len)
{
	int i;
	unsigned char buff[17];
	unsigned char *pc = (unsigned char*)addr;

	// Output description if given.
	if (desc != NULL)
		printf ("%s:\n", desc);

	// Process every byte in the data.
	for (i = 0; i < len; i++) {
		// Multiple of 16 means new line (with line offset).

		if ((i % 16) == 0) {
			// Just don't print ASCII for the zeroth line.
			if (i != 0)
				printf("  %s\n", buff);

			// Output the offset.
			printf("  %04x ", i);
		}

		// Now the hex code for the specific character.
		printf(" %02x", pc[i]);

		// And store a printable ASCII character for later.
		if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
			buff[i % 16] = '.';
		} else {
			buff[i % 16] = pc[i];
		}

		buff[(i % 16) + 1] = '\0';
	}

	// Pad out last line if not exactly 16 characters.
	while ((i % 16) != 0) {
		printf("   ");
		i++;
	}

	// And print the final ASCII bit.
	printf("  %s\n", buff);
}
