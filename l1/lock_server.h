#ifndef lock_server_h
#define lock_server_h

#include <deque>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <vector>
#include "lock_protocol.h"
#include "rpc.h"

class lock;

class lock_server {
 private:
  int nacquire;

  std::map<std::string, std::shared_ptr<lock>> lock_map;
  pthread_mutex_t lock_map_m;
  pthread_cond_t lock_map_c;

 public:
  lock_server();
  struct temp {
    temp(std::shared_ptr<lock> _lock_item, const std::string& _clt,
         lock_server* _ls)
        : lock_item(_lock_item), clt(_clt), ls(_ls){};
    std::shared_ptr<lock> lock_item;
    std::string clt;
    lock_server* ls;
  };
  std::deque<std::shared_ptr<lock>> workq;
  pthread_mutex_t workq_m;
  pthread_cond_t workq_c;

  lock_protocol::status stat(std::string clt, std::string name, int& r);
  lock_protocol::status acquire(std::string clt, std::string lock_name, int& r);
  lock_protocol::status release(std::string clt, std::string lock_name, int& r);
  // lock_protocol::status grant(std::string clt, std::string lock_name, int&
  // r);
  void granter();
};

class lock {
 public:
  lock() : s(lock_status::FREE), name(), waitq() {
    assert(pthread_mutex_init(&waitq_m, 0) == 0);
    assert(pthread_cond_init(&waitq_c, 0) == 0);
    assert(pthread_cond_init(&lock_c, 0) == 0);
    assert(pthread_mutex_init(&lock_m, 0) == 0);
  }
  lock(const std::string _name) : s(lock_status::FREE), name(_name), waitq() {
    std::cout << "Construct Lock " << _name << std::endl;
    assert(pthread_mutex_init(&waitq_m, 0) == 0);
    assert(pthread_cond_init(&waitq_c, 0) == 0);
    assert(pthread_cond_init(&lock_c, 0) == 0);
    assert(pthread_mutex_init(&lock_m, 0) == 0);
  }
  lock(const lock&) = delete;
  ~lock() { std::cout << "[~lock] Delete lock " << name << std::endl; }

  bool wq_empty() { return waitq.size() == 0; }
  std::string wq_popfront() {
    std::string res = waitq.front();
    waitq.pop_front();
    return res;
  }

  typedef int status;
  typedef std::string lock_name;
  status s;
  std::string name;
  std::string owner;

  pthread_mutex_t lock_m;
  pthread_cond_t lock_c;

  // private:
  pthread_mutex_t waitq_m;
  pthread_cond_t waitq_c;
  std::deque<std::string> waitq;

  void _enterq() { pthread_mutex_lock(&waitq_m); }
  void _enq(std::string client_id) { waitq.push_back(client_id); }
  void _exitq() { pthread_mutex_unlock(&waitq_m); }
  void _wait() { pthread_cond_wait(&waitq_c, &waitq_m); }
  void _signal() { pthread_cond_signal(&waitq_c); }
};

#endif