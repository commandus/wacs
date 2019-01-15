#ifndef SNMPAGENTONEWAYTICKET_H_
#define SNMPAGENTONEWAYTICKET_H_

#include <stdint.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

typedef struct SNMP_SVC_ONEWAYTICKET {
	char *databaseinstance;
	uint32_t starttime;
	uint32_t requestcount;
	uint32_t memorypeak;
	uint32_t memorycurrent;
	uint32_t errorcount;
	uint32_t ticketssold;
} SNMP_SVC_ONEWAYTICKET;


#ifdef __cplusplus
extern "C" {
#endif
	Netsnmp_Node_Handler handle_databaseinstance;
	Netsnmp_Node_Handler handle_starttime;
	Netsnmp_Node_Handler handle_requestcount;
	Netsnmp_Node_Handler handle_memorypeak;
	Netsnmp_Node_Handler handle_memorycurrent;
	Netsnmp_Node_Handler handle_errorcount;
	Netsnmp_Node_Handler handle_ticketssold;

	void init_mibs();
	void init_databaseinstance(void);
	void init_starttime(void);
	void init_requestcount(void);
	void init_memorypeak(void);
	void init_memorycurrent(void);
	void init_errorcount(void);
	void init_ticketssold(void);

	void set_requestcount(uint32_t value);
	void set_errorcount(uint32_t value);
	void set_ticketssold(uint32_t value);

	void inc_requestcount();
	void inc_errorcount();
	void inc_ticketssold();

#ifdef __cplusplus
}
#endif

#endif
