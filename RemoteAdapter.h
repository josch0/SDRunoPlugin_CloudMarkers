#pragma once

#include "common.h"

class RemoteAdapter
{
public:
	int Register();
	int Insert(marker::marker_t marker, int oid);
	bool Update(marker::marker_t marker, int oid);
	bool Delete(marker::marker_t marker, int oid);
	bool Vote(const marker::marker_t marker, int oid);

	std::string GetNewestVersion();

	sync::syncresult_t Get(int afterCommit, int oid, std::function<bool(uint64_t current, uint64_t total)> progress);

private:
	std::string markerToJson(marker::marker_t marker);
};

