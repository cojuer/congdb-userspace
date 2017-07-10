#pragma once

#include <string>

namespace congdb {

struct tcp_sock_data {
    uint32_t loc_ip;
    uint32_t loc_port;
    uint32_t rem_ip;
    uint32_t rem_port;

	operator std::string() {
		return std::to_string(loc_ip) + ":" + std::to_string(loc_port) + "-" +
			   std::to_string(rem_ip) + ":" + std::to_string(rem_port);
	}
};

struct congdb_entry {
    tcp_sock_data stats;
    char* ca_name;

	operator std::string() {
		return static_cast<std::string>(stats) + std::string(ca_name);
	}
};

struct congdb_data {
    size_t size;
    congdb_entry *entries;
};

} /* congdb namespace */
