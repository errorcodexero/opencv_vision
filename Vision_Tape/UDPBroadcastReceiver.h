#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <boost/asio.hpp>

class UDPBroadcastReceiver
{
public:
	UDPBroadcastReceiver(boost::asio::io_service &service, int port, size_t size)
		: m_service(service),
		  m_endpoint(boost::asio::ip::address_v4::any(), port),
		  m_socket(service)
	{
		boost::system::error_code ec;

		m_buffer.resize(size);
		m_socket.open(m_endpoint.protocol());
		m_socket.set_option(boost::asio::socket_base::reuse_address(true));
		m_socket.bind(m_endpoint, ec);
		m_socket.set_option(boost::asio::socket_base::reuse_address(true));
	}

	void receive(std::string &msg)
	{
		size_t count = m_socket.receive_from(boost::asio::buffer(m_buffer), m_endpoint);
		msg = std::string((const char *)&m_buffer[0], count);
	}

private:
	std::vector<uint8_t> m_buffer;
	boost::asio::io_service &m_service;
	boost::asio::ip::udp::socket m_socket;
	boost::asio::ip::udp::endpoint m_endpoint;
};

