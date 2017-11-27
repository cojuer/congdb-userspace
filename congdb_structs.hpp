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

static int popcount(uint32_t v) {
    v = v - ((v >> 1) & 0x55555555);                // put count of each 2 bits into those 2 bits
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333); // put count of each 4 bits into those 4 bits  
    return (((v + (v >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
}

struct rule_id {
    uint32_t loc_ip;
    uint32_t loc_mask;
    uint32_t rem_ip;
    uint32_t rem_mask;
    uint8_t priority;

    operator std::string() const {
        return std::string("pr:") + std::to_string(priority) + " " + 
               uint_ip_to_str(loc_ip) + "/" + std::to_string(popcount(loc_mask)) + "<->" + 
               uint_ip_to_str(rem_ip) + "/" + std::to_string(popcount(rem_mask));
    }
};

struct rule_stats {
    uint32_t acks_num;
    uint32_t loss_num;
    uint32_t rtt;

    operator std::string() const {
        return std::string("acks_num:") + std::to_string(acks_num) + std::string("\n") + 
               std::string("loss_num:") + std::to_string(loss_num) + std::string("\n") + 
               std::string("rtt:") + std::to_string(rtt);
    }
};

struct congdb_entry {
    rule_id id;
    rule_stats stats;
    char* ca_name;

    operator std::string() const {
        return static_cast<std::string>(id) + std::string(":") + std::string(ca_name)
               + std::string("\n") + static_cast<std::string>(stats);
    }
};

using congdb_data = std::vector<congdb_entry>; 

} /* congdb namespace */
