#pragma once

#include <arpa/inet.h>

#include <iostream>
#include <string>
#include <vector>

namespace congdb {

static std::string uint_ip_to_str(uint32_t int_ip)
{
	char str_ip[16];
	if (inet_ntop(AF_INET, (uint32_t*)&int_ip, str_ip, 16)) {
		return std::string(str_ip);
	}
	return "Error: could not convert ip from uin32_t";
}

struct tcp_sock_data {
    uint32_t loc_ip;
    uint32_t rem_ip;

	operator std::string() const {
		return uint_ip_to_str(loc_ip) + "<->" + uint_ip_to_str(rem_ip);
	}
};

struct congdb_entry {
    tcp_sock_data stats;
    char* ca_name;

	operator std::string() const {
		return static_cast<std::string>(stats) + std::string(":") + std::string(ca_name);
	}
};

using congdb_data = std::vector<congdb_entry>; 

} /* congdb namespace */
