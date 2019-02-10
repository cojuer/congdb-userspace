#include <iostream>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include <climits>

#include <libnl3/netlink/attr.h>
#include <libnl3/netlink/handlers.h>
#include <libnl3/netlink/msg.h>
#include <libnl3/netlink/netlink.h>
#include <libnl3/netlink/socket.h>
#include <libnl3/netlink/genl/ctrl.h>
#include <libnl3/netlink/genl/genl.h>

#include <arpa/inet.h>

#include "kernel_api.hpp"

namespace congdb {

static CONGDBKernelAPI kernel_api;

static uint32_t str_ip_to_uint(const char* str_ip)
{
    uint32_t ip;
    if (inet_pton(AF_INET, str_ip, (uint32_t*)&ip) == 1) {
        return ip;
    }
    return 0;
}

struct ip_addr
{
    uint32_t ip;
    uint32_t mask;
};

ip_addr str_to_ip_addr(char* input)
{
    auto separator = strchr(input, '/');
    if (separator) {
        auto iplen = separator - input;
        auto ipbuf = (char*)malloc(iplen + 1);
        memcpy(ipbuf, input, iplen);
        ipbuf[iplen] = 0;
        auto masklen = strlen(input) - strlen(ipbuf);
        auto maskbuf = (char*)malloc(masklen + 1);
        memcpy(maskbuf, separator + 1, masklen);
        maskbuf[masklen] = 0;
        uint32_t mask = 0;
        for (int i = 0; i < atoi(maskbuf); ++i)
        {
            mask += 1 << i;
        }
        ip_addr result{ str_ip_to_uint(ipbuf), mask };
        free(ipbuf);
        free(maskbuf);
        return result;
    }
    else {
        return { str_ip_to_uint(input), UINT_MAX };
    }
}

constexpr auto argc_to_add = 5;
int add_entry(int argc, char **argv) {
    if (argc != argc_to_add) {
        std::cout << "Usage: " << argv[0]
            << " <local_ip> <remote_ip> <cong_algo>" << std::endl;
        return -1;
    }
    rule_id id;
    auto loc_addr = str_to_ip_addr(argv[2]);
    id.loc_ip = loc_addr.ip;
    id.loc_mask = loc_addr.mask;
    
    auto rem_addr = str_to_ip_addr(argv[3]);
    id.rem_ip = rem_addr.ip;
    id.rem_mask = rem_addr.mask;
    
    id.priority = 0;
    std::string ca_name = argv[4];
    kernel_api.add_entry(id, ca_name);
    return 0;
}

constexpr auto argc_to_set = 5;
int set_entry(int argc, char **argv) {
    if (argc != argc_to_set) {
        std::cout << "Usage: " << argv[0]
            << " <local_ip> <remote_ip> <cong_algo>" << std::endl;
        return -1;
    }
    rule_id id;
    auto loc_addr = str_to_ip_addr(argv[2]);
    id.loc_ip = loc_addr.ip;
    id.loc_mask = loc_addr.mask;

    auto rem_addr = str_to_ip_addr(argv[3]);
    id.rem_ip = rem_addr.ip;
    id.rem_mask = rem_addr.mask;

    id.priority = 0;
    std::string ca_name = argv[4];
    kernel_api.set_entry(id, ca_name);
    return 0;
}

constexpr auto argc_to_del = 4;
int del_entry(int argc, char **argv) {
    if (argc != argc_to_del) {
        std::cout << "Usage: " << argv[0]
            << " <local_ip> <remote_ip>" << std::endl;
        return -1;
    }
    rule_id id;
    auto loc_addr = str_to_ip_addr(argv[2]);
    id.loc_ip = loc_addr.ip;
    id.loc_mask = loc_addr.mask;

    auto rem_addr = str_to_ip_addr(argv[3]);
    id.rem_ip = rem_addr.ip;
    id.rem_mask = rem_addr.mask;

    id.priority = 0;
    kernel_api.del_entry(id);
    return 0;
}

constexpr auto argc_to_get = 4;
congdb_entry get_entry(int argc, char **argv) {
    if (argc != argc_to_del) {
        std::cout << "Usage: " << argv[0]
            << " <local_ip> <remote_ip>" << std::endl;
        return congdb_entry();
    }
    rule_id id;
    auto loc_addr = str_to_ip_addr(argv[2]);
    id.loc_ip = loc_addr.ip;
    id.loc_mask = loc_addr.mask;

    auto rem_addr = str_to_ip_addr(argv[3]);
    id.rem_ip = rem_addr.ip;
    id.rem_mask = rem_addr.mask;

    id.priority = 0;
    return kernel_api.get_entry(id);
}

int run_db_op(int argc, char **argv) 
{
    if (argc < 2) {
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
    else if (cmd == "set-entry") {
        set_entry(argc, argv);
    }
    else if (cmd == "clear-entries") {
        kernel_api.clear_entries();
    }
    else if (cmd == "list-entries") {
        auto entries = kernel_api.list_entries();
        for (auto& entry : entries) {
            std::cout << std::string(entry) << std::endl;
        }
    }
    else if (cmd == "get-entry") {
        auto entry = get_entry(argc, argv);
        std::cout << std::string(entry) << std::endl;
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
        {"get-entry",     congdb::run_db_op},
        {"set-entry",     congdb::run_db_op},
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
