#pragma once

#include <nana/gui.hpp>

#ifdef CLOUDMARKER_RELEASE
	#include "remote_prod.h"
#else
	#include "remote_dev.h"
#endif // CLOUDMARKER_PROD


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

	// 0=Unknown, 1=Time Signal, 2=Morse, 3=DigiMode, 4=Voice, 5=Broadcast, 6=SingleCarrier, 7=Noise
	const int type_colors[] = { 0xd94545, 0xcccccc, 0xb12eb3 , 0xcfaa32, 0x247fc9, 0x2cb838, 0x777777, 0x777777, 0xf0d52b };

	const char* const type_names[] = { "Unknown", "Timesignal", "Morse", "Digimode", "Voice", "Broadcast", "Carrier", "Noise", "Bandmarker" };

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

		strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

		return std::string(buf);
	}
}