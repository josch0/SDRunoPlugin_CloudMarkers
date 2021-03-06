#pragma once

#include <nana/gui.hpp>
#include <iomanip>

#ifdef CLOUDMARKER_RELEASE
	#include "remote_prod.h"
#else
	#include "remote_dev.h"
#endif // CLOUDMARKER_PROD

namespace version {
	const std::string const version = "0.1.0";
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
	};

	inline bool operator<(const marker_t& lhs, const marker_t& rhs)
	{
		return lhs.lid < rhs.lid;
	}

	struct dotted : std::numpunct<char> {
		char do_thousands_sep()   const { return '.'; }  // separate with dots
		std::string do_grouping() const { return "\3"; } // groups of 3 digits
		static void imbue(std::ostream& os) {
			os.imbue(std::locale(os.getloc(), new dotted));
		}
	};
	
	inline std::string format_frequency(long long frequency) {
		std::stringstream ss;
		dotted::imbue(ss);
		ss << frequency;
		return ss.str();
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