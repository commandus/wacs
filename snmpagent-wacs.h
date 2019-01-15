#ifndef SNMP_AGENT_WACS_H
#define SNMP_AGENT_WACS_H

#include "snmp-params.h"
#include <stdint.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

MonitoringParams *getInstance();

#ifdef __cplusplus
extern "C" {
#endif
void init_mibs();
void init_databasefilename();
Netsnmp_Node_Handler handle_databasefilename;
void init_ip();
Netsnmp_Node_Handler handle_ip;
void init_port();
Netsnmp_Node_Handler handle_port;
void init_starttime();
Netsnmp_Node_Handler handle_starttime;
void init_requestsperhour();
Netsnmp_Node_Handler handle_requestsperhour;
void init_lastmac();
Netsnmp_Node_Handler handle_lastmac;
void init_lastssisignal();
Netsnmp_Node_Handler handle_lastssisignal;
void init_totalmac();
Netsnmp_Node_Handler handle_totalmac;
void init_databasefilesize();
Netsnmp_Node_Handler handle_databasefilesize;
void init_memorypeak();
Netsnmp_Node_Handler handle_memorypeak;
void init_memorycurrent();
Netsnmp_Node_Handler handle_memorycurrent;
void init_errorcount();
Netsnmp_Node_Handler handle_errorcount;
#endif
#ifdef __cplusplus
}
#endif
