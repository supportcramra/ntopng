/*
 *
 * (C) 2013-16 - ntop.org
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

#ifndef _REDIS_H_
#define _REDIS_H_

#include "ntop_includes.h"

class Host;

class Redis {
 private:
  redisContext *redis;
  Mutex *l;
  char *redis_host;
  u_int16_t redis_port;
  u_int8_t redis_db_id;
  pthread_t esThreadLoop;
  bool operational;

  void setDefaults();
  void reconnectRedis();
  int msg_push(const char *cmd, const char *queue_name, char *msg, u_int queue_trim_size);
  int oneOperator(const char *operation, char *key);
  int twoOperators(const char *operation, char *op1, char *op2);
  int pushHost(const char* ns_cache, const char* ns_list, char *hostname,
	       bool dont_check_for_existance, bool localHost);
  int popHost(const char* ns_list, char *hostname, u_int hostname_len);

 public:
  Redis(char *redis_host = (char*)"127.0.0.1", u_int16_t redis_port = 6379, u_int8_t _redis_db_id = 0);
  ~Redis();

  char* getVersion(char *str, u_int str_len);

  inline bool isOperational() { return(operational); };
  int expire(char *key, u_int expire_sec);
  int get(char *key, char *rsp, u_int rsp_len);
  int hashGet(char *key, char *member, char *rsp, u_int rsp_len);
  int hashDel(char *key, char *field);
  int hashSet(char *key, char *field, char *value);
  int delHash(char *key, char *member); 
  int set(char *key, char *value, u_int expire_secs=0);
  char* popSet(char *pop_name, char *rsp, u_int rsp_len);
  int keys(const char *pattern, char ***keys_p);
  int hashKeys(const char *pattern, char ***keys_p);
  int del(char *key); 
  int zIncr(char *key, char *member);
  int zTrim(char *key, u_int trim_len);
  int zRevRange(const char *pattern, char ***keys_p);
  int pushHostToResolve(char *hostname, bool dont_check_for_existance, bool localHost);
  int popHostToResolve(char *hostname, u_int hostname_len);

  int pushHostToTrafficFiltering(char *hostname, bool dont_check_for_existance, bool localHost);
  int popHostToTrafficFiltering(char *hostname, u_int hostname_len);

  char* getTrafficFilteringCategory(char *numeric_ip, char *buf, u_int buf_len, bool query_httpbl_if_unknown);
  int popDomainToCategorize(char *domainname, u_int domainname_len);
  
  int getAddress(char *numeric_ip, char *rsp, u_int rsp_len, bool queue_if_not_found);
  int getAddressTrafficFiltering(char *numeric_ip, NetworkInterface *iface,
		       char *rsp, u_int rsp_len, bool queue_if_not_found);
  int setResolvedAddress(char *numeric_ip, char *symbolic_ip);
  int setTrafficFilteringAddress(char* numeric_ip, char* httpbl);

  int hashIncr(char *key, char *field, u_int32_t value);

  int addHostToDBDump(NetworkInterface *iface, IpAddress *ip, char *name);

  int smembers(lua_State* vm, char *setName);

  void setHostId(NetworkInterface *iface, char *daybuf, char *host_name, u_int32_t id);
  u_int32_t host_to_id(NetworkInterface *iface, char *daybuf, char *host_name, bool *new_key);
  int id_to_host(char *daybuf, char *host_idx, char *buf, u_int buf_len);
  void pushEStemplate();
  void indexESdata();
  int lpush(const char *queue_name, char *msg, u_int queue_trim_size);
  int rpush(const char *queue_name, char *msg, u_int queue_trim_size);
  u_int llen(const char *queue_name);
  int lpop(const char *queue_name, char *buf, u_int buf_len);
  int lpop(const char *queue_name, char ***elements, u_int num_elements);
  void startFlowDump();

  /**
   * @brief Increment a redis key and return its new value
   *
   * @param key The key whose value will be incremented.
   */
  u_int32_t incrKey(char *key);

  /**
   * @brief Queue an alert in redis
   *
   * @param level The alert level
   * @param t     The alert type
   * @param msg   The alert message
   */
  void queueAlert(AlertLevel level, AlertType t, char *msg);

  /**
   * @brief Returns the number of queued alerts in redis generated by ntopng
   *
   */
  inline u_int getNumQueuedAlerts() { return(llen((char*)CONST_ALERT_MSG_QUEUE)); };

  /**
   * @brief Returns up to the specified number of alerts, and removes them from redis. The first parameter must be long enough to hold the returned results
   * @param allowed_hosts The list of hosts allowed to be returned by this function
   * @param alerts The returned alerts
   * @param start_idx The initial queue index from which extract messages. Zero (0) is the first (i.e. most recent) queue element.
   * @param num The maximum number of alerts to return.
   * @return The number of elements read.
   *
   */
  u_int getQueuedAlerts(patricia_tree_t *allowed_hosts, char **alerts, u_int start_idx, u_int num);

  /**
   * @brief Delete the alert identified by the specified index.
   * @param idx The queued alert index to delete. Zero (0) is the first (i.e. most recent) queue element.
   * @return The number of elements read.
   *
   */
  void deleteQueuedAlert(u_int32_t idx);

  /**
   * @brief Flush all queued alerts
   *
   */
  inline void flushAllQueuedAlerts() { delKey((char*)CONST_ALERT_MSG_QUEUE);       };
  int delKey(char *key)              { return(oneOperator("DEL", key));            };
  int rename(char *oldk, char *newk) { return(twoOperators("RENAME", oldk, newk)); };
};

#endif /* _REDIS_H_ */
