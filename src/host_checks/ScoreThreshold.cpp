/*
 *
 * (C) 2013-21 - ntop.org
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include "ntop_includes.h"
#include "host_checks_includes.h"

/* ***************************************************** */

ScoreThreshold::ScoreThreshold(u_int32_t threshold) : HostCheck(ntopng_edition_community, false /* All interfaces */, false /* Don't exclude for nEdge */, false /* NOT only for nEdge */) {
  threshold = (u_int32_t)-1;
};

/* ***************************************************** */

void ScoreThreshold::periodicUpdate(Host *h, HostAlert *engaged_alert) {
  HostAlert *alert = engaged_alert;
  risk_percentage cli_pctg = CLIENT_FULL_RISK_PERCENTAGE;
  u_int32_t cli_score = h->getScoreAsClient(), srv_score = h->getScoreAsServer(), value = 0;

  if(cli_score > threshold)
    cli_pctg = CLIENT_FULL_RISK_PERCENTAGE, value = cli_score;
  else if(srv_score > threshold)
    cli_pctg = CLIENT_NO_RISK_PERCENTAGE, value = srv_score;

  if(value) {
    if(!alert)
      alert = allocAlert(this, h, cli_pctg, value, threshold);

    /* Refresh */
    if(alert)
      h->triggerAlert(alert);
  }
}

/* ***************************************************** */

bool ScoreThreshold::loadConfiguration(json_object *config) {
  json_object *json_threshold;

  HostCheck::loadConfiguration(config); /* Parse parameters in common */

  if(json_object_object_get_ex(config, "threshold", &json_threshold))
    threshold = json_object_get_int64(json_threshold);

  // ntop->getTrace()->traceEvent(TRACE_NORMAL, "%s %u", json_object_to_json_string(config), dns_bytes_threshold);

  return(true);
}

/* ***************************************************** */
