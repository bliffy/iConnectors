/*
 * Copyright (C) 2015, Michael E. Jolley
 *
 * This software is released under the terms of The MIT License.
*/

// sloppy testing for non-blocking tcp
// implementation of connectors

#include <iostream>
#include <string.h>
#include "tcpconnectors.h"

#include <time.h>

#define HOSTNAME "127.0.0.1"
#define HOSTPORT "4040"

void client();
void server();
void extra();

void mysleep(int sec){
#ifdef WINDOWS
 Sleep(sec*1000);
#else
 sleep(sec);
#endif
}

int main(int argc,char** argv)
{
  if(argc!=2)
    return -1;
  if(BFNET::connector_tcp_init())
    return -1;
  extra();
  if(strcmp(argv[1],"client")==0) client();
  else if(strcmp(argv[1],"server")==0) server();
  BFNET::connector_tcp_cleanup();

  return 0;
}

void client()
{
  BFNET::tcpUri_type  uri;
  BFNET::TCPConnection client;
  strcpy(uri.uri, HOSTNAME);
  strcpy(uri.port,HOSTPORT);

  std::cout<<"panding connect...\n";
  if(client.connect(&uri))
  {
    std::cerr<<"connect failed\n";
    return;
  }
  else
    std::cout<<"connected\n";

  //mysleep(8);

  std::cout<<"sending \"hello\"...\n";
  char msg[] = "hello";
  char *buf = msg;
  int len = 6;
  while(len>0){
    int r = client.send(buf,1);
    std::cerr<< r <<std::endl;
//mysleep(3);
    if(r>0){ buf+=r; len-=r; }
    if(r==-1){
      std::cerr<<"dead peer\n";
      break;
    }
  }
  std::cout<<"done sending\n";

  mysleep(3);

  //////////////////////////////////////////
  std::cerr<<"shoop!\n";
  char payload[] = "1234123412341234";
  buf = payload;
  long t = (long)time(NULL);
  unsigned long sent = 0;
  int r = 0;
  while((long)time(NULL) - t < 10 && r!=-1)
  {
    r = client.send(buf,16);
    if(r>=0) sent += r;
    else
    {
      std::cerr<< "peer closed! ("<<r<<")\n";
    }
  }
  sent /= 10;
  std::cerr<< "sent "<< sent <<" bytes/s\n";
  ////////////////////////////////////////
  std::cout<<"shutting down\n";
  client.terminate();
  std::cout<<"exiting\n";
}

void server()
{
  BFNET::tcpUri_type  uri;
  BFNET::TCPProvider server;
  strcpy(uri.uri,HOSTNAME);
  strcpy(uri.port,HOSTPORT);
  int state = server.bind(&uri);
  if(state)
  {
    std::cerr<<"binding error: "<<state<<"\n";
    return;
  }
  else
    std::cout<<"bound to port\n";

  std::cout<<"waiting for accept\n";
  BFNET::TCPConnection c;
  if(server.accept(&c)!=0)
  {
    std::cerr<<"bogus accept result\n";
    server.terminate();
    return;
  }
  else
  {
    std::cout<<"accepted connection\n";
//    mysleep(4);
    std::cout<<"let's recv something\n";

    int len=6;
    char buf[6];
    char *b = buf;
    while(len>0){
      int r = c.recv(b,1);
      if(r>0)
      {
std::cerr<<" - "<< r <<std::endl;
        len-=r;b+=r;
      }
      if(r==-1){
        std::cerr<<"broken peer\n";
        *b=0;
        break;
      }
    }
    std::cout<<"received "<<buf<<std::endl;
    mysleep(3);
    /////////////////////////////////////////////
    std::cerr<<"shoop!\n";
    long t = (long)time(NULL);
    unsigned long recvd = 0;
    char payload[16];
    int r = 0;
    while(((long)time(NULL) - t < 10 || r>=0) && r!=-1)
    {
      if( (r=c.recv(payload,16))<0 )
        std::cerr<<"peer closed! ("<<r<<")\n";
      else
        recvd += r;
    }
    recvd /= 10;
    std::cerr<<"received "<<recvd<<" bytes/s\n";
    /////////////////////////////////////////////
    std::cout<<"terminate the con\n";
    c.terminate();
  }
//  mysleep(3);
  std::cout<<"terminate server too\n";
  server.terminate();
  std::cout<<"exiting\n";
}

void extra()
{
  typedef BFNET::TCPConnection CON;
  typedef BFNET::TCPProvider   PROV;
  CON  con1;
  PROV prov1;
  CON  con2(con1);
  PROV prov2(prov1);
  CON  con3  = con1;
  PROV prov3 = prov1;
  con2  = con3;
  prov2 = prov3;
}

