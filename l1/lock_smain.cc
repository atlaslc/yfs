// unmarshall RPCs from lock_smain and hand them to lock_server

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lock_server.h"
#include "rpc.h"

int main(int argc, char *argv[]) {
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

  srandom(getpid());

  if (argc != 2) {
    fprintf(stderr, "Usage: %s port\n", argv[0]);
    exit(1);
  }

  lock_server ls;
  rpcs server(htons(atoi(argv[1])));
  // 在 rpc server 中注册服务 lock_server::stat
  server.reg(lock_protocol::stat, &ls, &lock_server::stat);
  server.reg(lock_protocol::acquire, &ls, &lock_server::acquire);
  server.reg(lock_protocol::release, &ls, &lock_server::release);
  while (1) sleep(1000);
}
