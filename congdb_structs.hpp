#pragma once

#include <string>
#include <vector>

namespace congdb {

struct tcp_sock_data {
    uint32_t loc_ip;
    uint32_t rem_ip;

	operator std::string() const {
		return std::to_string(loc_ip) + "-" + std::to_string(rem_ip);
	}
};

struct congdb_entry {
    tcp_sock_data stats;
    char* ca_name;

	operator std::string() const {
		return static_cast<std::string>(stats) + std::string(ca_name);
	}
};

using congdb_data = std::vector<congdb_entry>; 

} /* congdb namespace */
