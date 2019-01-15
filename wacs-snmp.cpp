/**
 * @file wacs-snmp.cpp
 * Command line utility shows wacs service statistic
 * DB	service start date/time	elapsed	queries	memory	peak	errors	sold
	    2017-02-20 17:13:43     0:21    3       32588   32588   0       0
 *
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "wacs-snmp-config.h"

const std::string &OID_DATABASEFILENAME		= ".1.3.6.1.4.1.46821.2.1.1.1.0";
const std::string &OID_IP					= ".1.3.6.1.4.1.46821.2.1.1.2.0";
const std::string &OID_PORT					= ".1.3.6.1.4.1.46821.2.1.1.3.0";
const std::string &OID_STARTTIME			= ".1.3.6.1.4.1.46821.2.1.2.4.0";
const std::string &OID_REQUESTSPERHOUR		= ".1.3.6.1.4.1.46821.2.1.2.5.0";
const std::string &OID_LASTMAC				= ".1.3.6.1.4.1.46821.2.1.2.6.0";
const std::string &OID_LASTSSISIGNAL		= ".1.3.6.1.4.1.46821.2.1.2.7.0";
const std::string &OID_TOTALMAC				= ".1.3.6.1.4.1.46821.2.1.2.8.0";
const std::string &OID_DATABASEFILESIZE		= ".1.3.6.1.4.1.46821.2.1.2.9.0";
const std::string &OID_MEMORYPEAK			= ".1.3.6.1.4.1.46821.2.1.2.10.0";
const std::string &OID_MEMORYCURRENT		= ".1.3.6.1.4.1.46821.2.1.2.11.0";
const std::string &OID_ERRORCOUNT			= ".1.3.6.1.4.1.46821.2.1.2.12.0";

/**
 * Return first variable as string
 */
std::string getSNMPString1
(
		netsnmp_session *ss,
		const std::string &oidname
)
{
	// Create the PDU for the data for our request.
	netsnmp_pdu *pdu = snmp_pdu_create(SNMP_MSG_GET);

	oid anOID[MAX_OID_LEN];
	size_t anOID_len = MAX_OID_LEN;

	if (!snmp_parse_oid(oidname.c_str(), anOID, &anOID_len))
		return "";

	snmp_add_null_var(pdu, anOID, anOID_len);
	// Send the Request out.
	netsnmp_pdu *response;

	int status = snmp_synch_response(ss, pdu, &response);

	std::stringstream ostrm;

	// Process the response.
	if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
	{
		netsnmp_variable_list *vars = response->variables; // vars = // vars->next_variable)
		if (vars)
		{
			switch (vars->type)
			{
			case ASN_OCTET_STR:
				ostrm << std::string((const char*) vars->val.string, vars->val_len);
				break;
			case ASN_DOUBLE:
				ostrm << *vars->val.doubleVal;
				break;
			case ASN_COUNTER:
			case ASN_COUNTER64:
				ostrm << * (uint64_t*) vars->val.counter64;
				break;
			case ASN_BIT_STR:
				ostrm << *vars->val.bitstring;
				break;
			case ASN_FLOAT:
				ostrm << *vars->val.floatVal;
				break;
			case ASN_INTEGER:
			case ASN_INTEGER64:
				ostrm << *vars->val.integer;
				break;
			case ASN_OBJECT_ID:
				ostrm << *vars->val.objid;
				break;
			default:
				break;
			}
		}
	}
	// free the response
	if (response)
		snmp_free_pdu(response);
	return ostrm.str();
}

/**
 * Return first variable as integer
 */
int64_t getSNMPInt
(
		netsnmp_session *ss,
		const std::string &oidname
)
{
	// Create the PDU for the data for our request.
	netsnmp_pdu *pdu = snmp_pdu_create(SNMP_MSG_GET);

	oid anOID[MAX_OID_LEN];
	size_t anOID_len = MAX_OID_LEN;

	int64_t r = 0;

	if (!snmp_parse_oid(oidname.c_str(), anOID, &anOID_len))
		return r;

	snmp_add_null_var(pdu, anOID, anOID_len);
	// Send the Request out.
	netsnmp_pdu *response;

	int status = snmp_synch_response(ss, pdu, &response);

	// Process the response.
	if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
	{
		netsnmp_variable_list *vars = response->variables; // vars = // vars->next_variable)
		if (vars)
		{
			switch (vars->type)
			{
			case ASN_DOUBLE:
				r = *vars->val.doubleVal;
				break;
			case ASN_COUNTER:
			case ASN_COUNTER64:
				r = * (uint64_t*) vars->val.counter64;
				break;
			case ASN_FLOAT:
				r = *vars->val.floatVal;
				break;
			case ASN_INTEGER:
			case ASN_INTEGER64:
				r = *vars->val.integer;
				break;
			default:
				break;
			}
		}
	}
	// free the response
	if (response)
		snmp_free_pdu(response);
	return r;
}

int main(int argc, char ** argv)
{
	WacsSNMPConfig config(argc, argv);
	if (config.error())
		exit(config.error());
	
	netsnmp_session session;

	oid anOID[MAX_OID_LEN];
	size_t anOID_len;

	netsnmp_variable_list *vars;

	// Initialize the SNMP library
	init_snmp("wacs");

	// Initialize a "session" that defines who we're going to talk to
	memset(&session, 0, sizeof(netsnmp_session));
	snmp_sess_init(&session);
	session.peername = strdup(config.peer.c_str());

	// set the SNMP version number
	session.version = SNMP_VERSION_1;

	// set the SNMPv1 community name used for authentication
	session.community = (unsigned char*) config.community.c_str();
	session.community_len = strlen((char*) session.community);

	// Open the session
	SOCK_STARTUP;
	netsnmp_session *ss = snmp_open(&session);
	if (!ss)
	{
		snmp_sess_perror("ack", &session);
		SOCK_CLEANUP;
		exit(1);
	}

	std::string db = getSNMPString1(ss, OID_DATABASEFILENAME);
	std::string ip = getSNMPString1(ss, OID_IP);
	int64_t port = getSNMPInt(ss, OID_PORT);
	int64_t starttime = getSNMPInt(ss, OID_STARTTIME);
	int64_t requestsperhour= getSNMPInt(ss, OID_REQUESTSPERHOUR);
	std::string lastmac = getSNMPString1(ss, OID_LASTMAC);
	int64_t lastssisignal = getSNMPInt(ss, OID_LASTSSISIGNAL);
	int64_t totalmac = getSNMPInt(ss, OID_TOTALMAC);
	int64_t databasefilesize = getSNMPInt(ss, OID_DATABASEFILESIZE);
	int64_t memorypeak = getSNMPInt(ss, OID_MEMORYPEAK);
	int64_t memorycurrent  = getSNMPInt(ss, OID_MEMORYCURRENT);
	int64_t errorcount = getSNMPInt(ss, OID_ERRORCOUNT);

	time_t now = time(NULL);
	char st[30];
	strftime(st, sizeof(st), "%Y-%m-%d %H:%M:%S", localtime((time_t*)&starttime));

	int64_t mins = (now - starttime) / 60;
	int64_t hours = mins / 60;
	mins = mins - (hours * 60);

	if (config.verbosity > 0)
		std::cout << "DB\tIP\tPort\tStarted\tElapsed\tRequests/hour\tMAC\tSSI signal\tTotal MAC\tDB size, kB\tMem.peak, kB\tMem.current, kB" << std::endl;

	std::cout 
		<< db << "\t"
		<< ip << "\t"
		<< port << "\t"
		<< (char *) st << "\t"
		<< hours << ":" << std::setw(2) << std::setfill('0') << mins << "\t"
		<< requestsperhour << "\t"
		<< lastmac << "\t"
		<< lastssisignal << "\t"
		<< totalmac << "\t"
		<< databasefilesize << "\t"
		<< memorypeak << "\t"
		<< memorycurrent << "\t"
		<< errorcount
		<< std::endl;
	// Clean up: free the response, close the session.
	snmp_close(ss);

	SOCK_CLEANUP;
	return (0);
}
