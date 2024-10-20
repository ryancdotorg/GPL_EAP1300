#ifndef _JSON_EXT_API_H_
#define _JSON_EXT_API_H_

#include <api.h>

int ext_getcallback(HTTPEntry packet, ResponseEntry *rep);
int ext_setcallback(HTTPEntry packet, ResponseEntry *rep);

#endif

