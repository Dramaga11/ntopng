/*
 *
 * (C) 2013-24 - ntop.org
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

/* *************************************** */

UsedPorts::UsedPorts(Host* h) : localhost(h) {
  if(trace_new_delete) ntop->getTrace()->traceEvent(TRACE_NORMAL, "[new] %s", __FILE__);
  char redis_key[128];
  static char buf[64];

  snprintf(redis_key, sizeof(redis_key), LOCALHOST_SERVER_PORT_BITMAP, 
           localhost->getInterface()->get_id(), localhost->getSerializationKey(buf, sizeof(buf)));
  u_int actual_len = ntop->getRedis()->len(redis_key);

  char* json_str = (char *)malloc(actual_len + 1);
  if ((ntop->getRedis()->get(redis_key, json_str, actual_len + 1)) == 0) {
    bitmap_server_ports.deserializer((const char*) json_str);
  }
  
  free(json_str);
}

/* *************************************** */

UsedPorts::UsedPorts() {
  if(trace_new_delete) ntop->getTrace()->traceEvent(TRACE_NORMAL, "[new] %s", __FILE__);
}

/* *************************************** */

UsedPorts::~UsedPorts() { ; }

/* *************************************** */

void UsedPorts::reset() {
  udp_server_ports.clear(), tcp_server_ports.clear();
  udp_client_contacted_ports.clear(), tcp_client_contacted_ports.clear();
}

/* *************************************** */

void UsedPorts::setLuaArray(
    lua_State *vm, NetworkInterface *iface, bool isTCP,
    std::unordered_map<u_int16_t, ndpi_protocol> *ports) {
  if (ports) {
    std::unordered_map<u_int16_t, ndpi_protocol>::iterator it;

    for (it = ports->begin(); it != ports->end(); ++it) {
      char str[32], buf[64];

      snprintf(str, sizeof(str), "%s:%u", isTCP ? "tcp" : "udp", it->first);
      lua_push_str_table_entry(
          vm, str,
          ndpi_protocol2name(iface->get_ndpi_struct(), it->second, buf,
                             sizeof(buf)));
    }
  }
}

/* *************************************** */

void UsedPorts::lua(lua_State *vm, NetworkInterface *iface) {
  lua_newtable(vm);

  lua_newtable(vm);

  /* ***************************** */

  setLuaArray(vm, iface, true, &tcp_server_ports);
  setLuaArray(vm, iface, false, &udp_server_ports);

  lua_pushstring(vm, "local_server_ports");
  lua_insert(vm, -2);
  lua_settable(vm, -3);

  /* ***************************** */

  lua_newtable(vm);

  setLuaArray(vm, iface, true, &tcp_client_contacted_ports);
  setLuaArray(vm, iface, false, &udp_client_contacted_ports);

  lua_pushstring(vm, "remote_contacted_ports");
  lua_insert(vm, -2);
  lua_settable(vm, -3);

  /* ***************************** */

  lua_pushstring(vm, "used_ports");
  lua_insert(vm, -2);
  lua_settable(vm, -3);
}

/* *************************************** */

/*Return false if not new server port are detected after the learning period, true otherwise*/
bool UsedPorts::setServerPort(bool isTCP, u_int16_t port,
                              ndpi_protocol *proto) {
  bool set_new_port = false;
  if (isTCP) {
    if((proto->master_protocol == NDPI_PROTOCOL_FTP_DATA)
       || (proto->app_protocol == NDPI_PROTOCOL_FTP_DATA))
      ;
    else {
      if (tcp_server_ports.count(port) == 0) set_new_port = true;
      tcp_server_ports[port] = *proto;
      bitmap_server_ports.addPort(true, port);
    }
  } else {
    if (udp_server_ports.count(port) == 0) set_new_port = true;
    udp_server_ports[port] = *proto;
    bitmap_server_ports.addPort(false, port);
  }

  return set_new_port;
}

/* *************************************** */

void UsedPorts::setContactedPort(bool isTCP, u_int16_t port,
                                 ndpi_protocol *proto) {
  if (isTCP)
    tcp_client_contacted_ports[port] = *proto;
  else
    udp_client_contacted_ports[port] = *proto;
}
