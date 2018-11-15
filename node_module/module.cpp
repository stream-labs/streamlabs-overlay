#include <node.h>
#include "overlays.h"  // NOLINT(build/include)

namespace demo {

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;


void Method(const FunctionCallbackInfo<Value>& args) 
{
	start_overlays_thread();
	Isolate* isolate = args.GetIsolate();
	args.GetReturnValue().Set(String::NewFromUtf8(isolate, "world"));
}

void Initialize(Local<Object> exports) {
  NODE_SET_METHOD(exports, "hello", Method);
}
 
NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)

}  // namespace demo