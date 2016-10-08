/*
 * Basic cross-platform nonblocking TCP socket objects
 * using the connections abstraction.
 *
 * Copyright (C) 2015, Michael E. Jolley
 *
 * This software is released under the terms of The MIT License.
*/

/*
 * related references
 * http://cs.baylor.edu/~donahoo/practical/CSockets/textcode.html
 * https://msdn.microsoft.com/en-us/library/windows/desktop/ms741416%28v=vs.85%29.aspx
*/

#ifndef __TCP_CONNECTORS_H__
#define __TCP_CONNECTORS_H__

#include "iconnectors.h"

#include <string.h>

#if (defined WIN32 || defined WIN64 || defined WINDOWS) 
#include <winsock2.h>
#include <ws2tcpip.h>
#ifndef WINDOWS
#define WINDOWS
#endif
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#endif

namespace BFNET
{
#ifdef WINDOWS
  typedef SOCKET socket_type;
#else
  typedef int socket_type;
#endif

  // uri type for the connection
  struct tcpUri_type {
    char uri[1024];
    char port[16];
  };

  // initialize and release system-specific socket framework
  int  connector_tcp_init(void);
  void connector_tcp_cleanup(void);

  // the following for use with clients and I/O interfaces
  class TCPConnection : public iConnection<tcpUri_type>
  {
    public:
      TCPConnection(void);
      TCPConnection(const TCPConnection& cr);
      ~TCPConnection(void);
      TCPConnection& operator= (const TCPConnection& cr);
      int connect(const tcpUri_type* host);
      int send(const char* bytes,int length);
      int recv(char* bytes,int length);
      void terminate(void);
      inline void set_socket(const socket_type& s){_socket=s;}
      inline socket_type get_socket(void) const {return _socket;}
    protected:
      socket_type _socket;
  };

  // the following for use with service points that yield
  // resultant connections
  class TCPProvider : public iProvider<tcpUri_type,TCPConnection>
  {
    public:
      TCPProvider(void);
      TCPProvider(TCPProvider& pr);
      ~TCPProvider(void);
      TCPProvider& operator= (const TCPProvider& pr);
      int bind(const tcpUri_type* host);
      void terminate(void);
      int accept(TCPConnection* connection);
      void set_max_pending(int m);
    protected:
      socket_type _socket;
      int _maxPending;
  };
}

#endif

