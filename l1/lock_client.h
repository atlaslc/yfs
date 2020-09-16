// lock client interface.

#ifndef lock_client_h
#define lock_client_h

#include <map>
#include <memory>
#include <string>
#include "lock_client.h"
#include "lock_protocol.h"
#include "rpc.h"

//
// To work correctly for later labs, you MUST ensure that:
// 1. all rpc request handlers on the lock_server must run till
// completion without waiting for other remote clients.
// 2. upon acquire, lock_clients does not return until the lock is obtained from
// the server.
//
// Here is a suggested LOCK IMPLEMENTATION PLAN:
//
// On the client a lock can be in several states:
//	- absent: client does not own the lock
//  - acquiring: the client is acquiring ownership
//  - free: client owns the lock and no thread has it
//  - locked: client owns the lock and a thread has it
//
// At the lock_client side:
// The basic acquire() logic is like this:
//    check the state of the lock in a loop,
//    if free,
// 	    break out of loop, change the lock state to locked and proceed (i.e.
// return from acquire).
//    else if absent,
//      send acquire rpc to lock_server,
//      change lock state to acquiring,
//      waits for the conditional variable associated with the lock
//    else
//       waits for the conditional variable associated with the lock
//
// The basic release() logic is like this:
//    send release rpc to lock_server and changes the lock_state to absent.
//
// The rpc handler for grant() is like this:
//    change lock state to free and waits up some thread on this lock (if there
//    is one) by
//       signaling the corresponding conditional variable.
//
// On the server a lock can be in two states:
// - free: no clients own the lock
// - locked: 1 client owns the lock
//
// At the lock_server side:
// The rpc handler for acquire() is like this:
//   puts the client id on the waitqueue of this lock,
//   if lock state is free,
//      put lock name in granter's workqueue, signals granter thread
//
// The rpc handler for release() is like this:
//   change the lock state to free
//   if waitqueue of clients for this lock is non-zero,
//      put lock name in granter's workqueue, signals granter thread to grant
//      locks
//
// The granter thread:
//   for eachlock in its workqueue,
//   if lock state is free,
//      sends grant rpc requests to the first client on the lock's waitqueue.
//      change state to locked
class lock;

class lock_client {
 private:
  int rlock_port;
  std::string port;

  rpcc *cl;
  rpcs *rlsrpc;

  pthread_mutex_t lock_map_m;
  pthread_cond_t lock_map_c;

  std::map<std::string, std::shared_ptr<lock>> lock_map;

 public:
  lock_client(std::string xdst);
  ~lock_client(){};

  lock_protocol::status acquire(std::string);
  lock_protocol::status release(std::string);
  lock_protocol::status stat(std::string);

  // RPC handlers for rpc requests from the server
  lock_protocol::status grant(std::string, int &);
};

class lock {
 public:
  lock(std::string _name) : name(_name), s(lock_status::ABSENT) {
    pthread_mutex_init(&lock_m, NULL);
    pthread_cond_init(&lock_c, NULL);
  }
  typedef int status;
  typedef std::string lock_name;
  pthread_mutex_t lock_m;
  pthread_cond_t lock_c;
  status s;
  pthread_t own_th;
  std::string name;
};

#endif
