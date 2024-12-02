#pragma once

#include "irsdk/irsdk_defines.h"
#include <time.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <map>
#include <string>
#include <vector>
#include <napi.h>

namespace NodeIrSdk
{

  class IRSDKWrapper : public Napi::ObjectWrap<IRSDKWrapper>
  {
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    IRSDKWrapper(const Napi::CallbackInfo &info);
    ~IRSDKWrapper();

    Napi::Value Startup(const Napi::CallbackInfo &info);
    void Shutdown(const Napi::CallbackInfo &info);

    Napi::Value IsInitialized(const Napi::CallbackInfo &info);
    Napi::Value IsConnected(const Napi::CallbackInfo &info);

    Napi::Value UpdateTelemetry(const Napi::CallbackInfo &info);   // returns true if telemetry update available
    Napi::Value UpdateSessionInfo(const Napi::CallbackInfo &info); // returns true if session info update available

    Napi::Value GetSessionInfo(const Napi::CallbackInfo &info); // returns yaml string

    struct TelemetryVar
    {
      irsdk_varHeader *header;
      irsdk_VarType type;

      union
      { // choose correct based on irsdk_VarType
        char *value;
        float *floatValue;
        int *intValue;
        bool *boolValue;
        double *doubleValue;
      };

      TelemetryVar(irsdk_varHeader *varHeader);
      ~TelemetryVar();
    };

    Napi::Value GetVarHeaders(const Napi::CallbackInfo &info);

    Napi::Value GetVarVal(const Napi::CallbackInfo &info);

    Napi::Value GetLastTelemetryUpdateTS(const Napi::CallbackInfo &info); // returns JS compatible TS

  private:
    HANDLE hMemMapFile;
    const char *pSharedMem;
    const irsdk_header *pHeader;
    int lastTickCount;
    int lastSessionInfoUpdate;
    time_t lastValidTime;
    char *data;
    int dataLen;
    std::string sessionInfoStr;

    std::vector<irsdk_varHeader *> varHeadersArr;

    void updateVarHeaders(); // updates map and vector
    const char *getSessionInfoStr() const;
  };

} // namespace NodeIrSdk
