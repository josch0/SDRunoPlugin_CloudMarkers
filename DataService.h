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
	void SyncMarkers(std::function<void(bool download, int progress)> progressHandler);
	sync::syncinfo_t GetLastSync();
	int GetMarkerCount();

	void setSelectedMarker(marker::marker_t marker);
	marker::marker_t getSelectedMarker();	

	settings::types_t GetTypeSettings();
	void SetTypeSettings(settings::types_t types);

	void SaveWindowPos(channel_t channel, nana::point point);
	nana::point LoadWindowPos(channel_t channel);

	int GetVfoOffset();
	void SetVfoOffset(int offset);

	bool UpdateAvailable();

private:
	int m_oid;
	int m_vfoOffset;
	marker::marker_t m_selectedMarker;
	settings::types_t m_typeSettings;
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

	std::string getConfig(std::string key);
	void setConfig(std::string key, std::string value);
	int getConfigInt(std::string key, int defaultValue = 0);
	void setConfigInt(std::string key, int value);
};

