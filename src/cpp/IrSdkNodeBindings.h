#pragma once

#include <napi.h>
#include "IRSDKWrapper.h"

namespace NodeIrSdk
{

  IRSDKWrapper irsdk;

  Napi::Value Start(const Napi::CallbackInfo &info);

  void Shutdown(const Napi::CallbackInfo &info);

  Napi::Value IsInitialized(const Napi::CallbackInfo &info);

  Napi::Value IsConnected(const Napi::CallbackInfo &info);

  Napi::Value UpdateSessionInfo(const Napi::CallbackInfo &info);

  Napi::Value GetSessionInfo(const Napi::CallbackInfo &info);

  Napi::Value UpdateTelemetry(const Napi::CallbackInfo &info);

  Napi::Value GetTelemetry(const Napi::CallbackInfo &info);

  Napi::Value GetTelemetryDescription(const Napi::CallbackInfo &info);

  Napi::Value SendCmd(const Napi::CallbackInfo &info);

  static void CleanUp(void *arg);

  Napi::Object Init(Napi::Env env, Napi::Object exports)
  {
    irsdk.Startup(Napi::CallbackInfo(env, nullptr, 0));

    node::AddEnvironmentCleanupHook(env, CleanUp, nullptr);

    exports.Set("start", Napi::Function::New(env, Start));
    exports.Set("shutdown", Napi::Function::New(env, Shutdown));
    exports.Set("isInitialized", Napi::Function::New(env, IsInitialized));
    exports.Set("isConnected", Napi::Function::New(env, IsConnected));
    exports.Set("updateSessionInfo", Napi::Function::New(env, UpdateSessionInfo));
    exports.Set("getSessionInfo", Napi::Function::New(env, GetSessionInfo));
    exports.Set("updateTelemetry", Napi::Function::New(env, UpdateTelemetry));
    exports.Set("getTelemetryDescription", Napi::Function::New(env, GetTelemetryDescription));
    exports.Set("getTelemetry", Napi::Function::New(env, GetTelemetry));
    exports.Set("sendCmd", Napi::Function::New(env, SendCmd));

    return exports;
  }

  NODE_API_MODULE(IrSdkNodeBindings, Init)
}