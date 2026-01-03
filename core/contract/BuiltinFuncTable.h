#pragma once
#include <unordered_map>
#include <string>

struct BuiltinFuncTable
{
	BuiltinFuncTable()
	{//TODO refactor
		uint32_t off = 0xFFFF0000;

		_tbl.insert({"print", off++});
		_tbl.insert({"println", off++});
		_tbl.insert({"exit", off++});
		_tbl.insert({"readln", off++});
		_tbl.insert({"eprint", off++});
		_tbl.insert({"eprintln", off++});
	}

	uint32_t GetFuncId(const std::string& funcName) const
	{
		auto found = _tbl.find(funcName);
		return found != _tbl.end() ? found->second : 0;
	}

	std::unordered_map<std::string, uint32_t> _tbl;
};
