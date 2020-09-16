// lock protocol.

#ifndef lock_protocol_h
#define lock_protocol_h

#include <assert.h>
#include <pthread.h>
#include <list>
#include <set>
#include "rpc.h"

class lock_protocol {
 public:
  enum xxstatus { OK, RETRY, RPCERR, NOENT, IOERR };
  typedef int status;
  enum rpc_numbers { acquire = 0x7001, release, subscribe, grant, stat };
};

class lock_status {
 public:
  enum status { FREE, LOCKED, ACQUIRING, ABSENT };
};

#endif
