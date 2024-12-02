#include "IRSDKWrapper.h"
#include "IrSdkBindingHelpers.h"
#include "IrSdkNodeBindings.h"
#include "IrSdkCommand.h"

#include <iostream>
#include <stdint.h>

namespace NodeIrSdk
{

  Napi::Value Start(const Napi::CallbackInfo &info)
  {
    return Napi::Boolean::New(info.Env(), irsdk.Startup(info));
  }

  void Shutdown(const Napi::CallbackInfo &info)
  {
    irsdk.Shutdown(info);
  }

  Napi::Value IsInitialized(const Napi::CallbackInfo &info)
  {
    return irsdk.IsInitialized(info);
  }

  Napi::Value IsConnected(const Napi::CallbackInfo &info)
  {
    return irsdk.IsConnected(info);
  }

  Napi::Value UpdateSessionInfo(const Napi::CallbackInfo &info)
  {
    return irsdk.UpdateSessionInfo(info);
  }

  Napi::Value GetSessionInfo(const Napi::CallbackInfo &info)
  {
    return irsdk.GetSessionInfo(info);
  }

  Napi::Value UpdateTelemetry(const Napi::CallbackInfo &info)
  {
    return irsdk.UpdateTelemetry(info);
  }

  Napi::Value GetTelemetry(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();
    Napi::Object rootObj = Napi::Object::New(env);
    Napi::Object valuesObj = Napi::Object::New(env);

    rootObj.Set("timestamp", Napi::Date::New(env, irsdk.GetLastTelemetryUpdateTS(info).As<Napi::Number>().DoubleValue()));

    std::vector<irsdk_varHeader *> headers = irsdk.varHeadersArr;

    for (const auto item : headers)
    {
      IRSDKWrapper::TelemetryVar var(item);
      irsdk.GetVarVal(Napi::CallbackInfo(env, &var, 1));
      Napi::Value varValue = ConvertTelemetryVarToObject(env, var);
      valuesObj.Set(item->name, varValue);
    }

    rootObj.Set("values", valuesObj);
    return rootObj;
  }

  Napi::Value GetTelemetryDescription(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();
    Napi::Object obj = Napi::Object::New(env);
    std::vector<irsdk_varHeader *> headers = irsdk.varHeadersArr;

    for (const auto item : headers)
    {
      IRSDKWrapper::TelemetryVar var(item);
      irsdk.GetVarVal(Napi::CallbackInfo(env, &var, 1));
      Napi::Object varObj = Napi::Object::New(env);
      ConvertVarHeaderToObject(env, var, varObj);
      obj.Set(item->name, varObj);
    }

    return obj;
  }

  Napi::Value SendCmd(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();

    if (!irsdk.IsInitialized(info).As<Napi::Boolean>().Value() ||
        !irsdk.IsConnected(info).As<Napi::Boolean>().Value())
    {
      return env.Undefined();
    }

    if (info.Length() > 4 || info.Length() < 1)
    {
      Napi::TypeError::New(env, "sendCommand: invalid arguments (1 to 4 accepted)").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    for (size_t i = 0; i < info.Length(); ++i)
    {
      if (!info[i].IsNumber())
      {
        Napi::TypeError::New(env, "sendCommand: invalid argument type, number needed").ThrowAsJavaScriptException();
        return env.Undefined();
      }
    }

    switch (info.Length())
    {
    case 1:
      broadcastCmd(info[0].As<Napi::Number>().Int32Value(), 0, 0);
      break;
    case 2:
      broadcastCmd(info[0].As<Napi::Number>().Int32Value(),
                   info[1].As<Napi::Number>().Int32Value(), 0);
      break;
    case 3:
      broadcastCmd(info[0].As<Napi::Number>().Int32Value(),
                   info[1].As<Napi::Number>().Int32Value(),
                   info[2].As<Napi::Number>().Int32Value());
      break;
    case 4:
      broadcastCmd(info[0].As<Napi::Number>().Int32Value(),
                   info[1].As<Napi::Number>().Int32Value(),
                   info[2].As<Napi::Number>().Int32Value(),
                   info[3].As<Napi::Number>().Int32Value());
      break;
    }

    return env.Undefined();
  }

  void CleanUp(void *arg)
  {
    irsdk.Shutdown(Napi::CallbackInfo(nullptr, nullptr, 0));
  }

} // namespace NodeIrSdk