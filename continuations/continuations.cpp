/*
 * Functional programming:
 * - https://www.fpcomplete.com/blog/2012/06/asynchronous-api-in-c-and-the-continuation-monad
 * - https://www.fpcomplete.com/blog/2012/07/the-functor-pattern-in-c
 * - https://www.fpcomplete.com/blog/2012/09/functional-patterns-in-c
 * Example and Text mostly by Bartosz Milewski
 */

#include <iostream>
#include <functional>
#include <thread>
#include <chrono>

/**
 * Fake async API, simulates an asynchronous process.
 * Starts a thread sleeping for 3secs then simulates an event calling handler
 */
void asyncApi(std::function<void(std::string)> handler) {
    std::cout << "called async in thread: " << std::this_thread::get_id() << "\n";
    std::thread th([handler]() {
        std::cout << "Started async, sleeping 3secs\n";
        std::this_thread::sleep_for(std::chrono::seconds(3));
        handler("Data from async");
    });
    th.detach();
}

/**
 * Continuator
 * Remember, continuators are values that haven't materialized yet.
 * A calculation may return such a "special" value in the form of a continuator
 * (for instance a file handle from an asynchronous open call)
 */
template<class R, class A>
struct Continuator {
  virtual ~Continuator() {}
  virtual R andThen(std::function<R(A)> k) = 0;
};

/**
 * Continuator Object, continuation wrapper for the asyncApi
 * The way to understand a continuator in the context of async API is to see it
 * as the encapsulation of a value that's not there yet.
 * What can we do with a value that doesn't exist?
 * Well, whatever it is, we can do it inside the continuation.
 */
struct AsyncApi : Continuator<void, std::string> {
  virtual ~AsyncApi() {}
  void andThen(std::function<void(std::string)> k) {
      asyncApi(k);
  }
};


/**
 * "continuation monad" Monadic bind
 */
template<class R, class A, class C>
struct Bind : Continuator<R, A> {
  Bind(const C& ktor, std::function<std::unique_ptr<Continuator<R, A>>(A)> rest)
      : _ktor(ktor), _rest(rest){
  }
  virtual ~Bind() {}
  R andThen(std::function<R(A)> k) {
      std::function<std::unique_ptr<Continuator<R, A>>(A)> rest = _rest;
      std::function<R(A)> lambda = [k, rest](A a) {
          return rest(a)->andThen(k);
      };
      _ktor.andThen(lambda);
  }
  C _ktor;
  std::function<std::unique_ptr<Continuator<R, A>>(A)> _rest;
};

/**
 * Continuation containing of multiple sub-continuations
 */
struct Loop : Continuator<void, std::string> {
    Loop(std::string s) : _s(s) {}
    virtual ~Loop() {}
    void andThen(std::function<void(std::string)> k) {
        std::cout << "Loop::andThen: " <<_s << std::endl;
        Bind<void, std::string, AsyncApi>(AsyncApi(), [](std::string k) {
          return std::unique_ptr<Continuator>(new Loop(k));
        }).andThen(k);
    }
    std::string _s;
};

/**
 * Break the loop and return
 */
template<class R, class A>
struct Return : Continuator<R, A> {
  Return(A x) : _x(x) {}
  R andThen(std::function<R(A)> k) {
      return k(_x);
  }
  A _x;
};

struct LoopN : Continuator<void, std::string> {
    LoopN(std::string s, int n) : _s(s), _n(n) {}
    void andThen(std::function<void(std::string)> k) {
        std::cout << "[LoopN::andThen] " <<_s << " " << _n << std::endl;
        int n = _n;
        Bind<void, std::string, AsyncApi>(AsyncApi(),
        [n](std::string s) -> std::unique_ptr<Continuator<void, std::string>> {
            if (n > 0) {
                return std::unique_ptr<Continuator<void, std::string>>(
                    new LoopN(s, n - 1));
            } else {
                return std::unique_ptr<Continuator<void, std::string>> (
                    new Return<void, std::string>("Done!"));
            }
        }).andThen(k);
    }
    std::string _s;
    int    _n;
};

// Alternative without Lambdas ...
struct myAsyncHandler {
  void operator()(std::string s) {
    std::cout << "called me back with: \"" << s << "\" in thread: "
              << std::this_thread::get_id()
              << std::endl;
  }
};

int main() {
  std::cout << "simple call... done in 5 seconds" << std::endl;
  // Here's how you could use AsyncApi
  // with the continuation in the form of a lambda function:
  AsyncApi callApi;
  // call the async api, and then when it calls back execute the handler
  // this basically means: call the api, then continue with my handler
  callApi.andThen(myAsyncHandler());
  // call the async api, and then when it calls back execute the lambda
  // callApi.andThen([](std::string s) {
  //     std::cout << "called me back with: \"" << s << "\" in thread: "
  //               << std::this_thread::get_id()
  //               << std::endl;
  // });

  // run counter in parallel
  for(int i = 0; i < 5; ++i) {
      std::cout << i << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  // Real fun begins when you want to compose asynchronous APIs.
  // For instance, you might want to open a file and, if it succeeds, read its
  // contents, possibly in multiple chunks -- all using async APIs.
  //
  // You could express this logic with typical imperative control structures,
  // but only if you can afford to block on each API call.
  // Otherwise you have to deal with inversion of control.

  // endless loop
  // Loop("Loop: ").andThen([](std::string s) {
  //     std::cout << "Never happens: " << s << std::endl;
  // });

  // LoopN("Loop: ", 4).andThen([](std::string s) {
  //     std::cout << "My Final Hanlder: " << s << std::endl;
  // });

  LoopN("Loop: ", 4).andThen(myAsyncHandler());

  // run counter in parallel
  for(int i = 0; i < 20; ++i) {
      std::cout << i << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
