/*
This file is a part of MonaSolutions Copyright 2017
mathieu.poux[a]gmail.com
jammetthomas[a]gmail.com

This program is free software: you can redistribute it and/or
modify it under the terms of the the Mozilla Public License v2.0.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
Mozilla Public License v. 2.0 received along this program for more
details (or else see http://mozilla.org/MPL/2.0/).

*/

#pragma once

#include "Mona/Mona.h"
#include "Mona/Net/SocketAddress.h"
#include "Mona/Util/ByteRate.h"
#include "Mona/Memory/Packet.h"
#include "Mona/Threading/Handler.h"
#include "Mona/Util/Parameters.h"
#include <deque>

namespace Mona {

struct Socket : virtual Object, Net::Stats {
	typedef Event<void(Shared<Buffer>& pBuffer, const SocketAddress& address)>	  OnReceived;
	typedef Event<void(const Shared<Socket>& pSocket)>							  OnAccept;
	typedef Event<void(const Exception&)>										  OnError; const OnError& onError;
	typedef Event<void()>														  OnFlush;
	typedef Event<void()>														  OnDisconnection;

	/*!
	Decoder offers to decode data in the reception thread when socket is used with IOSocket
	If pBuffer is reseted, no onReceived is callen (data captured),
	/!\ pSocket must never be "attached" to the decoder in a instance variable otherwise a memory leak could happen (however a weak attachment stays acceptable) */
	struct Decoder : virtual Object {
		virtual void decode(Shared<Buffer>& pBuffer, const SocketAddress& address, const Shared<Socket>& pSocket) = 0;
		virtual void onRelease(Socket& socket) {}
	};

	enum Type {
		TYPE_STREAM = SOCK_STREAM,
		TYPE_DATAGRAM = SOCK_DGRAM,
		TYPE_OTHER = 0x10
	};

	enum ShutdownType {
		SHUTDOWN_RECV = 0,
		SHUTDOWN_SEND = 1,
		SHUTDOWN_BOTH = 2
	};

	enum {
		BACKLOG_MAX = 200 // blacklog maximum, see http://tangentsoft.net/wskfaq/advanced.html#backlog
	};

	/*!
	Creates a Socket which supports IPv4 and IPv6 */
	Socket(Type type);
	virtual ~Socket();

	const	Type		type;

	NET_SOCKET			id() const { return _id; }
	operator NET_SOCKET() const { return _id; }

	virtual bool		isSecure() const { return false; }

	Time				recvTime() const { return _recvTime.load(); }
	uint64_t				recvByteRate() const { return _recvByteRate; }
	Time				sendTime() const { return _sendTime.load(); }
	uint64_t				sendByteRate() const { return _sendByteRate; }

	uint32_t				recvBufferSize() const { return _recvBufferSize; }
	uint32_t				sendBufferSize() const { return _sendBufferSize; }

	virtual uint32_t		available() const;
	uint64_t				queueing() const { return _queueing; }
	
	const SocketAddress& address() const;
	const SocketAddress& peerAddress() const { return _peerAddress; }
	bool				 listening() const { return _listening; }

	virtual bool processParams(Exception& ex, const Parameters& parameter, const char* prefix = "net.");

	virtual bool setSendBufferSize(Exception& ex, uint32_t size);
	virtual bool getSendBufferSize(Exception& ex, uint32_t& size) const { return getOption(ex,SOL_SOCKET, SO_SNDBUF, size); }
	
	virtual bool setRecvBufferSize(Exception& ex, uint32_t size);
	virtual bool getRecvBufferSize(Exception& ex, uint32_t& size) const { return getOption(ex, SOL_SOCKET, SO_RCVBUF, size); }

	bool setNoDelay(Exception& ex, bool value) { return setOption(ex,IPPROTO_TCP, TCP_NODELAY, value ? 1 : 0); }
	bool getNoDelay(Exception& ex, bool& value) const { return getOption(ex, IPPROTO_TCP, TCP_NODELAY, value); }

	bool setKeepAlive(Exception& ex, bool value) { return setOption(ex, SOL_SOCKET, SO_KEEPALIVE, value ? 1 : 0); }
	bool getKeepAlive(Exception& ex, bool& value) const { return getOption(ex,SOL_SOCKET, SO_KEEPALIVE, value); }

	virtual bool setReuseAddress(Exception& ex, bool value) { return setOption(ex, SOL_SOCKET, SO_REUSEADDR, value ? 1 : 0); }
	virtual bool getReuseAddress(Exception& ex, bool& value) const { return getOption(ex, SOL_SOCKET, SO_REUSEADDR, value); }

	bool setOOBInline(Exception& ex, bool value) { return setOption(ex, SOL_SOCKET, SO_OOBINLINE, value ? 1 : 0); }
	bool getOOBInline(Exception& ex, bool& value) const { return getOption(ex, SOL_SOCKET, SO_OOBINLINE, value); }

	bool setBroadcast(Exception& ex, bool value) { return setOption(ex, SOL_SOCKET, SO_BROADCAST, value ? 1 : 0); }
	bool getBroadcast(Exception& ex, bool& value) const { return getOption(ex, SOL_SOCKET, SO_BROADCAST, value); }

	virtual bool setLinger(Exception& ex, bool on, int seconds);
	virtual bool getLinger(Exception& ex, bool& on, int& seconds) const;
	
	void setReusePort(bool value);
	bool getReusePort() const;

	virtual bool setNonBlockingMode(Exception& ex, bool value);
	bool getNonBlockingMode() const { return _nonBlockingMode; }

	bool joinGroup(Exception& ex, const IPAddress& ip, uint32_t interfaceIndex=0);
	void leaveGroup(const IPAddress& ip, uint32_t interfaceIndex = 0);

	virtual bool accept(Exception& ex, Shared<Socket>& pSocket);

	/*!
	Connect or disconnect (if address is Wildcard) to a peer address */
	virtual bool connect(Exception& ex, const SocketAddress& address, uint16_t timeout=0);
	/*!
	Bind socket, if socket is datagram and the address passed is a multicast ip it join the multicast group related (call joinGroup) */
	virtual bool bind(Exception& ex, const SocketAddress& address);
	/*!
	Bind on any available port */
	bool		 bind(Exception& ex, const IPAddress& ip=IPAddress::Wildcard()) { return bind(ex, SocketAddress(ip, 0)); }
	virtual bool listen(Exception& ex, int backlog = SOMAXCONN);
	bool		 shutdown(ShutdownType type = SHUTDOWN_BOTH);
	
	int			 receive(Exception& ex, char* buffer, uint32_t size, int flags = 0) { return receive(ex, buffer, size, flags, NULL); }
	int			 receiveFrom(Exception& ex, char* buffer, uint32_t size, SocketAddress& address, int flags = 0)  { return receive(ex, buffer, size, flags, &address); }

	int			 send(Exception& ex, const char* data, uint32_t size, int flags = 0) { return sendTo(ex, data, size, SocketAddress::Wildcard(), flags); }
	virtual int	 sendTo(Exception& ex, const char* data, uint32_t size, const SocketAddress& address, int flags=0);

	/*!
	Sequential and safe writing, can queue data if can't send immediatly (flush required on onFlush event)
	Returns size of data sent immediatly (or -1 if error, for TCP socket a SHUTDOWN_SEND is done, so socket will be disconnected) */
	int			 write(Exception& ex, const Packet& packet, int flags = 0) { return write(ex, packet, SocketAddress::Wildcard(), flags); }
	int			 write(Exception& ex, const Packet& packet, const SocketAddress& address, int flags = 0);

	bool		 flush(Exception& ex) { return flush(ex, false); }

	template <typename ...Args>
	static Exception& SetException(int error, Exception& ex, Args&&... args) {
		if (!error)
			error = Net::LastError();
		ex.set<Ex::Net::Socket>(Net::ErrorToMessage(error), std::forward<Args>(args)...).code = error;
		return ex;
	}
	template <typename ...Args>
	static Exception& SetException(Exception& ex, Args&&... args) { return SetException(Net::LastError(), ex, std::forward<Args>(args)...); }

protected:
	void init();
	
	// Create a socket from Socket::accept
	Socket(NET_SOCKET id, const sockaddr& addr, Type type=TYPE_STREAM);
	virtual Socket* newSocket(Exception& ex, NET_SOCKET sockfd, const sockaddr& addr) { return new Socket(sockfd, (sockaddr&)addr); }
	virtual int		receive(Exception& ex, char* buffer, uint32_t size, int flags, SocketAddress* pAddress);


	void			send(uint32_t count) { _sendTime = Time::Now(); _sendByteRate += count; }
	void			receive(uint32_t count) { _recvTime = Time::Now(); _recvByteRate += count; }
	virtual bool	flush(Exception& ex, bool deleting);
	virtual bool	close(ShutdownType type = SHUTDOWN_BOTH) { return ::shutdown(_id, type) == 0; }

	template<typename Type, typename = typename std::enable_if<std::is_arithmetic<Type>::value && !std::is_same<Type, bool>::value>::type>
	bool processParam(const Parameters& parameters, const char* name, Type& value, const char* prefix = NULL) {
		return (prefix && parameters.getNumber(String(prefix, name), value)) || parameters.getNumber(name, value);
	}
	bool processParam(const Parameters& parameters, const char* name, std::string& value, const char* prefix = NULL) { return (prefix && parameters.getString(String(prefix, name), value)) || parameters.getString(name, value); }
	bool processParam(const Parameters& parameters, const char* name, bool& value, const char* prefix = NULL) { return (prefix && parameters.getBoolean(String(prefix, name), value)) || parameters.getBoolean(name, value); }
	

	volatile bool			_listening;
	volatile bool			_nonBlockingMode;
	mutable SocketAddress	_address;
	SocketAddress			_peerAddress;
	NET_SOCKET				_id;

	mutable std::atomic<int>	_recvBufferSize;
	mutable std::atomic<int>	_sendBufferSize;

private:
	virtual bool setIPV6Only(Exception& ex, bool enable) { return setOption(ex, IPPROTO_IPV6, IPV6_V6ONLY, enable ? 1 : 0); }
	virtual void computeAddress();

	template<typename Type>
	bool getOption(Exception& ex, int level, int option, Type& value) const {
		if (_ex) {
			if (_ex.cast<Ex::Intern>())
				ex.set<Ex::Unsupported>("Option ", option," of type ", typeOf<Type>()," not supported by ", typeOf(self));
			else
				ex = _ex;
			return false;
		}
        NET_SOCKLEN length(sizeof(value));
		if (::getsockopt(_id, level, option, reinterpret_cast<char*>(&value), &length) != -1)
			return true;
		SetException(ex, " (level=",level,", option=",option,", length=",length,")");
		return false;
	}

	template<typename Type>
	bool setOption(Exception& ex, int level, int option, Type value) {
		if (_ex) {
			if (_ex.cast<Ex::Intern>())
				ex.set<Ex::Unsupported>("Option ", option, " of type ", typeOf<Type>(), " not supported by ", typeOf(self));
			else
				ex = _ex;
			return false;
		}
        NET_SOCKLEN length(sizeof(value));
		if (::setsockopt(_id, level, option, reinterpret_cast<const char*>(&value), length) != -1)
			return true;
		SetException(ex, " (level=",level,", option=",option,", length=",length,")");
		return false;
	}

	struct Sending : Packet, virtual Object {
		Sending(const Packet& packet, const SocketAddress& address, int flags) : Packet(std::move(packet)), address(address), flags(flags) {}

		const SocketAddress address;
		const int			flags;
	};

	Exception					_ex;
	mutable std::mutex			_mutexSending;
	std::deque<Sending>			_sendings;
	std::atomic<uint64_t>			_queueing;

	std::atomic<int64_t>			_recvTime;
	ByteRate					_recvByteRate;
	std::atomic<int64_t>			_sendTime;
	ByteRate					_sendByteRate;

//// Used by IOSocket /////////////////////
	Decoder*					_pDecoder;
	bool						_externDecoder;
	OnReceived					_onReceived;
	OnAccept					_onAccept;
	OnError						_onError;
	OnFlush						_onFlush;
	OnDisconnection				_onDisconnection;

	uint16_t						_threadReceive;
	std::atomic<uint32_t>			_receiving;
	std::atomic<uint8_t>			_reading;
	std::atomic<bool>			_sending;
	const Handler*				_pHandler; // to diminue size of Action+Handle

	bool						_opened;

#if !defined(_WIN32)
	Weak<Socket>*				_pWeakThis;
#endif

	friend struct IOSocket;
};


} // namespace Mona
