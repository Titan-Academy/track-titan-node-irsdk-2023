#pragma once

#include "irsdk/irsdk_defines.h"
#include <napi.h>

namespace NodeIrSdk
{
  Napi::Value broadcastCmd(const Napi::CallbackInfo &info);
}
