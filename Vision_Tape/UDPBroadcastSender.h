#pragma once

#include <string>
#include <strstream>
#include <boost/asio.hpp>
#include "UDPException.h"

class UDPBroadcastSender
{
public:
	UDPBroadcastSender(boost::asio::io_service &service, int port) 
		: m_service(service), 
		  m_socket(service),
		  //m_endpoint(boost::asio::ip::address_v4(0x0A464704), port)
		  m_endpoint(boost::asio::ip::address_v4::broadcast(), port)
	{
		boost::system::error_code error;

		m_socket.open(boost::asio::ip::udp::v4(), error);
		if (error)
		{
			std::string str("error opening UDP socket -");
			str += error.message();
			UDPException ex(str);
			throw ex;
		}

		m_socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
		m_socket.set_option(boost::asio::socket_base::broadcast(true));
	}

	void send(const std::string &msg)
	{
		m_socket.send_to(boost::asio::buffer(msg), m_endpoint);
	}

	virtual ~UDPBroadcastSender()
	{
		m_socket.close();
	}

private:
	boost::asio::io_service &m_service;
	boost::asio::ip::udp::socket m_socket;
	boost::asio::ip::udp::endpoint m_endpoint;
};
