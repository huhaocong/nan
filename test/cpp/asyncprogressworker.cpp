/*********************************************************************
 * NAN - Native Abstractions for Node.js
 *
 * Copyright (c) 2018 NAN contributors
 *
 * MIT License <https://github.com/nodejs/nan/blob/master/LICENSE.md>
 ********************************************************************/

#ifndef _WIN32
#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#endif
#include <nan.h>

using namespace Nan;  // NOLINT(build/namespaces)

class ProgressWorker : public AsyncProgressWorker {
 public:
  ProgressWorker(
      Callback *callback
    , Callback *progress
    , int milliseconds
    , int iters)
    : AsyncProgressWorker(callback), progress(progress)
    , milliseconds(milliseconds), iters(iters) {}

  ~ProgressWorker() {
    delete progress;
  }

  void Execute (const AsyncProgressWorker::ExecutionProgress& progress) {
    for (int i = 0; i < iters; ++i) {
      progress.Send(reinterpret_cast<const char*>(&i), sizeof(int));
      Sleep(milliseconds);
    }
  }

  void HandleProgressCallback(const char *data, size_t count) {
    HandleScope scope;

    v8::Local<v8::Value> argv[] = {
        New<v8::Integer>(*reinterpret_cast<int*>(const_cast<char*>(data)))
    };
    progress->Call(1, argv, async_resource);
  }

 private:
  Callback *progress;
  int milliseconds;
  int iters;
};

NAN_METHOD(DoProgress) {
  Callback *progress = new Callback(To<v8::Function>(info[2]).ToLocalChecked());
  Callback *callback = new Callback(To<v8::Function>(info[3]).ToLocalChecked());
  AsyncQueueWorker(new ProgressWorker(
      callback
    , progress
    , To<uint32_t>(info[0]).FromJust()
    , To<uint32_t>(info[1]).FromJust()));
}

NAN_MODULE_INIT(Init) {
  Set(target
    , New<v8::String>("a").ToLocalChecked()
    , New<v8::FunctionTemplate>(DoProgress)->GetFunction());
}

NODE_MODULE(asyncprogressworker, Init)
