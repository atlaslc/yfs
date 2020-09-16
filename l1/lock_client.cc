// RPC stubs for clients to talk to lock_server
// see lock_client.h for protocol details.

#include "lock_client.h"
#include "rpc.h"
#include <arpa/inet.h>
#include <sstream>
#include <iostream>
#include <stdio.h>

lock_client::lock_client(std::string xdst)
{
  //create a rpc client and associate it with the lock_server's rpcs 
  sockaddr_in dstsock;
  make_sockaddr(xdst.c_str(), &dstsock);
  cl = new rpcc(dstsock);
  if (cl->bind() < 0) {
    printf("lock_client: call bind\n");
  }
  
  //get a random port number for the client's reverse rpc server
  struct timeval tv;
  gettimeofday(&tv, NULL);
  srandom(tv.tv_usec);
  int rlock_port = ((random()%32000) | (0x1 << 10));

  std::ostringstream tmp;
  tmp << rlock_port;
  id = tmp.str();

  rpcs *rlsrpc = new rpcs(htons(rlock_port));
  //lock_client handles the grant rpc request from the lock_server
  rlsrpc->reg(lock_protocol::grant, this, &lock_client::grant);
}

int
lock_client::stat(std::string name)
{
  int r;
  int ret = cl->call(lock_protocol::stat, id, name, r);
  assert (ret == lock_protocol::OK);
  return r;
}

lock_protocol::status
lock_client::acquire(std::string name)
{
  return lock_protocol::OK;
}

lock_protocol::status
lock_client::release(std::string name)
{
  return lock_protocol::OK;
}

lock_protocol::status 
lock_client::grant(std::string name, int &dummy)
{
  return lock_protocol::OK;
}
