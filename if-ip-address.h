#include <string>
#include <vector> 

/**
 * Get the IP addresses of the machine
 * @param retAddress can be NULL
 * @param retInterface can be NULL
 * @see https://stackoverflow.com/questions/212528/get-the-ip-address-of-the-machine
 */
size_t ifIPAddress(std::vector<std::string> *retAddress, std::vector<std::string> *retInterface);

/**
 * Get the IP addresses comma delimited list of the machine
 * @param retAddress can be NULL
 * @param retInterface can be NULL
 * @see https://stackoverflow.com/questions/212528/get-the-ip-address-of-the-machine
 */
std::string ifIPAddressString();
