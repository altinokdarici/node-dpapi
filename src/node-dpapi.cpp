#include <napi.h>
#include <uv.h>
#include <Windows.h>
#include <dpapi.h>
#include <functional>
#include <iostream> // Include for std::cout


Napi::Value ProtectDataCommon(bool protect, const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();

	if (info.Length() != 3)
	{
		Napi::Error::New(env, "3 arguments are required").ThrowAsJavaScriptException();
		return env.Null();
	}

	if (info[0].IsNull() || info[0].IsUndefined() || !info[0].IsTypedArray() || info[0].As<Napi::TypedArray>().TypedArrayType() != napi_uint8_array)
	{
		Napi::Error::New(env, "First argument, data, must be a valid Uint8Array").ThrowAsJavaScriptException();
		return env.Null();
	}

	if (!info[1].IsNull() && !info[1].IsTypedArray() && info[1].As<Napi::TypedArray>().TypedArrayType() != napi_uint8_array)
	{
		Napi::Error::New(env, "Second argument, optionalEntropy, must be null or an ArrayBuffer").ThrowAsJavaScriptException();
		return env.Null();
	}

	if (info[2].IsNull() || info[2].IsUndefined() || !info[2].IsString())
	{
		Napi::Error::New(env, "Third argument, scope, must be a string").ThrowAsJavaScriptException();
		return env.Null();
	}   

	DWORD flags = 0;

	std::string scope = info[2].As<Napi::String>().Utf8Value();
	if (stricmp(scope.c_str(), "LocalMachine") == 0)
	{
		flags = CRYPTPROTECT_LOCAL_MACHINE;
	}

	auto buffer = info[0].As<Napi::Buffer<char>>().Data();
	auto len = info[0].As<Napi::Buffer<char>>().Length();

	DATA_BLOB entropyBlob;
	entropyBlob.pbData = nullptr;
	if (!info[1].IsNull()){
		entropyBlob.pbData = reinterpret_cast<BYTE *>(info[1].As<Napi::Buffer<char>>().Data());
		entropyBlob.cbData = info[1].As<Napi::Buffer<char>>().Length();
	}

	DATA_BLOB dataIn;
	DATA_BLOB dataOut;

	// initialize input data
	dataIn.pbData = reinterpret_cast<BYTE *>(buffer);
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
			flags,	 // dwFlags
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
			flags,	 // dwFlags
			&dataOut);
	}

	if (!success)
	{
		DWORD errorCode = GetLastError();
		Napi::Error::New(env, "Decryption failed. Error code:" + errorCode).ThrowAsJavaScriptException();
		return env.Null();
	}

	// Copy and free the buffer
	auto returnBuffer = Napi::Buffer<char>::Copy(env, reinterpret_cast<const char *>(dataOut.pbData), dataOut.cbData);
	LocalFree(dataOut.pbData);

	return returnBuffer;
}

// public unsafe static byte[] Protect(byte[] userData, byte[] optionalEntropy, DataProtectionScope scope)
Napi::Value protectData(const Napi::CallbackInfo &info)
{
	return ProtectDataCommon(true, info);
}

// public unsafe static byte[] Unprotect(byte[] encryptedData, byte[] optionalEntropy, DataProtectionScope scope)
Napi::Value unprotectData(const Napi::CallbackInfo &info)
{
	return ProtectDataCommon(false, info);
}

Napi::Object init(Napi::Env env, Napi::Object exports)
{
	exports.Set(Napi::String::New(env, "protectData"), Napi::Function::New(env, protectData));

	exports.Set(Napi::String::New(env, "unprotectData"), Napi::Function::New(env, unprotectData));

	return exports;
}

NODE_API_MODULE(addon, init)