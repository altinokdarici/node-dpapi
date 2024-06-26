#include <napi.h>
#include <uv.h>
#include <Windows.h>
#include <dpapi.h>
#include <functional>

void ProtectDataCommon(bool protect, const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();
	
	if (info.Length() != 3) {
		Napi::Error::New(env, "3 arguments are required").ThrowAsJavaScriptException();
	}

	if (info[0].IsNullOrUndefined() || !info[0].IsUint8Array())
	{
		Napi::Error::New(env, "First argument, data, must be a valid Uint8Array").ThrowAsJavaScriptException();
	}

	if (!info[1].IsNull() && !info[1].IsUint8Array())
	{
		Napi::Error::New(env, "Second argument, optionalEntropy, must be null or an ArrayBuffer").ThrowAsJavaScriptException();
	}

	if (info[2].IsNullOrUndefined() || !info[2].IsString())
	{
		Napi::Error::New(env, "Third argument, scope, must be a string").ThrowAsJavaScriptException();
	}

	DWORD flags = 0;
	if (!info[2].IsNullOrUndefined())
	{
		Napi::String strData(env, isolate, info[2]);
		std::string scope(*strData);
		if (stricmp(scope.c_str(), "LocalMachine") == 0)
		{
			flags = CRYPTPROTECT_LOCAL_MACHINE;
		}
	}

	auto buffer = info[0].As<Napi::Buffer<char>>().Data();
	auto len = info[0].As<Napi::Buffer<char>>().Length();

	DATA_BLOB entropyBlob;
	entropyBlob.pbData = nullptr;
	if (!info[1].IsNull())
	{
		entropyBlob.pbData = reinterpret_cast<BYTE*>(info[1].As<Napi::Buffer<char>>().Data());
		entropyBlob.cbData = info[1].As<Napi::Buffer<char>>().Length();
	}

	DATA_BLOB dataIn;
	DATA_BLOB dataOut;

	// initialize input data
	dataIn.pbData = reinterpret_cast<BYTE*>(buffer);
	dataIn.cbData = len;

	bool success = false;

	// Call either Protect or Unprotect based on the flag
	if (protect)
	{
		success = CryptProtectData(
			&dataIn,
			nullptr, // Description string
			entropyBlob.pbData ? &entropyBlob : nullptr,
			nullptr, // reserved
			nullptr, // pass null for the prompt structure
			flags, // dwFlags
			&dataOut);
	}
	else
	{
		success = CryptUnprotectData(
			&dataIn,
			nullptr, // Description string
			entropyBlob.pbData ? &entropyBlob : nullptr,
			nullptr, // reserved
			nullptr, // pass null for the prompt structure
			flags, // dwFlags
			&dataOut);
	}

	if (!success)
	{
		DWORD errorCode = GetLastError();
		Napi::Error::New(env, "Decryption failed. Error code:" + errorCode).ThrowAsJavaScriptException();
	}

	// Copy and free the buffer
	auto returnBuffer = Napi::Buffer::Copy(env, reinterpret_cast<const char*>(dataOut.pbData), dataOut.cbData);
	LocalFree(dataOut.pbData);

	return returnBuffer;
}

// public unsafe static byte[] Protect(byte[] userData, byte[] optionalEntropy, DataProtectionScope scope) 
Napi::Value protectData(const Napi::CallbackInfo& info)
{
	ProtectDataCommon(true, info);
}

// public unsafe static byte[] Unprotect(byte[] encryptedData, byte[] optionalEntropy, DataProtectionScope scope)
Napi::Value unprotectData(const Napi::CallbackInfo& info)
{
	ProtectDataCommon(false, info);
}

Napi::Object init(Napi::Env env, Napi::Object exports)
{
	(
		target).Set(Napi::String::New(env, "protectData"),
		Napi::GetFunction(Napi::Function::New(env, protectData)));

	(
		target).Set(Napi::String::New(env, "unprotectData"),
		Napi::GetFunction(Napi::Function::New(env, unprotectData)));
}

NODE_API_MODULE(binding, init)