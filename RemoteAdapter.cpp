#include "RemoteAdapter.h"

#include "Vendor/httplib.h"
#include "Vendor/Jzon.h"

#define API_VERSION "2"

int RemoteAdapter::Register()
{
	std::string remoteUrl = std::string("/register.php")
		.append("?version=").append(API_VERSION)
		.append("&token=").append(remote::token);

	httplib::Client remoteClient(remote::server, 80);
	httplib::Result result = remoteClient.Post(
		remoteUrl.c_str()
	);

	if (result.error() == httplib::Error::Success) {
		if (result->status == 201) {
			return std::stoi(result->body);
		}
		else {
			auto errorMessage = result->body;
			return 0;
		}
	}
}

int RemoteAdapter::Insert(marker::marker_t marker, int oid)
{
	std::string remoteUrl = std::string("/insert.php")
		.append("?version=").append(API_VERSION)
		.append("&token=").append(remote::token)
		.append("&oid=").append(std::to_string(oid));

	httplib::Client remoteClient(remote::server, 80);
	httplib::Result result = remoteClient.Post(
		remoteUrl.c_str(),
		markerToJson(marker),
		"application/json"
	);

	if (result.error() == httplib::Error::Success) {
		if (result->status == 201) {
			return std::stoi(result->body);
		}
		else {
			auto errorMessage = result->body;
			return 0;
		}
	}
}

bool RemoteAdapter::Update(marker::marker_t marker, int oid)
{
	std::string remoteUrl = std::string("/update.php")
		.append("?version=").append(API_VERSION)
		.append("&token=").append(remote::token)
		.append("&oid=").append(std::to_string(oid))
		.append("&cid=").append(std::to_string(marker.cid));
	
	httplib::Client remoteClient(remote::server, 80);
	httplib::Result result = remoteClient.Put(
		remoteUrl.c_str(),
		markerToJson(marker),
		"application/json"
	);

	if (result.error() == httplib::Error::Success) {
		if (result->status == 200) {
			return true;
		}
		else {
			auto errorMessage = result->body;
			return false;
		}
	}
}

bool RemoteAdapter::Delete(marker::marker_t marker, int oid)
{
	std::string remoteUrl = std::string("/delete.php")
		.append("?version=").append(API_VERSION)
		.append("&token=").append(remote::token)
		.append("&oid=").append(std::to_string(oid))
		.append("&cid=").append(std::to_string(marker.cid));

	httplib::Client remoteClient(remote::server, 80);
	httplib::Result result = remoteClient.Delete(
		remoteUrl.c_str()
	);

	if (result.error() == httplib::Error::Success) {
		if (result->status == 200) {
			return true;
		}
		else {
			auto errorMessage = result->body;
			return false;
		}
	}
}

bool RemoteAdapter::Vote(const marker::marker_t marker, const int oid)
{
	std::string remoteUrl = std::string("/vote.php")
		.append("?version=").append(API_VERSION)
		.append("&token=").append(remote::token)
		.append("&oid=").append(std::to_string(oid))
		.append("&cid=").append(std::to_string(marker.cid));

	httplib::Client remoteClient(remote::server, 80);
	httplib::Result result = remoteClient.Post(
		remoteUrl.c_str(),
		"[" + std::to_string(marker.vote) + "]",
		"application/json"
	);

	if (result.error() == httplib::Error::Success) {
		if (result->status == 201) {
			return true;
		}
		else {
			auto errorMessage = result->body;
			return false;
		}
	}
}

std::string RemoteAdapter::GetNewestVersion() {
	std::string remoteUrl = std::string("/version.txt");

	httplib::Client remoteClient(remote::server, 80);
	httplib::Result result = remoteClient.Get(remoteUrl.c_str());
	if (result.error() == httplib::Error::Success) {
		if (result->status == 200 && !result->body.empty())
			return result->body;
	}

	return "ERR";
}

sync::syncresult_t RemoteAdapter::Get(int afterCommit, int oid, std::function<bool(uint64_t current, uint64_t total)> progress)
{
	std::string remoteUrl = std::string("/get.php")
		.append("?version=").append(API_VERSION)
		.append("&token=").append(remote::token)
		.append("&oid=").append(std::to_string(oid))
		.append("&commit=").append(std::to_string(afterCommit));

	httplib::Client remoteClient(remote::server, 80);
	httplib::Result result = remoteClient.Get(remoteUrl.c_str(), progress);
	
	if (result.error() == httplib::Error::Success) {
		if (result->status == 200) {
			sync::syncresult_t syncresult;

			Jzon::Parser parser;
			Jzon::Node root = parser.parseString(result->body);
			
			if (root.isArray() && root.getCount() == 2) {
				Jzon::Node commit = root.get(0);
				if (commit.isNumber())
					syncresult.commit = root.get(0).toInt(0);

				Jzon::Node items = root.get(1);
				if (items.isArray() && items.getCount() > 0) {
					for (int i = 0; i < items.getCount(); i++)
					{
						Jzon::Node item = items.get(i);
						if (item.isArray() && item.getCount() > 0)
						{
							sync::syncitem_t syncitem;

							if (item.get(0).isString())
								syncitem.action = item.get(0).toString("UNKNOWN");

							if (syncitem.action == "DELETE" && item.getCount() == 2) {
								marker::marker_t marker;

								if (item.get(1).isNumber())
									marker.cid = item.get(1).toInt();

								syncitem.marker = marker;
							}

							if ((syncitem.action == "INSERT" || syncitem.action == "UPDATE") && item.getCount() == 15) {
								marker::marker_t marker;

								if (item.get(1).isNumber())
									marker.cid = item.get(1).toInt();
								if (item.get(2).isNumber())
									marker.oid = item.get(2).toInt();
								if (item.get(3).isNumber())
									marker.frequency = static_cast<long long>(item.get(3).toDouble());
								if (item.get(4).isString())
									marker.name = item.get(4).toString();
								if (item.get(5).isNumber())
									marker.type = item.get(5).toInt();
								if (!item.get(6).isNull() && item.get(6).isString())
									marker.modulation = item.get(6).toString();
								if (!item.get(7).isNull() && item.get(7).isString())
									marker.country = item.get(7).toString();
								if (!item.get(8).isNull() && item.get(8).isString())
									marker.location = item.get(8).toString();
								if (!item.get(9).isNull() && item.get(9).isString())
									marker.comment = item.get(9).toString();
								if (item.get(10).isNumber())
									marker.score = item.get(10).toInt();
								if (item.get(11).isNumber())
									marker.vote = item.get(11).toInt();
								// -- Version 2
								if (item.get(12).isNumber())
									marker.tune_modulation = item.get(12).toInt();
								if (item.get(13).isNumber())
									marker.tune_bandwidth = item.get(13).toInt();
								if (item.get(14).isBool())
									marker.tune_centered = item.get(14).toBool();
								// --
								syncitem.marker = marker;
							}

							syncresult.items.push_back(syncitem);
						}
					}
				}
			}

			return syncresult;
		}
		else {
			auto errorMessage = result->body;
			return sync::syncresult_t{ 0 };
		}
	}
	
}

std::string RemoteAdapter::markerToJson(marker::marker_t marker) {
	std::string result;

	Jzon::Node array = Jzon::array();
	array.add(marker.frequency);
	array.add(marker.name);
	array.add(marker.type);
	array.add(marker.modulation);
	array.add(marker.country);
	array.add(marker.location);
	array.add(marker.comment);

	// -- Version 2
	array.add(marker.tune_modulation);
	array.add(marker.tune_bandwidth);
	array.add(marker.tune_centered);
	//--

	Jzon::Writer stringWriter;
	stringWriter.writeString(array, result);

	return result;
}