#include "IrSdkCommand.h"
#include "irsdk/irsdk_defines.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

Napi::Value NodeIrSdk::broadcastCmd(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();

	if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber())
	{
		Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
		return env.Null();
	}

	int cmd = info[0].As<Napi::Number>().Int32Value();
	int var1 = info[1].As<Napi::Number>().Int32Value();
	int var2 = info.Length() >= 3 ? info[2].As<Napi::Number>().Int32Value() : 0;
	int var3 = info.Length() >= 4 ? info[3].As<Napi::Number>().Int32Value() : 0;

	static unsigned int msgId = RegisterWindowMessageA(IRSDK_BROADCASTMSGNAME);

	if (cmd >= 0 && cmd < irsdk_BroadcastLast)
	{
		if (info.Length() >= 4)
		{
			SendNotifyMessage(HWND_BROADCAST, msgId, MAKELONG(cmd, var1), MAKELONG(var2, var3));
		}
		else
		{
			SendNotifyMessage(HWND_BROADCAST, msgId, MAKELONG(cmd, var1), var2);
		}
	}

	return Napi::Boolean::New(env, true);
}
