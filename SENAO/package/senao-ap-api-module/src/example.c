#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcgiapp.h>

// color define
#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define LIGHT_RED "\033[1;31m"
#define GREEN "\033[0;32;32m"
#define LIGHT_GREEN "\033[1;32m"
#define BLUE "\033[0;32;34m"
#define LIGHT_BLUE "\033[1;34m"
#define DARY_GRAY "\033[1;30m"
#define CYAN "\033[0;36m"
#define LIGHT_CYAN "\033[1;36m"
#define PURPLE "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN "\033[0;33m"
#define YELLOW "\033[1;33m"
#define LIGHT_GRAY "\033[0;37m"
#define WHITE "\033[1;37m"
// color define


#define debug_printf(x, ...) do { \
    FILE *fp = fopen("/dev/console", "a"); \
    if (fp) { \
        fprintf(fp, LIGHT_RED"[api][%d] %s:%d "x"\n"NONE, getpid(),__func__, __LINE__, ##__VA_ARGS__ ); \
        fclose(fp); \
    } \
} while(0)


#define LISTENSOCK_FILENO 0
#define LISTENSOCK_FLAGS 0


int main(int argc, char** argv) {
  int err = FCGX_Init(); /* call before Accept in multithreaded apps */
  if (err) { debug_printf ("FCGX_Init failed: %d", err); return 1; }
  FCGX_Request cgi;
  err = FCGX_InitRequest(&cgi, LISTENSOCK_FILENO, LISTENSOCK_FLAGS);
  if (err) { debug_printf ("FCGX_InitRequest failed: %d", err); return 2; }

  while (1) {
    err = FCGX_Accept_r(&cgi);
    if (err) { debug_printf ("FCGX_Accept_r stopped: %d", err); break; }

    char** envp;
    int size = 200;
    for (envp = cgi.envp; *envp; ++envp) size += strlen(*envp) + 11;
    char*  result = (char*) malloc(size);

    strcpy(result, "Status: 200 OK\r\nContent-Type: text/html\r\n\r\n");
    strcat(result, "<html><head><title>testcgi</title></head><body><ul>\r\n");

    for (envp = cgi.envp; *envp; ++envp) {
      strcat(result, "<li>");
      strcat(result, *envp);
      strcat(result, "</li>\r\n");
    }

    strcat(result, "</ul></body></html>\r\n");
    FCGX_PutStr(result, strlen(result), cgi.out);
    free(result);
  }

  return 0;
}
