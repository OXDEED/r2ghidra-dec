/* radare - LGPL - Copyright 2019 - thestr4ng3r */

#include "R2LoadImage.h"
#include "R2Scope.h"
#include "R2Architecture.h"
#include "R2TypeFactory.h"
#include "R2CommentDatabase.h"

#include <iostream>

static const std::map<std::string, std::string> arch_map = {
		{ "x86", "x86" }
};

std::string FilenameFromCore(RCore *core)
{
	return core->bin->file;
}

std::string SleighIdFromCore(RCore *core)
{
	const char *arch = r_config_get(core->config, "asm.arch");
	bool be = r_config_get_i(core->config, "cfg.bigendian") != 0;
	ut64 bits = r_config_get_i(core->config, "asm.bits");

	auto arch_it = arch_map.find(arch);
	if(arch_it == arch_map.end())
		throw LowlevelError("Could not match asm.arch " + std::string(arch) + " to sleigh arch.");

	return arch_it->second + ":" + (be ? "BE" : "LE") + ":" + to_string(bits) + ":default";
}

R2Architecture::R2Architecture(RCore *core)
	: SleighArchitecture(FilenameFromCore(core), SleighIdFromCore(core), &cout),
	core(core)
{
}

void R2Architecture::buildLoader(DocumentStorage &store)
{
	collectSpecFiles(*errorstream);
	loader = new R2LoadImage(core);
}

Scope *R2Architecture::buildGlobalScope()
{
	Scope *globalscope = symboltab->getGlobalScope();
	if(globalscope)
		return globalscope;

	globalscope = new R2Scope(this);
	symboltab->attachScope(globalscope, nullptr);
	return globalscope;
}

void R2Architecture::buildTypegrp(DocumentStorage &store)
{
	types = r2TypeFactory = new R2TypeFactory(this);

	// TODO: load from r2?
	types->setCoreType("void", 1, TYPE_VOID, false);
	types->setCoreType("bool", 1, TYPE_BOOL, false);
	types->setCoreType("uint8_t", 1, TYPE_UINT, false);
	types->setCoreType("uint16_t", 2, TYPE_UINT, false);
	types->setCoreType("uint32_t", 4, TYPE_UINT, false);
	types->setCoreType("uint64_t", 8, TYPE_UINT, false);
	types->setCoreType("char", 1, TYPE_INT, true);
	types->setCoreType("int8_t", 1, TYPE_INT, false);
	types->setCoreType("int16_t", 2, TYPE_INT, false);
	types->setCoreType("int32_t", 4, TYPE_INT, false);
	types->setCoreType("int64_t", 8, TYPE_INT, false);
	types->setCoreType("float", 4, TYPE_FLOAT, false);
	types->setCoreType("double", 8, TYPE_FLOAT, false);
	types->setCoreType("float16", 16 ,TYPE_FLOAT, false);
	types->setCoreType("undefined", 1, TYPE_UNKNOWN, false);
	types->setCoreType("undefined2", 2, TYPE_UNKNOWN, false);
	types->setCoreType("undefined4", 4, TYPE_UNKNOWN, false);
	types->setCoreType("undefined8", 8, TYPE_UNKNOWN, false);
	types->setCoreType("code", 1, TYPE_CODE, false);
	types->setCoreType("wchar", 2, TYPE_INT, true);
	types->cacheCoreTypes();
}

void R2Architecture::buildCommentDB(DocumentStorage &store)
{
	commentdb = new R2CommentDatabase(this);
}