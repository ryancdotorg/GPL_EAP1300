#ifndef _SNLOG_PORTAL_H
#define _SNLOG_PORTAL_H

#define TOSTR(x) #x
// known issue, avoid snlogd restart, need re opensnlog, so open/close snlogd here
#define CLOUD_EVENT(prio, name, data, ...) do {\
    vsnlog("event." TOSTR(prio) " 1.0", name "," data, ##__VA_ARGS__); \
} while(0)

#define SNLOG_USER_LOGIN_EVENT        "ap_portal_user_login"
#define SNLOG_USER_LOGOUT_EVENT       "ap_portal_user_logout"
#define SNLOG_USER_ROAMING_EVENT      "ap_portal_user_roaming"

#endif /*_CHILLI_H */
