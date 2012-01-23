#pragma once


#include <osc/OscReceivedElements.h>
#include <osc/OscPacketListener.h>
#include <osc/OscOutboundPacketStream.h>
#include <ip/UdpSocket.h>


// we use qt for cross plattform threading - still waiting for a c++11 :(
#include <QThread>

#include <iostream>


#define PORT 7000

extern float g_testValue;

class OSCDispatcher : public QThread, osc::OscPacketListener
{
public:
	OSCDispatcher()
	{
		m_socket = new UdpListeningReceiveSocket( IpEndpointName( IpEndpointName::ANY_ADDRESS, PORT ), this );
	}

	~OSCDispatcher()
	{
		delete m_socket;
	}
	
	virtual void ProcessMessage( const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint )
	{
		try{
			// example of parsing single messages. osc::OsckPacketListener
			// handles the bundle traversal.

			if( std::strcmp( m.AddressPattern(), "/test1" ) == 0 )
			{
				// example #1 -- argument stream interface
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				bool a1;
				osc::int32 a2;
				float a3;
				const char *a4;
				args >> a1 >> a2 >> a3 >> a4 >> osc::EndMessage;

				std::cout << "received '/test1' message with arguments: "
					<< a1 << " " << a2 << " " << a3 << " " << a4 << "\n";

			}else if( std::strcmp( m.AddressPattern(), "/test2" ) == 0 )
			{
				// example #2 -- argument iterator interface, supports
				// reflection for overloaded messages (eg you can call
				// (*arg)->IsBool() to check if a bool was passed etc).
				osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
				bool a1 = (arg++)->AsBool();
				int a2 = (arg++)->AsInt32();
				float a3 = (arg++)->AsFloat();
				const char *a4 = (arg++)->AsString();
				if( arg != m.ArgumentsEnd() )
					throw osc::ExcessArgumentException();

				std::cout << "received '/test2' message with arguments: "
					<< a1 << " " << a2 << " " << a3 << " " << a4 << "\n";
			}else if( std::strcmp( m.AddressPattern(), "/color" ) == 0 )
			{
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				args >> g_testValue >> osc::EndMessage;
			}
		}catch( osc::Exception& e )
		{
			// any parsing errors such as unexpected argument types, or
			// missing arguments get thrown as exceptions.
			std::cout << "error while parsing message: " << m.AddressPattern() << ": " << e.what() << "\n";
		}
	}

	void run()
	{
		m_socket->Run();
	}
	void stop()
	{
		m_socket->AsynchronousBreak();
	}

	UdpListeningReceiveSocket *m_socket;
};

