#ifndef _INCLUDE_BASEWRAPPED_I_H_
#define _INCLUDE_BASEWRAPPED_I_H_

#include <unordered_map>

class SMJS_Plugin;
class SMJS_BaseWrapped {
public:
	virtual void OnWrapperAttached(SMJS_Plugin *plugin, v8::Persistent<v8::Value> wrapper) = 0;
	virtual v8::Persistent<v8::Value> GetWrapper(SMJS_Plugin *plugin) = 0;
	virtual void OnPluginDestroyed(SMJS_Plugin *plugin) = 0;

	static void SetupTemplate(v8::Persistent<v8::FunctionTemplate> temp, v8::Persistent<v8::Template> proto) {}
};

#define WRAPPED_FUNC_DECL(name) Handle<Value> name(const Arguments& args)

#define WRAPPED_CLS(cls, super) \
	v8::Persistent<v8::Value> GetWrapper(SMJS_Plugin *plugin); \
	static std::unordered_map<PLUGIN_ID, v8::Persistent<FunctionTemplate>> templates; \
	static v8::Persistent<v8::FunctionTemplate> GetTemplateForPlugin(SMJS_Plugin *plugin); \
	static v8::Persistent<v8::Value> CreateWrapper(SMJS_Plugin *plugin, cls* obj); \
	static void SOnPluginDestroyed(SMJS_Plugin *plugin); \
	static void SetupTemplate(v8::Persistent<FunctionTemplate> temp, v8::Persistent<v8::ObjectTemplate> proto) \


#define WRAPPED_FUNC(name) proto->Set(v8::String::New(#name), v8::FunctionTemplate::New(name));

#define WRAPPED_CLS_CPP(cls, super) \
	std::unordered_map<PLUGIN_ID, v8::Persistent<FunctionTemplate>> cls::templates; \
	v8::Persistent<v8::FunctionTemplate> cls::GetTemplateForPlugin(SMJS_Plugin *plugin){ \
		auto it = templates.find(plugin->id); \
		if(it != templates.end()) return it->second; \
		 \
		auto temp = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New());; \
		 \
		auto inst = temp->InstanceTemplate(); \
		inst->SetInternalFieldCount(1); \
		 \
		auto proto = v8::Persistent<v8::ObjectTemplate>::New(temp->PrototypeTemplate()); \
		 \
		temp->Inherit(super::GetTemplateForPlugin(plugin)); \
		SetupTemplate(temp, proto); \
		 \
		templates.insert(std::pair<PLUGIN_ID, v8::Persistent<v8::FunctionTemplate>>(plugin->id, temp)); \
		 \
		plugin->RegisterDestroyCallback(SOnPluginDestroyed); \
		return temp; \
	} \
	v8::Persistent<v8::Value> cls::CreateWrapper(SMJS_Plugin *plugin, cls* obj) { \
		auto v = GetTemplateForPlugin(plugin)->GetFunction()->NewInstance(); \
		v->SetInternalField(0, v8::External::New((void*)obj)); \
		return v8::Persistent<v8::Value>::New(v); \
	} \
	v8::Persistent<v8::Value> cls::GetWrapper(SMJS_Plugin *plugin) { \
		auto it = wrappers.find(plugin->id); \
		if(it != wrappers.end()) return it->second; \
		auto wrapper = CreateWrapper(plugin, this); \
		++refCount;	\
		wrappers.insert(std::pair<PLUGIN_ID, v8::Persistent<v8::Value>>(plugin->id, wrapper)); \
		super::OnWrapperAttached(plugin, wrapper); \
		OnWrapperAttached(plugin, wrapper); \
		plugin->RegisterDestroyCallback(this); \
		return wrapper; \
	} \
	void cls::SOnPluginDestroyed(SMJS_Plugin *plugin) { \
		templates.erase(plugin->id); \
	}

#endif