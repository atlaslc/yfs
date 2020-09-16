// the caching lock server implementation

#include "lock_server.h"
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <memory>
#include "rpc.h"

static void *grantthread(void *x) {
  lock_server *sc = (lock_server *)x;
  sc->granter();
  return 0;
}

lock_server::lock_server() : nacquire(0), lock_map() {
  pthread_t th;
  assert(pthread_mutex_init(&lock_map_m, 0) == 0);
  assert(pthread_mutex_init(&workq_m, 0) == 0);
  assert(pthread_cond_init(&lock_map_c, 0) == 0);
  assert(pthread_cond_init(&workq_c, 0) == 0);
  assert(pthread_create(&th, NULL, &grantthread, (void *)this) == 0);
}

static void *communicator(void *x) {
  auto lock_sp = *(std::shared_ptr<lock> *)x;
  pthread_mutex_lock(&lock_sp->lock_m);

  if (lock_sp->wq_empty()) {
    pthread_mutex_unlock(&lock_sp->lock_m);
    return NULL;
  }
  // xclt 就是对应客户端中 rpcs *rlsrpc 的端口号
  std::string xclt = lock_sp->wq_popfront();
  //->s = lock_status::LOCKED;
  lock_sp->owner = xclt;
  // 创建一个 rpcc
  sockaddr_in dstsock;
  make_sockaddr(xclt.c_str(), &dstsock);
  rpcc cl(dstsock);
  if (cl.bind() < 0) {
    printf("lock_client: call bind\n");
  }
  // grant lock_name to clt

  int r;

  auto ret = cl.call(lock_protocol::grant, lock_sp->name, r);
  if (ret == lock_protocol::OK && r == 1) {
    std::cout << "[comunicator thread] lock " << lock_sp->name
              << " has been granted to " << xclt << std::endl;
  }
  pthread_mutex_unlock(&lock_sp->lock_m);
  return NULL;
}
void lock_server::granter() {
  // This method should be a continuous loop, waiting for locks to become free
  // and then sending grant rpcs to those who are waiting for it.
  // 无限循环，处理 free lock 队列 workq
  // 为每一个 free lock 创建线程 communicator
  pthread_mutex_lock(&workq_m);
  while (1) {
    std::cout << "granter" << std::endl;
    while (workq.size() == 0) {
      pthread_cond_wait(&workq_c, &workq_m);
    }
    for (auto lock_item : workq) {
      if (lock_item->s == lock_status::LOCKED) {
        std::cout << "[lock_server::granter] lock " << lock_item->name
                  << " is LOCKED on server" << std::endl;
        pthread_t th;
        pthread_create(&th, NULL, communicator, (void *)&lock_item);
        pthread_detach(th);
      }
    }
    workq.clear();
  }
  pthread_mutex_unlock(&workq_m);
}

lock_protocol::status lock_server::stat(std::string clt, std::string name,
                                        int &r) {
  lock_protocol::status ret = lock_protocol::OK;
  printf("stat request from clt %s for lock %s\n", clt.c_str(), name.c_str());
  r = nacquire;
  return ret;
}

lock_protocol::status lock_server::acquire(std::string port,
                                           std::string lock_name, int &r) {
  std::cout << "[lock_server::acquire] acquire lock " << lock_name
            << " from clt " << port << std::endl;
  pthread_mutex_lock(&lock_map_m);
  auto pos_lock = lock_map.find(lock_name);

  // new lock_name
  if (pos_lock == lock_map.cend()) {
    std::cout << "[lock_server::acquire] new lock_name :" << lock_name
              << std::endl;
    // 创建新 FREE 锁 lock(lock_name)
    auto lock_sp = std::make_shared<lock>(lock_name);
    lock_sp->s = lock_status::LOCKED;
    lock_map.insert({lock_name, lock_sp});
    lock_sp->waitq.push_back(port);
    pthread_mutex_unlock(&lock_map_m);

    pthread_mutex_lock(&workq_m);
    workq.push_back(lock_sp);
    pthread_cond_signal(&workq_c);
    pthread_mutex_unlock(&workq_m);

    return lock_protocol::OK;
  }

  // old lock_name
  auto lock_sp = (*pos_lock).second;
  bool dup = false;
  for (auto clt : lock_sp->waitq) {
    if (clt == port) dup = true;
  }
  if (!dup) {
    pthread_mutex_lock(&lock_sp->lock_m);
    lock_sp->waitq.push_back(port);
    pthread_mutex_unlock(&lock_sp->lock_m);
  }
  if (lock_sp->s == lock_status::FREE) {
    lock_sp->s = lock_status::LOCKED;
    pthread_mutex_unlock(&lock_map_m);
    pthread_mutex_lock(&workq_m);
    workq.push_back(lock_sp);
    pthread_cond_signal(&workq_c);
    pthread_mutex_unlock(&workq_m);
    return lock_protocol::OK;
  }
  pthread_mutex_unlock(&lock_map_m);
  return lock_protocol::OK;
}

lock_protocol::status lock_server::release(std::string clt,
                                           std::string lock_name, int &r) {
  std::cout << "[lock_server::release] release request from clt " << clt << ' '
            << "for lock " << lock_name << std::endl;
  pthread_mutex_lock(&lock_map_m);
  auto pos_lock = lock_map.find(lock_name);
  if (pos_lock == lock_map.end()) {
    return lock_protocol::OK;
  }
  // pthread_mutex_unlock(&lock_map_m);
  auto lock_item = (*pos_lock).second;
  if (lock_item->s == lock_status::LOCKED) {
    if (lock_item->owner == clt) {
      std::cout << "[lock_server::release] clt " << clt << " release lock "
                << lock_name << " safely" << std::endl;
      if (lock_item->waitq.size() != 0) {
        pthread_mutex_unlock(&lock_map_m);
        pthread_mutex_lock(&workq_m);
        workq.push_back(lock_item);
        pthread_cond_signal(&workq_c);
        pthread_mutex_unlock(&workq_m);
        return lock_protocol::OK;
      }
      lock_item->s = lock_status::FREE;
      pthread_mutex_unlock(&lock_map_m);
      return lock_protocol::OK;
    } else {
      // clt is releasing a locked lock not owened by it.
      std::cout << "[lock_server::release] clt " << clt
                << " can't release lock " << lock_name << " , owned by "
                << lock_item->owner << std::endl;
      pthread_mutex_unlock(&lock_map_m);
      return lock_protocol::OK;
    }
  }  // end locked

  // lock is FREE
  if (lock_item->waitq.size() == 0) {
    std::cout << "[lock_server::release] lock " << lock_item->name
              << " is FREE and its waitq is empty. " << std::endl;
    pthread_mutex_unlock(&lock_map_m);
  } else {
    std::cout << "[lock_server::release] lock " << lock_item->name
              << " is FREE and its waitq is not empty. " << std::endl;
    lock_item->s = lock_status::LOCKED;
    pthread_mutex_unlock(&lock_map_m);
    pthread_mutex_lock(&workq_m);
    workq.push_back(lock_item);
    pthread_mutex_unlock(&workq_m);
    pthread_cond_signal(&workq_c);
  }

  return lock_protocol::OK;
}
