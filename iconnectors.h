/*
 * Interface for generic network connection abstraction.
 *
 * Copyright (C) 2015, Michael E. Jolley
 *
 * This software is released under the terms of The MIT License.
*/

#ifndef __I_CONNECTORS_H__
#define __I_CONNECTORS_H__

namespace BFNET
{
  // the following for use with clients and I/O interfaces
  template<typename uri_type=void>
  class iConnection
  {
    public:
      virtual int connect(const uri_type* host){return 0;}
      virtual int send(const char* bytes,int length){return 0;}
      virtual int recv(char* bytes,int length){return 0;}
      virtual void terminate(void){}
  };

  // the following for use with service points that produce
  // resultant connections
  template <typename uri_type=void,class connection_type=iConnection<> > 
  class iProvider
  {
    public:
      virtual int bind(const uri_type* host){return 0;}
      virtual int accept(connection_type* connection){return 0;}
      virtual void terminate(void){}
  };
}

#endif

