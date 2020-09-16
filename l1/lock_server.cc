// the caching lock server implementation

#include "lock_server.h"
#include "rpc.h"
#include <stdio.h>
#include <unistd.h>

static void *
grantthread(void *x)
{
  lock_server *sc = (lock_server *) x;
  sc->granter();
  return 0;
}

lock_server::lock_server(): nacquire(0)
{
  pthread_t th;
  assert (pthread_create(&th, NULL, &grantthread, (void *) this)==0);
}

void
lock_server::granter()
{

  // This method should be a continuous loop, waiting for locks to become free
  // and then sending grant rpcs to those who are waiting for it.

}

lock_protocol::status
lock_server::stat(std::string clt, std::string name, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  printf("stat request from clt %s for lock %s\n", clt.c_str(), name.c_str());
  r = nacquire;
  return ret;
}

