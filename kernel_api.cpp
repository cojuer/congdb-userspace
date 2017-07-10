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
	std::shared_ptr<nl_msg> msg = {nlmsg_alloc(), nlmsg_free};
	const int flags = NLM_F_REQUEST | NLM_F_ACK;
	if (!genlmsg_put(msg.get(), NL_AUTO_PID, NL_AUTO_SEQ, m_fam_id, 0, flags, CONGDB_C_ADD_ENTRY, VERSION))
		throw std::runtime_error("Failed to put message");
	if (nla_put_u32(msg.get(), CONGDB_A_LOC_IP, sock_data.loc_ip < 0))
		throw std::runtime_error("Failed to put local ip");
	if (nla_put_u32(msg.get(), CONGDB_A_LOC_PORT, sock_data.loc_port) < 0)
		throw std::runtime_error("Failed to put local port");
	if (nla_put_u32(msg.get(), CONGDB_A_REM_IP, sock_data.rem_ip < 0))
		throw std::runtime_error("Failed to put remote ip");
	if (nla_put_u32(msg.get(), CONGDB_A_REM_PORT, sock_data.rem_port < 0))
		throw std::runtime_error("Failed to put remote port");
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
	if (nla_put_u32(msg.get(), CONGDB_A_LOC_IP, sock_data.loc_ip < 0))
		throw std::runtime_error("Failed to put local ip");
	if (nla_put_u32(msg.get(), CONGDB_A_LOC_PORT, sock_data.loc_port) < 0)
		throw std::runtime_error("Failed to put local port");
	if (nla_put_u32(msg.get(), CONGDB_A_REM_IP, sock_data.rem_ip < 0))
		throw std::runtime_error("Failed to put remote ip");
	if (nla_put_u32(msg.get(), CONGDB_A_REM_PORT, sock_data.rem_port < 0))
		throw std::runtime_error("Failed to put remote port");
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

congdb_data CONGDBKernelAPI::receive_entries() 
{
	// FIXME: fix me
	return congdb_data();
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
