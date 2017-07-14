#include "kernel_api.hpp"

#include <iostream>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace congdb {

CONGDBKernelAPI::CONGDBKernelAPI() 
{
	m_sock = nl_socket_alloc();
	if (!m_sock) throw std::runtime_error("Failed to allocate nl socket");
	if (genl_connect(m_sock) != 0) {
		nl_socket_free(m_sock);
		throw std::runtime_error("Failed to connect nl socket");
	}

	m_fam_id = genl_ctrl_resolve(m_sock, "CONGDB_MANAGER");
	if (m_fam_id <= 0) {
		nl_close(m_sock);
		nl_socket_free(m_sock);
		throw std::runtime_error("Failed to resolve nl family");
	}
}

void CONGDBKernelAPI::add_entry(const tcp_sock_data& sock_data, 
							   	const std::string& ca_name) 
{
	std::cout << "add entry " << std::string(sock_data) << std::endl;
	std::shared_ptr<nl_msg> msg = {nlmsg_alloc(), nlmsg_free};
	const int flags = NLM_F_REQUEST | NLM_F_ACK;
	if (!genlmsg_put(msg.get(), NL_AUTO_PID, NL_AUTO_SEQ, m_fam_id, 0, flags, CONGDB_C_ADD_ENTRY, VERSION))
		throw std::runtime_error("Failed to put message");
	if (nla_put_u32(msg.get(), CONGDB_A_LOC_IP, sock_data.loc_ip) < 0)
		throw std::runtime_error("Failed to put local ip");
	if (nla_put_u32(msg.get(), CONGDB_A_REM_IP, sock_data.rem_ip) < 0)
		throw std::runtime_error("Failed to put remote ip");
	if (nla_put_string(msg.get(), CONGDB_A_CA, ca_name.c_str()) < 0)
		throw std::runtime_error("Failed to put congestion algorithm name");
	if (nl_send_auto(m_sock, msg.get()) < 0)
		throw std::runtime_error("Failed to send request");
	nl_wait_for_ack(m_sock);
}

void CONGDBKernelAPI::del_entry(const tcp_sock_data& sock_data) 
{
	std::shared_ptr<nl_msg> msg = {nlmsg_alloc(), nlmsg_free};
	const int flags = NLM_F_REQUEST | NLM_F_ACK;
	if (!genlmsg_put(msg.get(), NL_AUTO_PID, NL_AUTO_SEQ, m_fam_id, 0, flags, CONGDB_C_DEL_ENTRY, VERSION))
		throw std::runtime_error("Failed to put message");
	if (nla_put_u32(msg.get(), CONGDB_A_LOC_IP, sock_data.loc_ip) < 0)
		throw std::runtime_error("Failed to put local ip");
	if (nla_put_u32(msg.get(), CONGDB_A_REM_IP, sock_data.rem_ip) < 0)
		throw std::runtime_error("Failed to put remote ip");
	if (nl_send_auto(m_sock, msg.get()) < 0)
		throw std::runtime_error("Failed to send request");
	nl_wait_for_ack(m_sock);
}

void CONGDBKernelAPI::clear_entries() 
{
	std::shared_ptr<nl_msg> msg = {nlmsg_alloc(), nlmsg_free};
	const int flags = NLM_F_REQUEST | NLM_F_ACK;
	if (!genlmsg_put(msg.get(), NL_AUTO_PID, NL_AUTO_SEQ, m_fam_id, 0, flags, CONGDB_C_CLEAR_ENTRIES, VERSION))
		throw std::runtime_error("Failed to put message");
	if (nl_send_auto(m_sock, msg.get()) < 0)
		throw std::runtime_error("Failed to send request");
	if (nl_wait_for_ack(m_sock) < 0)
		throw std::runtime_error("Failed to receive ack");
}

void CONGDBKernelAPI::request_entries() 
{
	std::shared_ptr<nl_msg> msg = {nlmsg_alloc(), nlmsg_free};
	const int flags = NLM_F_REQUEST;
	if (!genlmsg_put(msg.get(), NL_AUTO_PID, NL_AUTO_SEQ, m_fam_id, 0, flags, CONGDB_C_LIST_ENTRIES, VERSION))
		throw std::runtime_error("Failed to put message");
	if (nl_send_auto(m_sock, msg.get()) < 0)
		throw std::runtime_error("Failed to send request");
}

// TODO: create map: typeid <-> nla_get, nla_put?

congdb_data CONGDBKernelAPI::receive_entries() 
{
	auto parser = [](nl_msg* msg, void *object) -> int {
		auto& entries = *static_cast<std::vector<congdb_entry>*>(object);
		genlmsghdr *hdr = genlmsg_hdr(nlmsg_hdr(msg));
		
		if (hdr->cmd != CONGDB_C_LIST_ENTRIES) return -1;
		nlattr *attrs = genlmsg_attrdata(hdr, 0);
		nlattr *attr = attrs;
		int len = genlmsg_attrlen(hdr, 0);
		int rem = len;

		while (nla_ok(attr, rem)) {
			if (nla_type(attr) != CONGDB_A_LOC_IP) return -1;
			uint32_t loc_ip{ nla_get_u32(attr) };
			attr = nla_next(attr, &rem);

			if (!nla_ok(attr, rem)) return -1;
			if (nla_type(attr) != CONGDB_A_REM_IP) return -1;
			uint32_t rem_ip{ nla_get_u32(attr) };
			attr = nla_next(attr, &rem);

			if (!nla_ok(attr, rem)) return -1;
			if (nla_type(attr) != CONGDB_A_CA) return -1;
			std::string ca_name{ nla_get_string(attr) };
			attr = nla_next(attr, &rem);

	  		congdb_entry entry;
			entry.stats.loc_ip = loc_ip;
			entry.stats.rem_ip = rem_ip;
			entry.ca_name = (char*)malloc(ca_name.size() * sizeof(char));
			strcpy(entry.ca_name, ca_name.c_str());
  			entries.push_back(entry);
		}

		if (rem) return -1;
		if ((nlmsg_hdr(msg)->nlmsg_flags & NLM_F_MULTI) &&
				(nlmsg_hdr(msg)->nlmsg_type != NLMSG_DONE)) {
			return NL_OK;
		}
		return NL_STOP;
	};

	std::vector<congdb_entry> entries;
	if (nl_socket_modify_cb(m_sock, NL_CB_MSG_IN, NL_CB_CUSTOM, parser, &entries) < 0)
		throw std::runtime_error("Failed to change the default nl message parser");
	if (nl_recvmsgs_default(m_sock) < 0)
		throw std::runtime_error("Failed to receive message");
	nl_wait_for_ack(m_sock);
	return entries;
}

congdb_data CONGDBKernelAPI::list_entries()
{
	request_entries();
	return receive_entries();
}

CONGDBKernelAPI::~CONGDBKernelAPI() 
{
	nl_close(m_sock);
	nl_socket_free(m_sock);
}

} /* fdmpd namespace */
