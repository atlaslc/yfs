#ifndef lock_server_h
#define lock_server_h

#include <string>
#include "lock_protocol.h"

class lock_server {
 private:
  int nacquire;

 public:
  lock_server();
  lock_protocol::status stat(std::string, std::string, int &);
  void granter();
};

#endif
