#include <iostream>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include <libnl3/netlink/attr.h>
#include <libnl3/netlink/handlers.h>
#include <libnl3/netlink/msg.h>
#include <libnl3/netlink/netlink.h>
#include <libnl3/netlink/socket.h>
#include <libnl3/netlink/genl/ctrl.h>
#include <libnl3/netlink/genl/genl.h>

#include "kernel_api.hpp"

namespace congdb {

static CONGDBKernelAPI kernel_api;

static uint32_t str_ip_to_uint(const char* str_ip)
{
	uint32_t ip;
	//if (!in4_pton(str_ip, -1, (u8*)&ip, -1, NULL))
	//	return 0;
	return ip;
}

constexpr auto argc_to_add = 6;
int add_entry(int argc, char **argv) {
	if (argc != argc_to_add) {
		std::cout << "Usage: " << argv[0]
			<< " <local_ip> <local_port> <remote_ip> <remote_port> <cong_algo>" << std::endl;
		return -1;
	}
	tcp_sock_data sock_data = {
		str_ip_to_uint(argv[2]),
		std::stoul(argv[3]),
		str_ip_to_uint(argv[4]),
		std::stoul(argv[5])
	};
	std::string ca_name = argv[6];
	kernel_api.add_entry(sock_data, ca_name);
	return 0;
}

constexpr auto argc_to_del = 5;
int del_entry(int argc, char **argv) {
	if (argc != argc_to_del) {
		std::cout << "Usage: " << argv[0]
			<< " <local_ip> <local_port> <remote_ip> <remote_port>" << std::endl;
		return -1;
	}
	tcp_sock_data sock_data = {
		str_ip_to_uint(argv[2]),
		std::stoul(argv[3]),
		str_ip_to_uint(argv[4]),
		std::stoul(argv[5])	
	};
	kernel_api.del_entry(sock_data);
	return 0;
}

int run_db_op(int argc, char **argv) 
{
	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " " << argv[1] << std::endl;
		return -EINVAL;
	}

	std::string cmd{ argv[1] };
	if (cmd == "add-entry") {
		add_entry(argc, argv);
	}
	else if (cmd == "del-entry") {
		del_entry(argc, argv);
	}
	else if (cmd == "clear-remotes") {
		kernel_api.clear_entries();
	}
	else if (cmd == "list-remotes") {
		auto entries = kernel_api.list_entries();
		
	}
	return 0;
}

} /* congdb namespace */

int main(int argc, char **argv) {
    using Handler = std::function<int(int, char**)>;
	const std::unordered_map<std::string, Handler> commands = {
		{"list-entries",  congdb::run_db_op},
		{"add-entry",     congdb::run_db_op},
		{"del-entry",     congdb::run_db_op},
		{"clear-entries", congdb::run_db_op},
	};

	if (argc > 1 && commands.count(argv[1])) {
		return commands.at(argv[1])(argc, argv);
	}

	std::cout << "Usage: " << argv[0] << " <command> <options>" << std::endl;
	std::cout << "Supported commands:" << std::endl;
	for(auto& pair : commands) {
		std::cout << pair.first << std::endl;
	}
	return -1;
}
