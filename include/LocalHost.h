/*
 *
 * (C) 2013-22 - ntop.org
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

#ifndef _LOCAL_HOST_H_
#define _LOCAL_HOST_H_

#include "ntop_includes.h"

class LocalHost : public Host, public SerializableElement {
 protected:
  int16_t local_network_id;
  bool systemHost;
  time_t initialization_time;
  LocalHostStats *initial_ts_point;
  std::unordered_map<u_int32_t, DoHDoTStats*> doh_dot_map;
  u_int8_t router_mac[6]; /* MAC address pf the first router used (no Mac* to avoid purging race conditions) */
  bool router_mac_set;
  HostPorts usedPorts;

  /* LocalHost data: update LocalHost::deleteHostData when adding new fields */
  char *os_detail;
  bool drop_all_host_traffic;
  /* END Host data: */

  void initialize();
  void freeLocalHostData();
  virtual void deleteHostData();

  char* getMacBasedSerializationKey(char *redis_key, size_t size, char *mac_key);
  char* getIpBasedSerializationKey(char *redis_key, size_t size);
  void luaDoHDot(lua_State *vm);
  
 public:
  LocalHost(NetworkInterface *_iface, Mac *_mac, VLANid _vlanId, u_int16_t _observation_point_id, IpAddress *_ip);
  LocalHost(NetworkInterface *_iface, char *ipAddress, VLANid _vlanId, u_int16_t _observation_point_id);
  virtual ~LocalHost();

  virtual void set_hash_entry_state_idle();
  virtual int16_t get_local_network_id() const { return(local_network_id);  };
  virtual bool isLocalHost()  const            { return(true);              };
  virtual bool isSystemHost() const            { return(systemHost);        };

  virtual NetworkStats* getNetworkStats(int16_t networkId) {
    return(iface->getNetworkStats(networkId));
  };
  virtual u_int32_t getActiveHTTPHosts() { return(getHTTPstats() ? getHTTPstats()->get_num_virtual_hosts() : 0); };
  virtual HostStats* allocateStats()     { return(new LocalHostStats(this)); };

  virtual bool dropAllTraffic() const { return(drop_all_host_traffic); };
  virtual void inlineSetOSDetail(const char *_os_detail);
  virtual const char* getOSDetail(char * const buf, ssize_t buf_len);
  virtual void updateHostTrafficPolicy(char *key);

  virtual void luaHTTP(lua_State *vm)              { stats->luaHTTP(vm);         };
  virtual void luaDNS(lua_State *vm, bool verbose) { stats->luaDNS(vm, verbose); luaDoHDot(vm); };
  virtual void luaICMP(lua_State *vm, bool isV4, bool verbose) { stats->luaICMP(vm,isV4,verbose); };
  virtual void lua_get_timeseries(lua_State* vm);
  virtual void lua_peers_stats(lua_State* vm)    const;
  virtual void lua_contacts_stats(lua_State *vm) const;
  virtual void incrVisitedWebSite(char *hostname)  { stats->incrVisitedWebSite(hostname); };
  virtual HTTPstats* getHTTPstats()                { return(stats->getHTTPstats());       };
  virtual DnsStats*  getDNSstats()                 { return(stats->getDNSstats());        };
  virtual ICMPstats* getICMPstats()                { return(stats->getICMPstats());       };
  virtual void luaTCP(lua_State *vm)               { stats->lua(vm,false,details_normal); };
  virtual u_int16_t getNumActiveContactsAsClient() { return stats->getNumActiveContactsAsClient(); };
  virtual u_int16_t getNumActiveContactsAsServer() { return stats->getNumActiveContactsAsServer(); };
  virtual void reloadPrefs();
  virtual void addContactedDomainName(char* domain_name)    { stats->addContactedDomainName(domain_name);   }         
  virtual u_int32_t getDomainNamesCardinality()             { return stats->getDomainNamesCardinality();    }      
  virtual void resetDomainNamesCardinality()                { stats->resetDomainNamesCardinality();         }  

  virtual void deserialize(json_object *obj);
  virtual void serialize(json_object *obj, DetailsLevel details_level) { return Host::serialize(obj, details_level); };
  virtual char* getSerializationKey(char *buf, uint bufsize);
  char* getRedisKey(char *buf, uint buf_len, bool skip_prefix = false);
  
  virtual void lua(lua_State* vm, AddressTree * ptree, bool host_details,
		   bool verbose, bool returnHost, bool asListElement);
  void custom_periodic_stats_update(const struct timeval *tv) { ; }

  virtual void luaHostBehaviour(lua_State* vm)    { if(stats) stats->luaHostBehaviour(vm); }
  virtual void incDohDoTUses(Host *srv_host);

  virtual void incCountriesContacts(char *country)    { stats->incCountriesContacts(country);             }
  virtual void resetCountriesContacts()               { stats->resetCountriesContacts();                  }
  virtual u_int8_t getCountriesContactsCardinality()  { return(stats->getCountriesContactsCardinality()); }

  virtual bool incNTPContactCardinality(Host *h)  { return(stats->incNTPContactCardinality(h));  }
  virtual bool incDNSContactCardinality(Host *h)  { return(stats->incDNSContactCardinality(h));  }
  virtual bool incSMTPContactCardinality(Host *h) { return(stats->incSMTPContactCardinality(h)); }
  virtual bool incIMAPContactCardinality(Host *h) { return(stats->incIMAPContactCardinality(h)); }
  virtual bool incPOPContactCardinality(Host *h)  { return(stats->incPOPContactCardinality(h)); }

  virtual u_int32_t getNTPContactCardinality()    { return(stats->getNTPContactCardinality());  }
  virtual u_int32_t getDNSContactCardinality()    { return(stats->getDNSContactCardinality());  }
  virtual u_int32_t getSMTPContactCardinality()   { return(stats->getSMTPContactCardinality()); }
  virtual u_int32_t getIMAPContactCardinality()   { return(stats->getIMAPContactCardinality()); }
  virtual u_int32_t getPOPContactCardinality()    { return(stats->getPOPContactCardinality()); }

  void setRouterMac(Mac *gw);
  
  inline void setServerPort(bool isTCP, u_int16_t port)    { usedPorts.setServerPort(isTCP, port);    };
  inline void setContactedPort(bool isTCP, u_int16_t port) { usedPorts.setContactedPort(isTCP, port); };
  virtual void luaUsedPorts(lua_State* vm)                 { usedPorts.lua(vm); }
  virtual ndpi_bitmap* getServerPorts()                    { return(usedPorts.getServerPorts()); }
};

#endif /* _LOCAL_HOST_H_ */
