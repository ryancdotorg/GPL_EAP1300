/* -*- mode: c; c-basic-offset: 2 -*- */
/*
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "chilli.h"

static char * kname_fmt = "/proc/net/coova/%s";

static int
kmodmac(char cmd, struct in_addr *addr, uint8_t *mac) {
  char file[128];
  char line[256];
  int fd, rd;

  if (!_options.kname) return -1;
  safe_snprintf(file, sizeof(file), kname_fmt, _options.kname);
  fd = open(file, O_RDWR, 0);
  if (fd > 0) {
    if (addr)
        if(mac) {
            safe_snprintf(line, sizeof(line), "%c%02x-%02x-%02x-%02x-%02x-%02x@%s\n",
                    cmd, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], inet_ntoa(*addr));
        }
        else {
            safe_snprintf(line, sizeof(line), "%c%s\n", cmd, inet_ntoa(*addr));
        }
    else
      safe_snprintf(line, sizeof(line), "%c\n", cmd);

    rd = safe_write(fd, line, strlen(line));
    log_dbg("kmod wrote %d %s", rd, line);
    close(fd);
    return rd == strlen(line);
  } else {
    log_err(errno, "could not open %s", file);
  }
  return 0;
}

static int
kmod(char cmd, struct in_addr *addr) {
    return kmodmac(cmd, addr, NULL);
}

int
kmod_coova_reset(struct app_conn_t *appconn) {
    if(appconn->hisip.s_addr != 0)
    {
        return kmodmac('%', &appconn->hisip, appconn->hismac);
    }
    else
    {
        return 1;
    }
}

int
kmod_coova_update(struct app_conn_t *appconn) {
    if(appconn->hisip.s_addr != 0)
    {
        return kmodmac(appconn->s_state.authenticated ? '+' : '-',
                &appconn->hisip, appconn->hismac);
    }
    else
    {
        return 1;
    }
}

int
kmod_coova_release(struct dhcp_conn_t *conn) {
  return kmod('*', &conn->hisip);
}

int
kmod_coova_clear() {
  return kmod('/', 0);
}

int
kmod_coova_sync() {
  char file[128];
  char * line = 0;
  size_t len = 0;
  ssize_t read;
  FILE *fp;

  char ip[256];
  unsigned int maci[6];
  unsigned int state;
  unsigned long long int bin;
  unsigned long long int bout;
  unsigned long long int pin;
  unsigned long long int pout;
  struct dhcp_conn_t *conn;

  if (!_options.kname) return -1;

  safe_snprintf(file, sizeof(file), kname_fmt, _options.kname);

  fp = fopen(file, "r");
  if (fp == NULL)
    return -1;

  while ((read = getline(&line, &len, fp)) != -1) {
    if (len > 256) {
      log_err(errno, "problem");
      continue;
    }

    if (sscanf(line,
         "mac=%X-%X-%X-%X-%X-%X "
         "src=%s state=%u "
         "bin=%llu bout=%llu "
         "pin=%llu pout=%llu",
         &maci[0], &maci[1], &maci[2], &maci[3], &maci[4], &maci[5],
         ip, &state, &bin, &bout, &pin, &pout) == 12) {
      uint8_t mac[6];
      int i;
      int update;

      for (i=0;i<6;i++)
        mac[i]=maci[i]&0xFF;

        if (!dhcp_hashget(dhcp, &conn, mac)) {
          struct app_conn_t *appconn = conn->peer;
          if (appconn) {

            // new bridge mode cannot update idletime
            if(_options.bridgemode)
            {
                if (_options.swapoctets) {
                    if( appconn->s_state.input_octets != bin ||
                            appconn->s_state.output_octets != bout ||
                            appconn->s_state.input_packets != pin ||
                            appconn->s_state.output_packets != pout)
                    {
                        update = 1;
                    }
                    else
                    {
                        update = 0;
                    }
                } else {
                    if(appconn->s_state.output_octets != bin ||
                            appconn->s_state.input_octets != bout ||
                            appconn->s_state.output_packets != pin ||
                            appconn->s_state.input_packets != pout)
                    {
                        update = 1;
                    }
                    else
                    {
                        update = 0;
                    }
                }

                if(update)
                {
                    appconn->s_state.last_sent_time = mainclock_now();
                    appconn->s_state.last_time = mainclock_now();
                }
            }

            if (_options.swapoctets) {
              appconn->s_state.input_octets = bin;
              appconn->s_state.output_octets = bout;
              appconn->s_state.input_packets = pin;
              appconn->s_state.output_packets = pout;
            } else {
              appconn->s_state.output_octets = bin;
              appconn->s_state.input_octets = bout;
              appconn->s_state.output_packets = pin;
              appconn->s_state.input_packets = pout;
            }

            if(appconn->s_state.authenticated != state)
            {
                // fix state not sync.
                kmod_coova_update(appconn);
            }
          } else {
              log_dbg("Unknown entry - %.2X%.2X%.2X%.2X%.2X%.2X",
                      MAC_ARG(mac));
          }
        }
        else
        {
            log_dbg("dhcp-connect by nfcoova");
            if (dhcp_newconn(dhcp, &conn, mac)) {
                log_err(0, "[dhcp-connect] by nfcoova error");
            }
        }

    } else {
      log_err(errno, "Error parsing %s", line);
    }
  }

  if (line)
    free(line);

  fclose(fp);

  return 0;
}

