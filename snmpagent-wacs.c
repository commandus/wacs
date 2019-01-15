#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "snmpagent-wacs.h"
#include "get_rss.h"

static struct SNMP_SVC_ONEWAYTICKET counter;

void init_databasefilename(void)
{
    const oid       databasefilename_oid[] =
        { 1, 3, 6, 1, 4, 1, 46821, 2, 1, 1, 1 };
    DEBUGMSGTL(("databasefilename", "Initializing\n"));
    netsnmp_register_scalar(netsnmp_create_handler_registration
                            ("databasefilename", handle_databasefilename,
                             databasefilename_oid,
                             OID_LENGTH(databasefilename_oid),
                             HANDLER_CAN_RONLY));
}

void init_ip(void)
{
    const oid       ip_oid[] = { 1, 3, 6, 1, 4, 1, 46821, 2, 1, 1, 2 };
    DEBUGMSGTL(("ip", "Initializing\n"));
    netsnmp_register_scalar(netsnmp_create_handler_registration
                            ("ip", handle_ip, ip_oid, OID_LENGTH(ip_oid),
                             HANDLER_CAN_RONLY));
}

void init_port(void)
{
    const oid       port_oid[] = { 1, 3, 6, 1, 4, 1, 46821, 2, 1, 1, 3 };
    DEBUGMSGTL(("port", "Initializing\n"));
    netsnmp_register_scalar(netsnmp_create_handler_registration
                            ("port", handle_port, port_oid,
                             OID_LENGTH(port_oid), HANDLER_CAN_RONLY));
}

void init_starttime(void)
{
    const oid       starttime_oid[] =
        { 1, 3, 6, 1, 4, 1, 46821, 2, 1, 1, 4 };
    DEBUGMSGTL(("starttime", "Initializing\n"));
    netsnmp_register_scalar(netsnmp_create_handler_registration
                            ("starttime", handle_starttime, starttime_oid,
                             OID_LENGTH(starttime_oid),
                             HANDLER_CAN_RONLY));
}

void init_requestsperhour(void)
{
    const oid       requestsperhour_oid[] =
        { 1, 3, 6, 1, 4, 1, 46821, 2, 1, 2, 5 };
    DEBUGMSGTL(("requestsperhour", "Initializing\n"));
    netsnmp_register_scalar(netsnmp_create_handler_registration
                            ("requestsperhour", handle_requestsperhour,
                             requestsperhour_oid,
                             OID_LENGTH(requestsperhour_oid),
                             HANDLER_CAN_RONLY));
}

void init_lastmac(void)
{
    const oid       lastmac_oid[] =
        { 1, 3, 6, 1, 4, 1, 46821, 2, 1, 1, 6 };
    DEBUGMSGTL(("lastmac", "Initializing\n"));
    netsnmp_register_scalar(netsnmp_create_handler_registration
                            ("lastmac", handle_lastmac, lastmac_oid,
                             OID_LENGTH(lastmac_oid), HANDLER_CAN_RONLY));
}

void init_lastssisignal(void)
{
    const oid       lastssisignal_oid[] =
        { 1, 3, 6, 1, 4, 1, 46821, 2, 1, 2, 7 };
    DEBUGMSGTL(("lastssisignal", "Initializing\n"));
    netsnmp_register_scalar(netsnmp_create_handler_registration
                            ("lastssisignal", handle_lastssisignal,
                             lastssisignal_oid,
                             OID_LENGTH(lastssisignal_oid),
                             HANDLER_CAN_RONLY));
}

void init_totalmac(void)
{
    const oid       totalmac_oid[] =
        { 1, 3, 6, 1, 4, 1, 46821, 2, 1, 2, 8 };
    DEBUGMSGTL(("totalmac", "Initializing\n"));
    netsnmp_register_scalar(netsnmp_create_handler_registration
                            ("totalmac", handle_totalmac, totalmac_oid,
                             OID_LENGTH(totalmac_oid), HANDLER_CAN_RONLY));
}

void init_databasefilesize(void)
{
    const oid       databasefilesize_oid[] =
        { 1, 3, 6, 1, 4, 1, 46821, 2, 1, 2, 9 };
    DEBUGMSGTL(("databasefilesize", "Initializing\n"));
    netsnmp_register_scalar(netsnmp_create_handler_registration
                            ("databasefilesize", handle_databasefilesize,
                             databasefilesize_oid,
                             OID_LENGTH(databasefilesize_oid),
                             HANDLER_CAN_RONLY));
}

void init_memorypeak(void)
{
    const oid       memorypeak_oid[] =
        { 1, 3, 6, 1, 4, 1, 46821, 2, 1, 2, 10 };
    DEBUGMSGTL(("memorypeak", "Initializing\n"));
    netsnmp_register_scalar(netsnmp_create_handler_registration
                            ("memorypeak", handle_memorypeak,
                             memorypeak_oid, OID_LENGTH(memorypeak_oid),
                             HANDLER_CAN_RONLY));
}

void init_memorycurrent(void)
{
    const oid       memorycurrent_oid[] =
        { 1, 3, 6, 1, 4, 1, 46821, 2, 1, 2, 11 };
    DEBUGMSGTL(("memorycurrent", "Initializing\n"));
    netsnmp_register_scalar(netsnmp_create_handler_registration
                            ("memorycurrent", handle_memorycurrent,
                             memorycurrent_oid,
                             OID_LENGTH(memorycurrent_oid),
                             HANDLER_CAN_RONLY));
}

void init_errorcount(void)
{
    const oid       errorcount_oid[] =
        { 1, 3, 6, 1, 4, 1, 46821, 2, 1, 2, 12 };
    DEBUGMSGTL(("errorcount", "Initializing\n"));
    netsnmp_register_scalar(netsnmp_create_handler_registration
                            ("errorcount", handle_errorcount,
                             errorcount_oid, OID_LENGTH(errorcount_oid),
                             HANDLER_CAN_RONLY));
}

/*
 * ------------------ Handlers --------------------
 */

int handle_databasefilename(netsnmp_mib_handler *handler,
                        netsnmp_handler_registration *reginfo,
                        netsnmp_agent_request_info *reqinfo,
                        netsnmp_request_info *requests)
{
    switch (reqinfo->mode) {
    case MODE_GET:
        snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                 /*
                                  * XXX: a pointer to the scalar's data 
                                  */ ,
                                 /*
                                  * XXX: the length of the data in bytes 
                                  */ );
        break;
    default:
        snmp_log(LOG_ERR, "unknown mode (%d) in handle_databasefilename\n",
                 reqinfo->mode);
        return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}


int handle_ip(netsnmp_mib_handler *handler,
          netsnmp_handler_registration *reginfo,
          netsnmp_agent_request_info *reqinfo,
          netsnmp_request_info *requests)
{
    switch (reqinfo->mode) {
    case MODE_GET:
        snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                 /*
                                  * XXX: a pointer to the scalar's data 
                                  */ ,
                                 /*
                                  * XXX: the length of the data in bytes 
                                  */ );
        break;
    default:
        snmp_log(LOG_ERR, "unknown mode (%d) in handle_ip\n",
                 reqinfo->mode);
        return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int handle_port(netsnmp_mib_handler *handler,
            netsnmp_handler_registration *reginfo,
            netsnmp_agent_request_info *reqinfo,
            netsnmp_request_info *requests)
{
    switch (reqinfo->mode) {
    case MODE_GET:
        snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                 /*
                                  * XXX: a pointer to the scalar's data 
                                  */ ,
                                 /*
                                  * XXX: the length of the data in bytes 
                                  */ );
        break;
    default:
        snmp_log(LOG_ERR, "unknown mode (%d) in handle_port\n",
                 reqinfo->mode);
        return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}

int handle_starttime(netsnmp_mib_handler *handler,
                 netsnmp_handler_registration *reginfo,
                 netsnmp_agent_request_info *reqinfo,
                 netsnmp_request_info *requests)
{
    switch (reqinfo->mode) {
    case MODE_GET:
        snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                 /*
                                  * XXX: a pointer to the scalar's data 
                                  */ ,
                                 /*
                                  * XXX: the length of the data in bytes 
                                  */ );
        break;
    default:
        snmp_log(LOG_ERR, "unknown mode (%d) in handle_starttime\n",
                 reqinfo->mode);
        return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}


int handle_requestsperhour(netsnmp_mib_handler *handler,
                       netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo,
                       netsnmp_request_info *requests)
{
    switch (reqinfo->mode) {
    case MODE_GET:
        snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                 /*
                                  * XXX: a pointer to the scalar's data 
                                  */ ,
                                 /*
                                  * XXX: the length of the data in bytes 
                                  */ );
        break;
    default:
        snmp_log(LOG_ERR, "unknown mode (%d) in handle_requestsperhour\n",
                 reqinfo->mode);
        return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int handle_lastmac(netsnmp_mib_handler *handler,
               netsnmp_handler_registration *reginfo,
               netsnmp_agent_request_info *reqinfo,
               netsnmp_request_info *requests)
{
    switch (reqinfo->mode) {

    case MODE_GET:
        snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                 /*
                                  * XXX: a pointer to the scalar's data 
                                  */ ,
                                 /*
                                  * XXX: the length of the data in bytes 
                                  */ );
        break;


    default:
        snmp_log(LOG_ERR, "unknown mode (%d) in handle_lastmac\n",
                 reqinfo->mode);
        return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int handle_lastssisignal(netsnmp_mib_handler *handler,
                     netsnmp_handler_registration *reginfo,
                     netsnmp_agent_request_info *reqinfo,
                     netsnmp_request_info *requests)
{
    switch (reqinfo->mode) {
    case MODE_GET:
        snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                 /*
                                  * XXX: a pointer to the scalar's data 
                                  */ ,
                                 /*
                                  * XXX: the length of the data in bytes 
                                  */ );
        break;
    default:
        snmp_log(LOG_ERR, "unknown mode (%d) in handle_lastssisignal\n",
                 reqinfo->mode);
        return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int handle_totalmac(netsnmp_mib_handler *handler,
                netsnmp_handler_registration *reginfo,
                netsnmp_agent_request_info *reqinfo,
                netsnmp_request_info *requests)
{
    switch (reqinfo->mode) {
    case MODE_GET:
        snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                 /*
                                  * XXX: a pointer to the scalar's data 
                                  */ ,
                                 /*
                                  * XXX: the length of the data in bytes 
                                  */ );
        break;
    default:
        snmp_log(LOG_ERR, "unknown mode (%d) in handle_totalmac\n",
                 reqinfo->mode);
        return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}

int handle_databasefilesize(netsnmp_mib_handler *handler,
                        netsnmp_handler_registration *reginfo,
                        netsnmp_agent_request_info *reqinfo,
                        netsnmp_request_info *requests)
{
    switch (reqinfo->mode) {

    case MODE_GET:
        snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                 /*
                                  * XXX: a pointer to the scalar's data 
                                  */ ,
                                 /*
                                  * XXX: the length of the data in bytes 
                                  */ );
        break;


    default:
        /*
         * we should never get here, so this is a really bad error 
         */
        snmp_log(LOG_ERR, "unknown mode (%d) in handle_databasefilesize\n",
                 reqinfo->mode);
        return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}


int handle_memorypeak(netsnmp_mib_handler *handler,
                  netsnmp_handler_registration *reginfo,
                  netsnmp_agent_request_info *reqinfo,
                  netsnmp_request_info *requests)
{
    switch (reqinfo->mode) {
    case MODE_GET:
        	counter.memorypeak = getPeakRSS() / 1024;
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
            		&counter.memorypeak,
            		sizeof(counter.memorypeak));
        break;
    default:
        /*
         * we should never get here, so this is a really bad error 
         */
        snmp_log(LOG_ERR, "unknown mode (%d) in handle_memorypeak\n",
                 reqinfo->mode);
        return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int handle_memorycurrent(netsnmp_mib_handler *handler,
                     netsnmp_handler_registration *reginfo,
                     netsnmp_agent_request_info *reqinfo,
                     netsnmp_request_info *requests)
{
    switch (reqinfo->mode) {
    case MODE_GET:
        	counter.memorycurrent = getCurrentRSS() / 1024;
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
            		&counter.memorycurrent,
            		sizeof(counter.memorycurrent));
        break;
    default:
        snmp_log(LOG_ERR, "unknown mode (%d) in handle_memorycurrent\n",
                 reqinfo->mode);
        return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}

int handle_errorcount(netsnmp_mib_handler *handler,
                  netsnmp_handler_registration *reginfo,
                  netsnmp_agent_request_info *reqinfo,
                  netsnmp_request_info *requests)
{
    switch (reqinfo->mode) {
    case MODE_GET:
        snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                 /*
                                  * XXX: a pointer to the scalar's data 
                                  */ ,
                                 /*
                                  * XXX: the length of the data in bytes 
                                  */ );
        break;
    default:
        snmp_log(LOG_ERR, "unknown mode (%d) in handle_errorcount\n",
                 reqinfo->mode);
        return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
