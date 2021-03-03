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

	sync::syncresult_t Get(int afterCommit, int oid);

private:
	std::string markerToJson(marker::marker_t marker);
};

