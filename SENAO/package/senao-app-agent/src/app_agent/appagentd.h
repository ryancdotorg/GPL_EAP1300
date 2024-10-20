#if __cplusplus
extern "C" {
#endif

#ifndef _APPAGENTD_H_
#define _APPAGENTD_H_

#include "ostypes.h"

INT32	httpd_Start();
INT32   httpd_AddListenPort(UINT16 port);

#endif

#if __cplusplus
}
#endif

