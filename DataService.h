#pragma once

#include "common.h"

#include <set>
#include <string>
#include <functional>

#include <iunoplugincontroller.h>
#include "RemoteAdapter.h"

#include "Vendor/sqlite3.h"

class DataService
{
public:
	DataService(IUnoPluginController& controller);
	~DataService();
	std::set<marker::marker_t> GetMarkers(const long long vfoFrequency, const long range, const bool onlyChecked = false);
	void SaveMarkerShow(const marker::marker_t marker);
	void SaveMarkerVote(const marker::marker_t marker);
	void RegisterCallback(std::function<void()> callback);
	int GetMyOid();
	void Init();

	void SaveMarker(marker::marker_t marker, const bool share);
	void DeleteMarker(marker::marker_t marker);
	void SyncMarkers();
	sync::syncinfo_t GetLastSync();
	int GetMarkerCount();

private:
	int m_oid;
	std::function<void()> m_dataChangedCallback;
	sqlite3* m_database;

	std::string m_pluginPath;
	IUnoPluginController& m_controller;
	RemoteAdapter m_remoteAdapter;

	void insertMarker(const marker::marker_t marker);
	void updateMarker(const marker::marker_t marker);
	void deleteMarker(const marker::marker_t marker);

	void insertSyncedMarker(const marker::marker_t marker);
	void updateSyncedMarker(const marker::marker_t marker);
};

