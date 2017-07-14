#pragma once

#include <string>
#include <vector>

#include <libnl3/netlink/attr.h>
#include <libnl3/netlink/handlers.h>
#include <libnl3/netlink/msg.h>
#include <libnl3/netlink/netlink.h>
#include <libnl3/netlink/socket.h>
#include <libnl3/netlink/genl/ctrl.h>
#include <libnl3/netlink/genl/genl.h>

#include "congdb_structs.hpp"

namespace congdb {

enum congdb_nl_cmds {
    CONGDB_C_UNSPEC,

    CONGDB_C_ADD_ENTRY,
    CONGDB_C_DEL_ENTRY,
    CONGDB_C_CLEAR_ENTRIES,

    CONGDB_C_LIST_ENTRIES,

    // used to verify nl commands
	__CONGDB_C_MAX,
	CONGDB_C_MAX = __CONGDB_C_MAX - 1
};

enum congdb_nl_attrs {
	CONGDB_A_UNSPEC,
    
    CONGDB_A_LOC_IP,
    CONGDB_A_REM_IP,

    CONGDB_A_CA,

	// used to verify nl attributes
	__CONGDB_A_MAX,
	CONGDB_A_MAX = __CONGDB_A_MAX - 1
};

class CONGDBKernelAPI {
public:
	CONGDBKernelAPI();
	~CONGDBKernelAPI();
	
	congdb_data list_entries();
	
    void add_entry(const tcp_sock_data& sock_data, const std::string &ca_name);
    void del_entry(const tcp_sock_data& sock_data);
    void clear_entries();

private: /* methods */
    void request_entries();

    congdb_data receive_entries();

private: /* members */
    nl_sock* m_sock;
    int      m_fam_id;

    constexpr static int VERSION = 1;
};

} /* congdb namespace */
