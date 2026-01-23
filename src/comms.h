#ifndef _GIT_EACH_COMMS_H
#define _GIT_EACH_COMMS_H

/*
 * This implementation is not recommended / performant.
 * one goal of this project is zero deps
 */

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

template <typename T> class FanOut {
  std::queue<T> buf;
  std::mutex mtx;
  std::condition_variable cv;
  bool finished;

public:
  class Recv {
    FanOut<T> *out;
    Recv(FanOut<T> *out) : out{out} {}
    friend FanOut<T>::Recv FanOut<T>::recv();

  public:
    std::optional<T> recv() {
      std::unique_lock<std::mutex> lck(out->mtx);
      while (out->buf.empty() && !out->finished)
        out->cv.wait(lck);

      if (!out->buf.empty()) {
        T t = out->buf.front();
        out->buf.pop();
        return t;
      }

      return std::nullopt;
    }
  };

  FanOut() : buf{}, mtx{}, cv{}, finished{false} {}

  FanOut(const FanOut &) = delete;
  FanOut &operator=(const FanOut &) = delete;

  void push(T t) {
    std::unique_lock<std::mutex> lck(mtx);
    buf.push(t);
    cv.notify_one();
  }

  void close() {
    std::unique_lock<std::mutex> lck(mtx);
    finished = true;
    cv.notify_all();
  }

  Recv recv() { return Recv(this); }
};

template <typename T> class FanIn {
  std::queue<T> buf;
  std::mutex mtx;
  std::condition_variable cv;
  uint64_t count;

public:
  class Sender {
    FanIn<T> *in;
    Sender(FanIn<T> *in) : in{in} {}
    friend FanIn<T>::Sender FanIn<T>::sender();

  public:
    void push(T t) {
      std::unique_lock<std::mutex> lck(in->mtx);
      in->buf.push(t);
      in->cv.notify_one();
    }

    void close() {
      std::unique_lock<std::mutex> lck(in->mtx);
      in->count--;
      in->cv.notify_all();
    }
  };

  FanIn() : buf{}, mtx{}, cv{}, count{0} {}
  Sender sender() {
    std::unique_lock<std::mutex> lck(mtx);
    count++;

    return Sender(this);
  }

  std::optional<T> recv() {
    std::unique_lock<std::mutex> lck(mtx);

    while (buf.empty() && count != 0)
      cv.wait(lck);

    if (!buf.empty()) {
      T t = buf.front();
      buf.pop();
      return t;
    }

    return std::nullopt;
  }
};

#endif
