#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sstream>
#include "if-ip-address.h"

/**
 * Get the IP address of the machine
 * @param retAddress can be NULL
 * @param retInterface can be NULL
 * @see https://stackoverflow.com/questions/212528/get-the-ip-address-of-the-machine
 */
size_t ifIPAddress(std::vector<std::string> *retAddress, std::vector<std::string> *retInterface) 
{
	size_t r = 0;
	struct ifaddrs *ifAddrStruct = NULL;
	struct ifaddrs *ifa = NULL;
	void *tmpAddrPtr = NULL;
	getifaddrs(&ifAddrStruct);
	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) 
	{
		if (!ifa->ifa_addr)
			continue;
		if (ifa->ifa_addr->sa_family == AF_INET)
		{ 
			// check is it a valid IP4 Address
			tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			if (retAddress)
				retAddress->push_back(std::string(addressBuffer));
			if (retInterface)
				retInterface->push_back(std::string(ifa->ifa_name));
		} else if (ifa->ifa_addr->sa_family == AF_INET6) {
			// check is it a valid IP6 Address
			tmpAddrPtr = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
			char addressBuffer[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
			if (retAddress)
				retAddress->push_back(std::string(addressBuffer));
			if (retInterface)
				retInterface->push_back(std::string(ifa->ifa_name));
		}
		r++;
	}
	if (ifAddrStruct) 
		freeifaddrs(ifAddrStruct);
	return r;
}

/**
 * Get the IP addresses comma delimited list of the machine
 * @param retAddress can be NULL
 * @param retInterface can be NULL
 * @see https://stackoverflow.com/questions/212528/get-the-ip-address-of-the-machine
 */
std::string ifIPAddressString()
{
	std::stringstream r;
	std::vector<std::string> retAddress;
	ifIPAddress(&retAddress, NULL);
	for (std::vector<std::string>::const_iterator it = retAddress.begin(); it != retAddress.end(); ++it)
	{
		r << *it << ", ";
	}
	std::string rr(r.str());
	if (rr.size() > 2)
		rr = rr.substr(0, rr.size() - 2);
	return rr;
}
