#include "DataService.h"

#include "Vendor/sqlite3.h"

#include <Windows.h>
#include <future>
#include <iomanip>


DataService::DataService(IUnoPluginController& controller) :
	m_controller(controller),
	m_remoteAdapter(),
	m_selectedMarker(),
	m_typeSettings(),
	m_vfoOffset(0),
	m_oid(0)
{
	HMODULE hModule = GetModuleHandle(L"SDRunoPlugin_CloudMarkers");
	std::vector<wchar_t> pathBuf;
	DWORD copied = 0;
	do {
		pathBuf.resize(pathBuf.size() + MAX_PATH);
		copied = GetModuleFileName(hModule, pathBuf.data(), pathBuf.size());
	} while (copied >= pathBuf.size());

	pathBuf.resize(copied);

	m_pluginPath = std::string(pathBuf.begin(), pathBuf.end() - 29);
}

DataService::~DataService()
{
	sqlite3_close(m_database);
}

void DataService::prepareDatabase()
{

	if (sqlite3_open((m_pluginPath + "SDRunoPlugin_CloudMarkers.db").c_str(), &m_database) != SQLITE_OK)
	{
		return;
	}

	const char* createMarkersTable =
		"CREATE TABLE IF NOT EXISTS marker ("
		"	lid INTEGER PRIMARY KEY,"
		"	cid INTEGER DEFAULT 0,"
		"	oid INTEGER NOT NULL,"
		"	frequency INTEGER NOT NULL,"
		"	name TEXT NOT NULL,"
		"   type INTEGER NOT NULL,"
		"	modulation TEXT,"
		"   country TEXT,"
		"	location TEXT,"
		"	comment TEXT,"
		"	score INTEGER DEFAULT 0,"
		"	show NUMERIC DEFAULT 0,"
		"	vote INTEGER DEFAULT 0"
		")";

	if (sqlite3_exec(m_database, createMarkersTable, 0, 0, 0) != SQLITE_OK)
	{
		return;
	}

	const char* createConfigTable =
		"CREATE TABLE IF NOT EXISTS config ("
		"	key TEXT PRIMARY KEY,"
		"	value1 TEXT,"
		"	value2 TEXT,"
		"	value3 TEXT,"
		"	value4 TEXT,"
		"	value5 TEXT"
		")";

	if (sqlite3_exec(m_database, createConfigTable, 0, 0, 0) != SQLITE_OK)
	{
		return;
	}

	// Get DB Version
	int dbversion = 1;

	sqlite3_stmt* statement;
	int rc = sqlite3_prepare(m_database, "SELECT value1 FROM config WHERE key = 'DB_VERSION';", -1, &statement, 0);
	if (rc == SQLITE_OK)
	{
		rc = sqlite3_step(statement);
		if (rc == SQLITE_ROW)
		{
			dbversion = sqlite3_column_int(statement, 0);
		}
		sqlite3_finalize(statement);
	}

	if (dbversion < 2) { // Update to db version 2
		sqlite3_exec(m_database, "BEGIN TRANSACTION;", 0, 0, 0);
		sqlite3_exec(m_database, "ALTER TABLE marker ADD COLUMN tune_modulation INTEGER DEFAULT 0;", 0, 0, 0);
		sqlite3_exec(m_database, "ALTER TABLE marker ADD COLUMN tune_bandwidth INTEGER DEFAULT 0;", 0, 0, 0);
		sqlite3_exec(m_database, "ALTER TABLE marker ADD COLUMN tune_centered INTEGER DEFAULT 0;", 0, 0, 0);
		sqlite3_exec(m_database, "INSERT OR REPLACE INTO config(key, value1) VALUES('DB_VERSION', 2 );", 0, 0, 0);
		sqlite3_exec(m_database, "DELETE FROM marker WHERE cid > 0;", 0, 0, 0);
		sqlite3_exec(m_database, "DELETE FROM config WHERE key = 'LAST_SYNC';", 0, 0, 0);
		sqlite3_exec(m_database, "END TRANSACTION;", 0, 0, 0);
	}
}

void DataService::ReSync()
{
	sqlite3_exec(m_database, "BEGIN TRANSACTION;", 0, 0, 0);
	sqlite3_exec(m_database, "DELETE FROM marker WHERE cid > 0;", 0, 0, 0);
	sqlite3_exec(m_database, "DELETE FROM config WHERE key = 'LAST_SYNC';", 0, 0, 0);
	sqlite3_exec(m_database, "END TRANSACTION;", 0, 0, 0);

	m_dataChangedCallback();
}

void DataService::Init()
{
	prepareDatabase();

	// Get OwnerID
	sqlite3_stmt* statement;
	int rc = sqlite3_prepare(m_database, "SELECT value1 FROM config WHERE key = 'OID';", -1, &statement, 0);
	if (rc == SQLITE_OK)
	{
		rc = sqlite3_step(statement);
		if (rc == SQLITE_ROW)
		{
			m_oid = sqlite3_column_int(statement, 0);
		}
		sqlite3_finalize(statement);
	}

	if (m_oid == 0) {
		m_oid = m_remoteAdapter.Register();
		if (m_oid > 0) {
			int rc = sqlite3_prepare(m_database, "INSERT OR REPLACE INTO config (key, value1) VALUES ('OID', ?);", -1, &statement, 0);
			if (rc == SQLITE_OK)
			{
				sqlite3_bind_int(statement, 1, m_oid);
				sqlite3_step(statement);
				sqlite3_finalize(statement);
			}
		}
	}

	if (m_oid == 0)
		m_oid = 1000; // 1000 = Anonymous

	// Get QTH
	rc = sqlite3_prepare(m_database, "SELECT value1 FROM config WHERE key = 'QTH';", -1, &statement, 0);
	if (rc == SQLITE_OK)
	{
		rc = sqlite3_step(statement);
		if (rc == SQLITE_ROW)
		{
			if (sqlite3_column_bytes(statement, 0) > 0)
				m_qth = std::string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 0)));
		}
		sqlite3_finalize(statement);
	}

	m_typeSettings[0].hex_color(getConfig("ColorUnknown"), 0xd94545);
	m_typeSettings[1].hex_color(getConfig("ColorTimesignal"), 0xcccccc);
	m_typeSettings[2].hex_color(getConfig("ColorMorse"), 0xb12eb3);
	m_typeSettings[3].hex_color(getConfig("ColorDigimode"), 0xcfaa32);
	m_typeSettings[4].hex_color(getConfig("ColorVoice"), 0x247fc9);
	m_typeSettings[5].hex_color(getConfig("ColorBroadcast"), 0x2cb838);
	m_typeSettings[6].hex_color(getConfig("ColorNoise"), 0x777777);
	m_typeSettings[7].hex_color(getConfig("ColorNoise"), 0x777777);
	m_typeSettings[8].hex_color(getConfig("ColorBandmarker"), 0xf0d52b);

	m_vfoOffset = getConfigInt("VfoOffset", 10);

	m_dblClickSetting = getConfigInt("DblClickMode", 0);
}

int DataService::GetMyOid()
{
	return m_oid;
}


std::set<marker::marker_t> DataService::GetMarkers(const long long vfoFrequency, const long range, const bool onlyChecked)
{
	std::set<marker::marker_t> result;
	sqlite3_stmt* statement;

	int rc = sqlite3_prepare(m_database, "SELECT * FROM marker WHERE frequency >= ? AND frequency <= ? AND (show == 1 OR show == ?) ORDER BY frequency;", -1, &statement, 0);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_int(statement, 1, vfoFrequency - range);
		sqlite3_bind_int(statement, 2, vfoFrequency + range);
		sqlite3_bind_int(statement, 3, onlyChecked ? 1 : 0);

		rc = sqlite3_step(statement);
		while (rc == SQLITE_ROW)
		{
			marker::marker_t marker;

			marker.lid = sqlite3_column_int(statement, 0);
			marker.cid = sqlite3_column_int(statement, 1);
			marker.oid = sqlite3_column_int(statement, 2);
			marker.frequency = sqlite3_column_int64(statement, 3);
			marker.name = std::string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 4)));
			marker.type = sqlite3_column_int(statement, 5);
			if (sqlite3_column_bytes(statement, 6) > 0)
				marker.modulation = std::string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 6)));
			if (sqlite3_column_bytes(statement, 7) > 0)
				marker.country = std::string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 7)));
			if (sqlite3_column_bytes(statement, 8) > 0)
				marker.location = std::string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 8)));
			if (sqlite3_column_bytes(statement, 9) > 0)
				marker.comment = std::string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 9)));
			marker.score = sqlite3_column_int(statement, 10);
			marker.show = sqlite3_column_int(statement, 11) != 0;
			marker.vote = sqlite3_column_int(statement, 12);
			
			// -- Version 2
			marker.tune_modulation = sqlite3_column_int(statement, 13);
			marker.tune_bandwidth = sqlite3_column_int(statement, 14);
			marker.tune_centered = sqlite3_column_int(statement, 15) != 0;
			// --

			marker.score += marker.vote;

			marker.readOnly = marker.oid < 1000;
			marker.owner = marker.oid  >= 1000 && marker.oid == m_oid;
			marker.synced = marker.cid > 0;
			marker.deleteable = marker.owner && (marker.score < 10 || !marker.synced);

			if (marker.vote < 0)
				marker.flagsl.append(" (-)");

			if (marker.vote > 0)
				marker.flagsl.append(" (+)");

			if (marker.synced)
			{
				marker.flagsl.append(" shared");
			}
			else
			{
				marker.flags.append("L");
				marker.flagsl.append(" local");
			}

			if (marker.owner)
			{
				marker.flags.append("O");
				marker.flagsl.append(" owner");
			}			

			if (marker.readOnly)
			{
				marker.flags.append("R");
				marker.flagsl.append(" readonly");
			}

			result.insert(marker);

			rc = sqlite3_step(statement);
		}
		sqlite3_finalize(statement);

	}

	return result;
}

void DataService::SaveMarkerShow(const marker::marker_t marker)
{
	sqlite3_stmt* statement;
	int rc = sqlite3_prepare(m_database, "UPDATE marker SET show = ? WHERE lid = ?", -1, &statement, 0);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_int(statement, 1, marker.show ? 1 : 0);
		sqlite3_bind_int(statement, 2, marker.lid);
		sqlite3_step(statement);
		sqlite3_finalize(statement);
	}

	m_dataChangedCallback();
}

void DataService::SaveMarkerVote(const marker::marker_t marker)
{
	sqlite3_stmt* statement;
	int rc = sqlite3_prepare(m_database, "UPDATE marker SET vote = ? WHERE lid = ?", -1, &statement, 0);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_int(statement, 1, marker.vote);
		sqlite3_bind_int(statement, 2, marker.lid);
		sqlite3_step(statement);
		sqlite3_finalize(statement);
	}

	if (marker.synced) {
		m_remoteAdapter.Vote(marker, m_oid);
	}

	m_dataChangedCallback();
}


void DataService::SaveMarker(marker::marker_t marker, const bool share)
{
	if (marker.lid == 0) // INSERT
	{
		marker.oid = m_oid;
		marker.cid = 0;

		if (share) {
			int cid = m_remoteAdapter.Insert(marker, m_oid);
			if (cid > 0) {
				marker.cid = cid;
			}
		}

		insertMarker(marker);
	} 
	else if (marker.lid > 0 && (marker.cid == 0 || marker.oid >= 1000)) // UPDATE
	{
		if (marker.cid == 0 && share) // local -> shared
		{
			marker.oid = m_oid;

			int cid = m_remoteAdapter.Insert(marker, m_oid);
			if (cid > 0) {
				marker.cid = cid;
			}
		}
		else if (marker.cid > 0 && !share) // shared -> local
		{
			marker.cid = 0;
			marker.score = 0;
			marker.vote = 0;
			marker.oid = m_oid;
		} 
		else if (marker.cid > 0 && share) // shared update
		{
			m_remoteAdapter.Update(marker, m_oid);
		}

		updateMarker(marker);
	}

	m_dataChangedCallback();
}

void DataService::DeleteMarker(marker::marker_t marker)
{
	if (marker.lid > 0 && (marker.cid == 0 || marker.score < 10) && marker.oid == m_oid) {
		if (marker.cid > 0) {
			m_remoteAdapter.Delete(marker, m_oid);
		}
		deleteMarker(marker);
	}

	m_dataChangedCallback();
}

void DataService::insertMarker(const marker::marker_t marker)
{
	sqlite3_stmt* statement;
	int rc = sqlite3_prepare(m_database, "INSERT INTO marker VALUES(NULL, ?, ?, ?, ?, ?, ?, ?, ?, ?, 0, 0, 0, ?, ?, ?);", -1, &statement, 0);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_int(statement, 1, marker.cid);
		sqlite3_bind_int(statement, 2, marker.oid);
		sqlite3_bind_int64(statement, 3, marker.frequency);
		sqlite3_bind_text(statement, 4, marker.name.c_str(), marker.name.size(), NULL);
		sqlite3_bind_int(statement, 5, marker.type);
		sqlite3_bind_text(statement, 6, marker.modulation.c_str(), marker.modulation.size(), NULL);
		sqlite3_bind_text(statement, 7, marker.country.c_str(), marker.country.size(), NULL);
		sqlite3_bind_text(statement, 8, marker.location.c_str(), marker.location.size(), NULL);
		sqlite3_bind_text(statement, 9, marker.comment.c_str(), marker.comment.size(), NULL);
		// -- Version 2
		sqlite3_bind_int(statement, 10, marker.tune_modulation);
		sqlite3_bind_int(statement, 11, marker.tune_bandwidth);
		sqlite3_bind_int(statement, 12, marker.tune_centered ? 1 : 0);
		// --
		sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
}

void DataService::updateMarker(const marker::marker_t marker)
{
	sqlite3_stmt* statement;
	int rc = sqlite3_prepare(m_database, "UPDATE marker SET cid = ?, oid = ?, frequency = ?, name = ? , type = ? , modulation = ? , country = ? , location = ? , comment = ? , score = ? , vote = ?, tune_modulation = ?, tune_bandwidth = ?, tune_centered = ? WHERE lid = ?;", -1, &statement, 0);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_int(statement, 1, marker.cid);
		sqlite3_bind_int(statement, 2, marker.oid);
		sqlite3_bind_int64(statement, 3, marker.frequency);
		sqlite3_bind_text(statement, 4, marker.name.c_str(), marker.name.size(), NULL);
		sqlite3_bind_int(statement, 5, marker.type);
		sqlite3_bind_text(statement, 6, marker.modulation.c_str(), marker.name.size(), NULL);
		sqlite3_bind_text(statement, 7, marker.country.c_str(), marker.country.size(), NULL);
		sqlite3_bind_text(statement, 8, marker.location.c_str(), marker.location.size(), NULL);
		sqlite3_bind_text(statement, 9, marker.comment.c_str(), marker.comment.size(), NULL);
		sqlite3_bind_int(statement, 10, marker.score);
		sqlite3_bind_int(statement, 11, marker.vote);
		// -- Version 2
		sqlite3_bind_int(statement, 12, marker.tune_modulation);
		sqlite3_bind_int(statement, 13, marker.tune_bandwidth);
		sqlite3_bind_int(statement, 14, marker.tune_centered ? 1 : 0);
		// --
		sqlite3_bind_int(statement, 15, marker.lid);

		sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
}

void DataService::insertSyncedMarker(const marker::marker_t marker)
{
	sqlite3_stmt* statement;
	int rc = sqlite3_prepare(m_database, "INSERT INTO marker VALUES(NULL, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 0, ?, ?, ?, ?);", -1, &statement, 0);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_int(statement, 1, marker.cid);
		sqlite3_bind_int(statement, 2, marker.oid);
		sqlite3_bind_int64(statement, 3, marker.frequency);
		sqlite3_bind_text(statement, 4, marker.name.c_str(), marker.name.size(), NULL);
		sqlite3_bind_int(statement, 5, marker.type);
		sqlite3_bind_text(statement, 6, marker.modulation.c_str(), marker.modulation.size(), NULL);
		sqlite3_bind_text(statement, 7, marker.country.c_str(), marker.country.size(), NULL);
		sqlite3_bind_text(statement, 8, marker.location.c_str(), marker.location.size(), NULL);
		sqlite3_bind_text(statement, 9, marker.comment.c_str(), marker.comment.size(), NULL);
		sqlite3_bind_int(statement, 10, marker.score);
		sqlite3_bind_int(statement, 11, marker.vote);
		// -- Version 2
		sqlite3_bind_int(statement, 12, marker.tune_modulation);
		sqlite3_bind_int(statement, 13, marker.tune_bandwidth);
		sqlite3_bind_int(statement, 14, marker.tune_centered ? 1 : 0);
		// --
		sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
}

void DataService::updateSyncedMarker(const marker::marker_t marker)
{
	sqlite3_stmt* statement;
	int rc = sqlite3_prepare(m_database, "UPDATE marker SET oid = ?, frequency = ?, name = ? , type = ? , modulation = ? , country = ? , location = ? , comment = ? , score = ? , vote = ?, tune_modulation = ?, tune_bandwidth = ?, tune_centered = ?  WHERE cid = ?;", -1, &statement, 0);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_int(statement, 1, marker.oid);
		sqlite3_bind_int64(statement, 2, marker.frequency);
		sqlite3_bind_text(statement, 3, marker.name.c_str(), marker.name.size(), NULL);
		sqlite3_bind_int(statement, 4, marker.type);
		sqlite3_bind_text(statement, 5, marker.modulation.c_str(), marker.name.size(), NULL);
		sqlite3_bind_text(statement, 6, marker.country.c_str(), marker.country.size(), NULL);
		sqlite3_bind_text(statement, 7, marker.location.c_str(), marker.location.size(), NULL);
		sqlite3_bind_text(statement, 8, marker.comment.c_str(), marker.comment.size(), NULL);
		sqlite3_bind_int(statement, 9, marker.score);
		sqlite3_bind_int(statement, 10, marker.vote);
		// -- Version 2
		sqlite3_bind_int(statement, 11, marker.tune_modulation);
		sqlite3_bind_int(statement, 12, marker.tune_bandwidth);
		sqlite3_bind_int(statement, 13, marker.tune_centered ? 1 : 0);
		// --
		sqlite3_bind_int(statement, 14, marker.cid);

		sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
}

void DataService::deleteMarker(const marker::marker_t marker)
{
	sqlite3_stmt* statement;
	int rc = sqlite3_prepare(m_database, "DELETE FROM marker WHERE lid = ?;", -1, &statement, 0);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_int(statement, 1, marker.lid);
		sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
}

void DataService::RegisterCallback(std::function<void()> callback)
{
	m_dataChangedCallback = callback;
}

sync::syncinfo_t DataService::GetLastSync() {
	sync::syncinfo_t result = { "0", "0", "Never" };

	sqlite3_stmt* statement;
	int rc = sqlite3_prepare(m_database, "SELECT value1, value2, value3 FROM config WHERE key = 'LAST_SYNC';", -1, &statement, 0);
	if (rc == SQLITE_OK)
	{
		rc = sqlite3_step(statement);
		if (rc == SQLITE_ROW)
		{
			if (sqlite3_column_bytes(statement, 0) > 0)
				result.commit = std::string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 0)));
			if (sqlite3_column_bytes(statement, 1) > 0)
				result.count = std::string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
			if (sqlite3_column_bytes(statement, 2) > 0)
				result.time = std::string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)));
		}
		sqlite3_finalize(statement);
	}

	return result;
}

int DataService::GetMarkerCount()
{
	int count = -1;

	sqlite3_stmt* statement;
	int rc = sqlite3_prepare(m_database, "SELECT COUNT(*) AS count FROM marker;", -1, &statement, 0);
	if (rc == SQLITE_OK)
	{
		rc = sqlite3_step(statement);
		if (rc == SQLITE_ROW)
		{
			count = sqlite3_column_int(statement, 0);
		}
		sqlite3_finalize(statement);
	}

	return count;
}

void DataService::SyncMarkers(std::function<void(bool download, int progress) > progressHandler)
{
	auto syncinfo = GetLastSync();
	int lastCommit = std::stoi(syncinfo.commit);

	std::string commit = syncinfo.commit;
	std::string count = "0";
	std::string time = sync::currentDateTime();

	auto syncresult = m_remoteAdapter.Get(lastCommit, m_oid, [&](uint64_t c, uint64_t t) {
		progressHandler(true, c * 100 / t);
		return true;
	});

	progressHandler(true, 100);

	if (syncresult.commit > lastCommit && syncresult.items.size() > 0)
	{
		int t = syncresult.items.size();
		int c = 0;

		commit = std::to_string(syncresult.commit);
		count = std::to_string(syncresult.items.size());

		for (auto item : syncresult.items)
		{
			if (item.action == "DELETE")
				deleteMarker(item.marker);

			if (item.action == "INSERT" && (lastCommit == 0 || item.marker.oid != m_oid))
				insertSyncedMarker(item.marker);

			if (item.action == "UPDATE")
				updateSyncedMarker(item.marker);

			progressHandler(false, ++c * 100 / t);
		}
	}

	progressHandler(false, 100);

	sqlite3_stmt* statement;
	int rc = sqlite3_prepare(m_database, "INSERT OR REPLACE INTO config (key, value1, value2, value3) VALUES ('LAST_SYNC', ?, ?, ?);", -1, &statement, 0);
	if (rc == SQLITE_OK)
	{

		sqlite3_bind_text(statement, 1, commit.c_str(), commit.size(), NULL);
		sqlite3_bind_text(statement, 2, count.c_str(), count.size(), NULL);
		sqlite3_bind_text(statement, 3, time.c_str(), time.size(), NULL);
		sqlite3_step(statement);
		sqlite3_finalize(statement);
	}

	m_dataChangedCallback();
}

void DataService::setSelectedMarker(marker::marker_t marker)
{
	m_selectedMarker = marker;
}
marker::marker_t DataService::getSelectedMarker()
{
	return m_selectedMarker;
}

std::string DataService::getConfig(std::string key)
{
	std::string value;
	m_controller.GetConfigurationKey("CloudMarker." + key, value);
	return value;
}

void DataService::setConfig(std::string key, std::string value)
{
	m_controller.SetConfigurationKey("CloudMarker." + key, value);
}

int DataService::getConfigInt(std::string key, int defaultValue)
{
	try {
		return std::stoi(getConfig(key));
	}
	catch(...)
	{
		return defaultValue;
	}
}
void DataService::setConfigInt(std::string key, int value)
{
	setConfig(key, std::to_string(value));
}

settings::types_t DataService::GetTypeSettings()
{
	return m_typeSettings;
}

void DataService::SetTypeSettings(settings::types_t types)
{
	m_typeSettings = types;

	setConfig("ColorUnknown", m_typeSettings[0].hex_color());
	setConfig("ColorTimesignal", m_typeSettings[1].hex_color());
	setConfig("ColorMorse", m_typeSettings[2].hex_color());
	setConfig("ColorDigimode", m_typeSettings[3].hex_color());
	setConfig("ColorVoice", m_typeSettings[4].hex_color());
	setConfig("ColorBroadcast", m_typeSettings[5].hex_color());
	setConfig("ColorNoise", m_typeSettings[6].hex_color());
	setConfig("ColorBandmarker", m_typeSettings[8].hex_color());

	m_dataChangedCallback();
}

int DataService::GetDblClickSetting()
{
	return m_dblClickSetting;
}

void DataService::SetDblClickSetting(int mode)
{
	m_dblClickSetting = mode;
	setConfigInt("DblClickMode", mode);
}

void DataService::SaveWindowPos(channel_t channel, nana::point point)
{
	setConfigInt("Window" + std::to_string(channel) + "PosX", point.x);
	setConfigInt("Window" + std::to_string(channel) + "PosY", point.y);
}

nana::point DataService::LoadWindowPos(channel_t channel)
{
	int x = getConfigInt("Window" + std::to_string(channel) + "PosX", -1);
	int y = getConfigInt("Window" + std::to_string(channel) + "PosY", -1);
	return nana::point(x, y);
}

int DataService::GetVfoOffset() {
	return m_vfoOffset;
}

void DataService::SetVfoOffset(int offset) {
	if (offset >= 10 && offset <= 5000) {
		m_vfoOffset = offset;
		setConfigInt("VfoOffset", offset);
		m_dataChangedCallback();
	}
}

bool DataService::UpdateAvailable() {
	return version::version != m_remoteAdapter.GetNewestVersion();
}

std::string DataService::GetQTH()
{
	return m_qth;
}

void DataService::SetQTH(std::string qth)
{
	m_qth = util::uppercase(qth);
	sqlite3_stmt* statement;
	int rc = sqlite3_prepare(m_database, "INSERT OR REPLACE INTO config (key, value1) VALUES ('QTH', ?);", -1, &statement, 0);
	if (rc == SQLITE_OK)
	{

		sqlite3_bind_text(statement, 1, m_qth.c_str(), m_qth.size(), NULL);
		sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
}

void DataService::Tune(channel_t channel, marker::marker_t marker)
{
	auto frequency = marker.frequency;

	if (marker.tune_modulation > 0 && marker.tune_modulation < 14){

		int mapping[] = { 0,1,7,8,9,3,4,5,6,7,8,9,10,11 };

		auto demodulator = static_cast<IUnoPluginController::DemodulatorType>(mapping[marker.tune_modulation]);
		m_controller.SetDemodulatorType(channel, IUnoPluginController::DemodulatorType::DemodulatorNone);

		if ((marker.tune_modulation >= 2) && (marker.tune_modulation <= 4)) // SAM
			m_controller.SetDemodulatorType(channel, IUnoPluginController::DemodulatorType::DemodulatorSAM);
		
		m_controller.SetDemodulatorType(channel, demodulator);

		if (marker.tune_bandwidth > 0) {
			m_controller.SetFilterBandwidth(channel, marker.tune_bandwidth);
		}

		if (marker.tune_centered) {
			if (marker.tune_modulation == 10) // LSB
				frequency += marker.tune_bandwidth / 2;
			if (marker.tune_modulation == 11 || marker.tune_modulation == 13) // USB + Digital
				frequency -= marker.tune_bandwidth / 2;
 		}

	}

	m_controller.SetVfoFrequency(channel, frequency);
}
