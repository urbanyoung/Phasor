//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>

using boost::asio::ip::tcp;

namespace remote 
{
	class packet
	{
	public:

		static const size_t max_body_length = 2048;
	private:
		struct s_packet
		{
			struct s_header
			{
				unsigned short length;
				unsigned short opcode;
			} header;
			char body[max_body_length];
		};

		s_packet data;

	public:
		const char* header() const { return (const char*)&data.header;	}
		char* header() { return (char*)&data.header; }
		const char* body() const { return data.body; }
		char* body() { return data.body; }
		size_t body_length() const { return data.header.length - header_length; }

		const char* all() const { return (const char*)&data; }
		char* all() { return (char*)&data; }
		size_t length() const { return data.header.length + header_length; }

		bool validate_header() 
		{
			return length <= max_body_length;
		}

		static const size_t header_length = sizeof(s_packet::s_header);

	};

	class connection
	{
	public:

		typedef std::shared_ptr<connection> pointer;

		connection(boost::asio::io_service& io_service,
			std::function<void (const packet& p, connection::pointer)> received_callback)
		: io_service_(io_service),
		  socket_(io_service),
		  received_callback(received_callback)
		{
		}

		void write(const packet& p)
		{
			io_service_.post(boost::bind(&connection::do_write, this, p));
		}

		void close()
		{
			io_service_.post(boost::bind(&connection::do_close, this));
		}

		tcp::socket& socket()
		{
			return socket_;
		}
		
	protected:

		void wait_for_header()
		{
			boost::asio::async_read(socket_,
				boost::asio::buffer(read_packet.header(), packet::header_length),
				boost::bind(&connection::handle_read_header, this,
				boost::asio::placeholders::error));
		}

	private:
		void wait_for_write(const packet& p)
		{
			boost::asio::async_write(socket_,
				boost::asio::buffer(p.all(), p.length()),
				boost::bind(&connection::handle_write, this,
				boost::asio::placeholders::error));
		}

		void handle_read_header(const boost::system::error_code& error)
		{
			if (!error && read_packet.validate_header())
			{
				boost::asio::async_read(socket_,
					boost::asio::buffer(read_packet.body(), read_packet.body_length()),
					boost::bind(&connection::handle_read_body, this,
					boost::asio::placeholders::error));	
			}
			else
			{
				do_close();
			}
		}

		void handle_read_body(const boost::system::error_code& error)
		{
			if (!error)
			{
				//std::cout.write(read_msg_.body(), read_msg_.body_length());
				//std::cout << "\n";
				received_callback(read_packet, )
				
				wait_for_header();
			}
			else
			{
				do_close();
			}
		}

		void do_write(packet p)
		{
			bool write_in_progress = !packet_queue.empty();
			packet_queue.push_back(p);
			if (!write_in_progress)
			{
				wait_for_write(packet_queue.front());
			}
		}

		void handle_write(const boost::system::error_code& error)
		{
			if (!error)
			{
				// one write is done, check if there's another
				packet_queue.pop_front();
				if (!packet_queue.empty())
				{
					wait_for_write(packet_queue.front());
				}
			}
			else
			{
				do_close();
			}
		}

		void do_close()
		{
			socket_.close();
		}

	private:
		boost::asio::io_service& io_service_;
		tcp::socket socket_;
		packet read_packet;
		std::list<packet> packet_queue;
		std::function<void (const packet& p, connection::pointer)> received_callback;
	};

	class client : public connection
	{
	public:
		client(boost::asio::io_service& io_service,
			tcp::resolver::iterator endpoint_iterator)
			: connection(io_service, 
			std::bind(&client::received_packet, this, _1, _2))
		{
			boost::asio::async_connect(socket(), endpoint_iterator,
				boost::bind(&client::handle_connect, this,
				boost::asio::placeholders::error));
		}

		void handle_connect(const boost::system::error_code& error)
		{
			if (!error)
			{
				// wait for first packet from server
				wait_for_header();
			}
		}

		void received_packet(const packet& p, connection::pointer con)
		{

		}
	};


	class server
	{
	private:
		tcp::acceptor acceptor;

		void start_accept()
		{
			connection::pointer new_connection(
				new connection(acceptor.get_io_service(),
				std::bind(&client::received_packet, this, _1, _2)));

			acceptor.async_accept(new_connection->socket(),
				boost::bind(&server::handle_accept, this, new_connection,
				boost::asio::placeholders::error));
		}

		void handle_accept(connection::pointer new_connection,
			const boost::system::error_code& error)
		{
			if (!error)
			{
				// send first packet
				// make sure new_connection is retained somewhere (maybe in connection?)
			}

			start_accept();
		}

	public:
		server(boost::asio::io_service& io_service, unsigned short port)
			: acceptor(io_service, tcp::endpoint(tcp::v4(), port))
		{
			start_accept();
		}

		void received_packet(const packet& p, connection::pointer con)
		{

		}

	};
}

int main()
{
	try
	{
		boost::asio::io_service io_service;
		tcp_server server(io_service);
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}