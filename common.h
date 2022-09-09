#pragma once

#include <nana/gui.hpp>
#include <iomanip>

namespace remote {
	const char* const server = "cloudmarkers.josch0.dev";
	const char* const token = "fd3852f0950c99e7919b6ae6d35f9ee13822ae97";
}

namespace version {
	const std::string const version = "0.4.0";
}

namespace settings {
	struct type_t {
		std::string name;
		int color;

		bool hex_color(std::string value, int defaulColor = 0) {
			color = defaulColor;

			if (value.empty() || value.length() != 7 || value[0] != '#')
				return false;

			int c;
			try {
				c = std::stoi(value.substr(1), nullptr, 16);
			}
			catch(...) {
				return false;
			}

			if (c < 0x000000 || c > 0xFFFFFF)
				return false;

			color = c;
			return true;
		}

		std::string hex_color() {
			std::stringstream stream;
			stream << "#" << std::uppercase << std::setfill('0') << std::setw(6) << std::hex << color;
			return stream.str();
		}
	};

	class types_t : public std::vector<type_t> {
	public:
		types_t() : std::vector<type_t>{
			{ "Unknown", 0xd94545 },
			{ "Timesignal", 0xcccccc },
			{ "Morse", 0xb12eb3 },
			{ "Digimode", 0xcfaa32 },
			{ "Voice", 0x247fc9 },
			{ "Broadcast", 0x2cb838 },
			{ "Noise", 0x777777 },
			{ "Carrier", 0x777777 },
			{ "Bandmarker", 0xf0d52b }}
		{
		}		
	};

}

namespace marker {

	enum action {
		DELETEMARKER,
		INSERTMARKER, 
		UPDATEMARKER
	};

	struct marker_t {
		int lid;
		int cid;
		int oid;
		long long frequency;
		std::string name;
		int type;
		std::string modulation;
		std::string country;
		std::string location;
		std::string comment;
		int score;
		int vote;
		bool show;
		bool owner;
		bool readOnly;
		bool deleteable;
		bool synced;
		std::string flags;
		std::string flagsl;
		int tune_modulation;
		int tune_bandwidth;
		bool tune_centered;
	};

	inline bool operator<(const marker_t& lhs, const marker_t& rhs)
	{
		return lhs.frequency < rhs.frequency || (lhs.frequency == rhs.frequency && lhs.lid < rhs.lid);
	}

	inline std::string format_frequency(long long frequency) {
		std::string value = std::to_string(frequency);
		int len = value.size();

		for(int pos = 9; pos >= 3; pos -= 3)
			if (len > pos)
				value.insert(value.end() - pos, '.');

		return value;
	}


}

namespace sync {
	struct syncinfo_t {
		std::string commit;
		std::string count;
		std::string time;
	};

	struct syncitem_t {
		std::string action;
		marker::marker_t marker;
	};

	struct syncresult_t {
		int commit;
		std::vector<syncitem_t> items;
	};

	inline const std::string currentDateTime() {
		time_t     now = time(0);
		struct tm  tstruct;
		char       buf[80];
		tstruct = *localtime(&now);

		strftime(buf, sizeof(buf), "%y-%m-%d %X", &tstruct);

		return std::string(buf);
	}
}

namespace util {
	inline std::string& uppercase(std::string& value) {
		for (auto& c : value) c = toupper(c);
		return value;
	}
}
