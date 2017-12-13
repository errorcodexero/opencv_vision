#pragma once

#include <exception>
#include <string>

class UDPException : std::exception
{
public:
	UDPException(const char *msg_p)
	{
		m_what = msg_p;
	}

	UDPException(const std::string &msg)
	{
		m_what = msg;
	}

	virtual ~UDPException()
	{
	}

	virtual const char *what() const noexcept
	{
		return m_what.c_str();
	}

private:
	std::string m_what;
};
