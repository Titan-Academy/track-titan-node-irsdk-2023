#include "IRSDKWrapper.h"
#include <iostream>

// npm install --debug enables debug prints
#ifdef _DEBUG
#define debug(x) std::cout << x << std::endl;
#else
#define debug(x)
#endif

Napi::Object NodeIrSdk::IRSDKWrapper::Init(Napi::Env env, Napi::Object exports)
{
  Napi::Function func = DefineClass(env, "IRSDKWrapper", {InstanceMethod("startup", &IRSDKWrapper::Startup), InstanceMethod("shutdown", &IRSDKWrapper::Shutdown), InstanceMethod("isInitialized", &IRSDKWrapper::IsInitialized), InstanceMethod("isConnected", &IRSDKWrapper::IsConnected), InstanceMethod("updateTelemetry", &IRSDKWrapper::UpdateTelemetry), InstanceMethod("updateSessionInfo", &IRSDKWrapper::UpdateSessionInfo), InstanceMethod("getSessionInfo", &IRSDKWrapper::GetSessionInfo), InstanceMethod("getVarHeaders", &IRSDKWrapper::GetVarHeaders), InstanceMethod("getVarVal", &IRSDKWrapper::GetVarVal), InstanceMethod("getLastTelemetryUpdateTS", &IRSDKWrapper::GetLastTelemetryUpdateTS)});

  Napi::FunctionReference *constructor = new Napi::FunctionReference();
  *constructor = Napi::Persistent(func);
  env.SetInstanceData(constructor);

  exports.Set("IRSDKWrapper", func);
  return exports;
}

NodeIrSdk::IRSDKWrapper::IRSDKWrapper(const Napi::CallbackInfo &info) : Napi::ObjectWrap<IRSDKWrapper>(info),
                                                                        hMemMapFile(NULL), pSharedMem(NULL), pHeader(NULL), lastTickCount(INT_MIN), lastSessionInfoUpdate(INT_MIN),
                                                                        data(NULL), dataLen(-1), sessionInfoStr()
{
  debug("IRSDKWrapper: constructing...");
}

NodeIrSdk::IRSDKWrapper::~IRSDKWrapper()
{
  debug("IRSDKWrapper: deconstructing...");
}

Napi::Value NodeIrSdk::IRSDKWrapper::Startup(const Napi::CallbackInfo &info)
{
  debug("IRSDKWrapper: starting up...");

  if (!hMemMapFile)
  {
    debug("IRSDKWrapper: opening mem map...");
    hMemMapFile = OpenFileMapping(FILE_MAP_READ, FALSE, IRSDK_MEMMAPFILENAME);
    if (hMemMapFile == NULL)
    {
      return Napi::Boolean::New(info.Env(), false);
    }
    pSharedMem = (const char *)MapViewOfFile(hMemMapFile, FILE_MAP_READ, 0, 0, 0);
    pHeader = (irsdk_header *)pSharedMem;
    lastTickCount = INT_MIN;
  }
  debug("IRSDKWrapper: start up ready.");
  return Napi::Boolean::New(info.Env(), true);
}

Napi::Value NodeIrSdk::IRSDKWrapper::IsInitialized(const Napi::CallbackInfo &info)
{
  if (!hMemMapFile)
  {
    debug("IRSDKWrapper: not initialized...");
    return Napi::Boolean::New(info.Env(), false);
  }
  debug("IRSDKWrapper: is initialized...");
  return Napi::Boolean::New(info.Env(), true);
}

Napi::Value NodeIrSdk::IRSDKWrapper::IsConnected(const Napi::CallbackInfo &info)
{
  bool status = pHeader->status == irsdk_stConnected;
  debug("IRSDKWrapper: sim status: " << status);
  return Napi::Boolean::New(info.Env(), status);
}

void NodeIrSdk::IRSDKWrapper::Shutdown(const Napi::CallbackInfo &info)
{
  debug("IRSDKWrapper: shutting down...");
  if (pSharedMem)
    UnmapViewOfFile(pSharedMem);

  if (hMemMapFile)
    CloseHandle(hMemMapFile);

  hMemMapFile = NULL;
  pSharedMem = NULL;
  pHeader = NULL;

  lastTickCount = INT_MIN;
  lastSessionInfoUpdate = INT_MIN;
  delete[] data;
  data = NULL;
  lastValidTime = time(NULL);
  varHeadersArr.clear();
  sessionInfoStr = "";

  debug("IRSDKWrapper: shutdown ready.");
}

Napi::Value NodeIrSdk::IRSDKWrapper::UpdateSessionInfo(const Napi::CallbackInfo &info)
{
  debug("IRSDKWrapper: updating session info...");
  if (startup())
  {
    int counter = pHeader->sessionInfoUpdate;

    if (counter > lastSessionInfoUpdate)
    {
      sessionInfoStr = getSessionInfoStr();
      lastSessionInfoUpdate = counter;
      return Napi::Boolean::New(info.Env(), true);
    }
    return Napi::Boolean::New(info.Env(), false);
  }
  return Napi::Boolean::New(info.Env(), false);
}

Napi::Value NodeIrSdk::IRSDKWrapper::GetSessionInfo(const Napi::CallbackInfo &info)
{
  return Napi::String::New(info.Env(), sessionInfoStr);
}

Napi::Value NodeIrSdk::IRSDKWrapper::UpdateTelemetry(const Napi::CallbackInfo &info)
{
  debug("IRSDKWrapper: updating telemetry...");
  if (isInitialized() && isConnected())
  {
    if (varHeadersArr.empty())
    {
      updateVarHeaders();
    }
    // if sim is not active, then no new data
    if (pHeader->status != irsdk_stConnected)
    {
      debug("IRSDKWrapper: not connected, break");
      lastTickCount = INT_MIN;
      return Napi::Boolean::New(info.Env(), false);
    }

    debug("IRSDKWrapper: finding lastest buffer");
    int latest = 0;
    for (int i = 1; i < pHeader->numBuf; i++)
      if (pHeader->varBuf[latest].tickCount < pHeader->varBuf[i].tickCount)
        latest = i;

    debug("IRSDKWrapper: lastest buffer " << latest);

    // if newer than last recieved, than report new data
    if (lastTickCount < pHeader->varBuf[latest].tickCount)
    {
      debug("IRSDKWrapper: new data, attempting to copy");
      if (data == NULL || dataLen != pHeader->bufLen)
      {
        debug("IRSDKWrapper: create new data array");
        if (data != NULL)
          delete[] data;
        data = NULL;

        if (pHeader->bufLen > 0)
        {
          dataLen = pHeader->bufLen;
          data = new char[dataLen];
        }
        else
        {
          debug("IRSDKWrapper: weird bufferLen.. skipping");
          return Napi::Boolean::New(info.Env(), false);
        }
      }
      // try to get data
      // try twice to get the data out
      for (int count = 0; count < 2; count++)
      {
        debug("IRSDKWrapper: copy attempt " << count);
        int curTickCount = pHeader->varBuf[latest].tickCount;
        memcpy(data, pSharedMem + pHeader->varBuf[latest].bufOffset, pHeader->bufLen);
        if (curTickCount == pHeader->varBuf[latest].tickCount)
        {
          lastTickCount = curTickCount;
          lastValidTime = time(NULL);
          debug("IRSDKWrapper: copy complete");
          return Napi::Boolean::New(info.Env(), true);
        }
      }
      // if here, the data changed out from under us.
      debug("IRSDKWrapper: copy failed");
      return Napi::Boolean::New(info.Env(), false);
    }
    // if older than last recieved, than reset, we probably disconnected
    else if (lastTickCount > pHeader->varBuf[latest].tickCount)
    {
      debug("IRSDKWrapper: ???");
      lastTickCount = INT_MIN;
      return Napi::Boolean::New(info.Env(), false);
    }
    // else the same, and nothing changed this tick
  }
  debug("IRSDKWrapper: no new telemetry data");
  return Napi::Boolean::New(info.Env(), false);
}

Napi::Value NodeIrSdk::IRSDKWrapper::GetLastTelemetryUpdateTS(const Napi::CallbackInfo &info)
{
  return Napi::Number::New(info.Env(), 1000.0f * lastValidTime);
}

const char *NodeIrSdk::IRSDKWrapper::getSessionInfoStr() const
{
  debug("IRSDKWrapper: getSessionInfoStr");
  if (isInitialized())
  {
    return pSharedMem + pHeader->sessionInfoOffset;
  }

  return NULL;
}

void NodeIrSdk::IRSDKWrapper::updateVarHeaders()
{
  debug("IRSDKWrapper: updating varHeaders...");
  varHeadersArr.clear();

  for (int index = 0; index < pHeader->numVars; ++index)
  {
    irsdk_varHeader *pVarHeader = &((irsdk_varHeader *)(pSharedMem + pHeader->varHeaderOffset))[index];
    varHeadersArr.push_back(pVarHeader);
  }
  debug("IRSDKWrapper: varHeaders update done.");
}

NodeIrSdk::IRSDKWrapper::TelemetryVar::TelemetryVar(irsdk_varHeader *varHeader) : header(varHeader)
{
  value = new char[irsdk_VarTypeBytes[varHeader->type] * varHeader->count];
  type = (irsdk_VarType)varHeader->type;
}

NodeIrSdk::IRSDKWrapper::TelemetryVar::~TelemetryVar()
{
  delete value;
}

Napi::Value NodeIrSdk::IRSDKWrapper::GetVarHeaders(const Napi::CallbackInfo &info)
{
  debug("IRSDKWrapper: getVarHeaders");
  Napi::Array headers = Napi::Array::New(info.Env());
  for (size_t i = 0; i < varHeadersArr.size(); i++)
  {
    headers[i] = Napi::External<irsdk_varHeader>::New(info.Env(), varHeadersArr[i]);
  }
  return headers;
}

Napi::Value NodeIrSdk::IRSDKWrapper::GetVarVal(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 1 || !info[0].IsObject())
  {
    Napi::TypeError::New(env, "TelemetryVar object expected").ThrowAsJavaScriptException();
    return env.Null();
  }

  TelemetryVar &var = *Napi::ObjectWrap<TelemetryVar>::Unwrap(info[0].As<Napi::Object>());

  debug("IRSDKWrapper: getVarVal " << var.header->name);
  if (data == NULL)
  {
    debug("no data available..");
    return Napi::Boolean::New(env, false);
  }

  int valueBytes = irsdk_VarTypeBytes[var.header->type] * var.header->count;
  memcpy(var.value, data + var.header->offset, valueBytes);
  return Napi::Boolean::New(env, true);
}