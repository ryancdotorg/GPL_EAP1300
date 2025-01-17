/* -*- mode: c; c-basic-offset: 2 -*- */
/*
 * Copyright (C) 2003, 2004, 2005 Mondru AB.
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

#include "system.h"
#include "session.h"
#include "dhcp.h"
#include "radius.h"
#include "chilli.h"

#ifdef ENABLE_JSON
int session_redir_json_fmt(bstring json, char *userurl, char *redirurl,
			   bstring logouturl, uint8_t *hismac,
			   struct in_addr *hisip) {
  bcatcstr(json,",\"redir\":{\"originalURL\":\"");
  bcatcstr(json, userurl?userurl:"");
  bcatcstr(json,"\",\"redirectionURL\":\"");
  bcatcstr(json, redirurl?redirurl:"");
  if (logouturl) {
    bcatcstr(json,"\",\"logoutURL\":\"");
    bconcat(json, logouturl);
  }
  bcatcstr(json,"\",\"ipAddress\":\"");
  bcatcstr(json, inet_ntoa(*hisip));
#ifdef ENABLE_LAYER3
  if (!_options.layer3) {
#endif
    bcatcstr(json,"\",\"macAddress\":\"");
    if (hismac) {
      char mac[REDIR_MACSTRLEN+1];
      safe_snprintf(mac, sizeof(mac), "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
                    (unsigned int)hismac[0], (unsigned int)hismac[1],
                    (unsigned int)hismac[2], (unsigned int)hismac[3],
                    (unsigned int)hismac[4], (unsigned int)hismac[5]);
      bcatcstr(json, mac);
    }
#ifdef ENABLE_LAYER3
  }
#endif
  bcatcstr(json,"\"}");
  return 0;
}

int session_json_params(struct session_state *state,
			struct session_params *params,
			bstring json, int init) {
  bstring tmp = bfromcstr("");
  time_t starttime = state->start_time;

  bcatcstr(json,"\"sessionId\":\"");
  bcatcstr(json,state->sessionid);
  bcatcstr(json,"\",\"userName\":\"");
  bcatcstr(json,state->redir.username);
  bcatcstr(json, "\",\"startTime\":");
  bassignformat(tmp, "%ld", mainclock_towall(init ? mainclock_now() : starttime));
  bconcat(json, tmp);
  bcatcstr(json,",\"sessionTimeout\":");
  bassignformat(tmp, "%lld", params->sessiontimeout);
  bconcat(json, tmp);
  bcatcstr(json,",\"idleTimeout\":");
  bassignformat(tmp, "%ld", params->idletimeout);
  bconcat(json, tmp);
  bcatcstr(json,",\"goneTimeout\":");
  bassignformat(tmp, "%ld", params->gonetimeout);
  bconcat(json, tmp);
#ifdef ENABLE_IEEE8021Q
  if (_options.ieee8021q && state->tag8021q) {
    bcatcstr(json,",\"vlan\":");
    bassignformat(tmp, "%d", (int)ntohs(state->tag8021q & PKT_8021Q_MASK_VID));
    bconcat(json, tmp);
  }
#endif
  if (params->maxinputoctets) {
    bcatcstr(json,",\"maxInputOctets\":");
    bassignformat(tmp, "%lld", params->maxinputoctets);
    bconcat(json, tmp);
  }
  if (params->maxoutputoctets) {
    bcatcstr(json,",\"maxOutputOctets\":");
    bassignformat(tmp, "%lld", params->maxoutputoctets);
    bconcat(json, tmp);
  }
  if (params->maxtotaloctets) {
    bcatcstr(json,",\"maxTotalOctets\":");
    bassignformat(tmp, "%lld", params->maxtotaloctets);
    bconcat(json, tmp);
  }

  bdestroy(tmp);
  return 0;
}

int session_json_acct(struct session_state *state,
                      struct session_params *params,
                      bstring json, int init) {
  bstring tmp = bfromcstr("");
  uint32_t inoctets = state->input_octets;
  uint32_t outoctets = state->output_octets;
  uint32_t ingigawords = (state->input_octets >> 32);
  uint32_t outgigawords = (state->output_octets >> 32);
  uint32_t sessiontime;
  uint32_t idletime;
  uint32_t gonetime;
  uint32_t connecttime;
  uint32_t acctconnecttime;
  int userhere = state->user_here;
  uint32_t inpkts = state->input_packets;
  uint32_t outpkts = state->output_packets;

  sessiontime = mainclock_diffu(state->start_time);
  idletime    = mainclock_diffu(state->last_sent_time);
  gonetime    = mainclock_diffu(state->gone_time);
  connecttime    = mainclock_diffu(state->connect_start_time);
  acctconnecttime = mainclock_diffu(state->acct_start_time);

  init = init || !state->authenticated;


  bcatcstr(json,"\"present\":");
  bassignformat(tmp, "%d", init ? 0 : current_user_here(userhere));
  bconcat(json, tmp);
  bcatcstr(json,",\"noAccounting\":");
  bassignformat(tmp, "%d", init ? 0 : (params->flags & NO_ACCOUNTING)?1:0);
  bconcat(json, tmp);
  bcatcstr(json,",\"syncFlag\":");
  bassignformat(tmp, "%d", init ? 0 : state->sync_flag);
  bconcat(json, tmp);
  bcatcstr(json,",\"sessionTime\":");
  bassignformat(tmp, "%lu", init ? 0 : sessiontime);
  bconcat(json, tmp);
  bcatcstr(json,",\"idleTime\":");
  bassignformat(tmp, "%lu", init ? 0 : idletime);
  bconcat(json, tmp);
  bcatcstr(json,",\"goneTime\":");
  bassignformat(tmp, "%lu", init ? 0 : gonetime);
  bconcat(json, tmp);
  bcatcstr(json,",\"connectTime\":");
  bassignformat(tmp, "%lu", init ? 0 : current_user_here(userhere) ? connecttime : 0);
  bconcat(json, tmp);
  bcatcstr(json,",\"acctTime\":");
  bassignformat(tmp, "%lu", init ? 0 : (params->flags & NO_ACCOUNTING)?0:acctconnecttime);
  bconcat(json, tmp);
  bcatcstr(json,",\"acctInterval\":");
  bassignformat(tmp, "%lu", init ? 0 : (params->flags & NO_ACCOUNTING)?0:params->interim_interval);
  bconcat(json, tmp);
  bcatcstr(json,",\"inputOctets\":");
  bassignformat(tmp, "%lu",init ? 0 :  inoctets);
  bconcat(json, tmp);
  bcatcstr(json,",\"outputOctets\":");
  bassignformat(tmp, "%lu", init ? 0 : outoctets);
  bconcat(json, tmp);
  bcatcstr(json,",\"inputPackets\":");
  bassignformat(tmp, "%lu",init ? 0 :  inpkts);
  bconcat(json, tmp);
  bcatcstr(json,",\"outputPackets\":");
  bassignformat(tmp, "%lu", init ? 0 : outpkts);
  bconcat(json, tmp);
  bcatcstr(json,",\"inputGigawords\":");
  bassignformat(tmp, "%lu", init ? 0 : ingigawords);
  bconcat(json, tmp);
  bcatcstr(json,",\"outputGigawords\":");
  bassignformat(tmp, "%lu", init ? 0 : outgigawords);
  bconcat(json, tmp);
  bassignformat(tmp, ",\"viewPoint\":\"%s\"",
		_options.swapoctets ? "nas" : "client");
  bconcat(json, tmp);

  bdestroy(tmp);
  return 0;
}

int session_json_fmt(struct session_state *state,
		     struct session_params *params,
		     bstring json, int init) {
  bcatcstr(json,",\"session\":{");
  session_json_params(state,params,json,init);
  bcatcstr(json,"}");

  bcatcstr(json,",\"accounting\":{");
  session_json_acct(state,params,json,init);
  bcatcstr(json,"}");

  return 0;
}
#endif

