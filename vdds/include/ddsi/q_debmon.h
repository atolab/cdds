/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef Q_DEBMON_H
#define Q_DEBMON_H

struct debug_monitor;
typedef int (*debug_monitor_cpf_t) (ddsi_tran_conn_t conn, const char *fmt, ...);
typedef int (*debug_monitor_plugin_t) (ddsi_tran_conn_t conn, debug_monitor_cpf_t cpf, void *arg);

struct debug_monitor *new_debug_monitor (int port);
void add_debug_monitor_plugin (struct debug_monitor *dm, debug_monitor_plugin_t fn, void *arg);
void free_debug_monitor (struct debug_monitor *dm);

#endif /* defined(__ospli_osplo__q_debmon__) */
