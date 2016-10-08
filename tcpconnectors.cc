/*
 * Copyright (C) 2015, Michael E. Jolley
 *
 * This software is released under the terms of The MIT License.
*/

#include "tcpconnectors.h"

#ifdef WINDOWS
#define SOCKET_DEFAULT INVALID_SOCKET
#else
#define SOCKET_DEFAULT -1
#endif

#ifdef WINDOWS
#define _TCP_SOCKET_INVALID(s) (s==INVALID_SOCKET)
#else
#define _TCP_SOCKET_INVALID(s) (s<0)
#endif

namespace BFNET
{
  int connector_tcp_init(void)
  {
#ifdef WINDOWS
    static WSADATA wsaData;
    if(0!=WSAStartup(MAKEWORD(2,2),&wsaData))
      return -1;
    else
      return 0;
#else
    return 0;
#endif
  }

  void connector_tcp_cleanup(void)
  {
#ifdef WINDOWS
    WSACleanup();
#endif
  }

  TCPConnection::TCPConnection(void)
  {
    _socket = SOCKET_DEFAULT;
  }

  TCPConnection::TCPConnection(const TCPConnection& cr)
  {
    _socket = cr._socket;
  }

  TCPConnection::~TCPConnection(void)
  {
    // TODO: potentially add termination via reference counter
  }

  TCPConnection& TCPConnection::operator= (const TCPConnection& cr)
  {
    _socket = cr._socket;
    return *this;
  }

  int TCPConnection::connect(const tcpUri_type* host)
  {
    addrinfo  hints,
              *addrResult = NULL;
    memset(&hints,0,sizeof(addrinfo));
    hints.ai_family   = AF_INET;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;

    if(getaddrinfo(host->uri,host->port,&hints,&addrResult))
      return -1;

    for(addrinfo* ai=addrResult;ai!=NULL;ai=ai->ai_next)
    {   
      if((ai->ai_family!=PF_INET)&&(ai->ai_family!=PF_INET6))
        continue;

      _socket = socket(ai->ai_family,ai->ai_socktype,ai->ai_protocol);
      if( _TCP_SOCKET_INVALID(_socket) )
        continue;
#ifdef WINDOWS
      if(::connect(_socket,ai->ai_addr,ai->ai_addrlen)==SOCKET_ERROR)
        closesocket(_socket);
      else
      {
        // moved this beyond accept(), because async accept() is awful
        unsigned long iMode = 1;
        ioctlsocket(_socket,FIONBIO,&iMode);
#else
      //fcntl(_socket,F_SETFL,O_NONBLOCK);
      //^this is supposed to be a similar mechanism for nonblocking,
      // but it behaves very strangely. We'll use nonblock on
      // send/recv instead.
      if(::connect(_socket,ai->ai_addr,ai->ai_addrlen)<0)
        close(_socket);
      else
      {
#endif
        freeaddrinfo(addrResult);
        return 0;
      }
    }
    freeaddrinfo(addrResult);
    return -1;
  }

  int TCPConnection::send(const char* bytes,int length)
  {
    if( _TCP_SOCKET_INVALID(_socket) ) return -1;
#ifdef WINDOWS
    register int r = ::send(_socket,bytes,length,0);
    if(r==SOCKET_ERROR) return -1;
    //else if( r>=0 ) return r;
    //else return r;
    return r;
#else
    register int r = ::send(_socket,bytes,length,MSG_DONTWAIT);
    if( r>=0 ) return r;
    else if( errno==EAGAIN || errno==EWOULDBLOCK ) return 0;
    else return -1;
#endif
  }

  int TCPConnection::recv(char* bytes,int length)
  {
    if( _TCP_SOCKET_INVALID(_socket) ) return -1;
#ifdef WINDOWS
    register int r = ::recv(_socket,bytes,length,0);
    if( r==SOCKET_ERROR ) return -1;
    else if( r>=0 ) return r;
    else return -1;
#else
    register int r = ::recv(_socket,bytes,length,MSG_DONTWAIT);
    if( r>0 )
      return r;
    else if( r<0 && (errno==EAGAIN || errno==EWOULDBLOCK) ) return 0;
    else return -1;
#endif
  }

  void TCPConnection::terminate(void)
  {
    if( _TCP_SOCKET_INVALID(_socket) ) return;
#ifdef WINDOWS
    shutdown(_socket,SD_BOTH);
    closesocket(_socket);
#else
    close(_socket);
#endif
    _socket = SOCKET_DEFAULT;
  }
  

  TCPProvider::TCPProvider(void)
  {
    _socket = SOCKET_DEFAULT;
    _maxPending = SOMAXCONN;
  }

  TCPProvider::TCPProvider(TCPProvider& pr)
  {
    _socket = pr._socket;
    _maxPending = pr._maxPending;
  }

  TCPProvider::~TCPProvider(void)
  {
    // TODO: potentially terminate via reference counter
  }

  void TCPProvider::terminate(void)
  {
    if( _TCP_SOCKET_INVALID(_socket) ) return;
#ifdef WINDOWS
    shutdown(_socket,SD_BOTH);
    closesocket(_socket);
#else
    close(_socket);
#endif
    _socket = SOCKET_DEFAULT;
  }

  TCPProvider& TCPProvider::operator= (const TCPProvider& pr)
  {
    _socket = pr._socket;
    _maxPending = pr._maxPending;
    return *this;
  }

  void TCPProvider::set_max_pending(int m)
  {
    _maxPending = m; 
  }

  int TCPProvider::bind(const tcpUri_type* host)
  {
    addrinfo  hints,
              *addrResult = NULL;
    memset(&hints,0,sizeof(addrinfo));
    hints.ai_family   = AF_INET;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    if(getaddrinfo(host->uri,host->port,&hints,&addrResult))
      return -1;
    
    addrinfo* ai;
    for(ai=addrResult;ai!=NULL;ai=ai->ai_next)
    {
      if((ai->ai_family!=PF_INET)&&(ai->ai_family!=PF_INET6))
        continue;
      _socket = socket(ai->ai_family,ai->ai_socktype,ai->ai_protocol);
      if( _TCP_SOCKET_INVALID(_socket) )
        continue;
      if(::bind(_socket,ai->ai_addr,ai->ai_addrlen))
#ifdef WINDOWS
        closesocket(_socket);
#else
        close(_socket);
#endif
      else
        break;
    }

    if(ai==NULL)
    {
      freeaddrinfo(addrResult);
      return -3;
    }
    freeaddrinfo(addrResult);

    if( listen(_socket,_maxPending) )
      return -4;
    return 0;
  }

  int TCPProvider::accept(TCPConnection* connection)
  {
    socket_type s = ::accept(_socket,NULL,NULL);
    connection->set_socket(s);
    if( _TCP_SOCKET_INVALID(s) ) return -1;
    else return 0;
  }
}

#undef SOCKET_DEFAULT
#undef _TCP_SOCKET_INVALID

