#include <nana/gui.hpp>
#include <nana/gui/msgbox.hpp>

#include "EditWindow.h"
#include "CountryWindow.h"

#define WIDTH (567)
#define HEIGHT (470)

#define TEXTHEIGHT (24)
#define FULLWIDTH (WIDTH - 40)
#define HALFWIDTH (FULLWIDTH / 2 - 6)
#define QUARTERWIDTH (HALFWIDTH / 2 - 6)
#define LEFT (20)
#define LEFT2 (20 + QUARTERWIDTH + 12)
#define RIGHT (WIDTH / 2 + 6)
#define ROW(x) (40 + ((x - 1) * 45))
#define TEXTROW(x) (52 + ((x - 1) * 45))

EditWindow::EditWindow(nana::form* parent, DataService& dataService, marker::marker_t marker) :
	sdrwindow(parent, marker.lid > 0 ? "EDIT" : "ADD", WIDTH, HEIGHT, dataService),
	m_marker(marker)
{
}

void EditWindow::createWidgets()
{
	auto types = m_dataService.GetTypeSettings();

	using namespace nana;

	// ### LEFT

	// Frequenz fine tuning
	m_lblFreqC = makeLabelTitle(LEFT, ROW(1), HALFWIDTH, "FREQUENCY FINE TUNING");
	m_txtFreq = makeTextbox(LEFT, TEXTROW(1), HALFWIDTH, TEXTHEIGHT, false, "");
	m_txtFreq->editable(false);
	m_txtFreq->caption(marker::format_frequency(m_marker.frequency) + " Hz");

	m_btnFreqP10  = makeButton(RIGHT,       ROW(1),      40, "+10.", "Add 10 kHz");
	m_btnFreqM10  = makeButton(RIGHT,       ROW(1) + 20, 40, "-10.", "Sub 10 kHz");
	m_btnFreqP1   = makeButton(RIGHT +  44, ROW(1),      40, "+1.", "Add 1 kHz");
	m_btnFreqM1   = makeButton(RIGHT +  44, ROW(1) + 20, 40, "-1.", "Sub 1 kHz");
	m_btnFreqP01  = makeButton(RIGHT +  88, ROW(1),      40, "+.1", "Add 100 Hz");
	m_btnFreqM01  = makeButton(RIGHT +  88, ROW(1) + 20, 40, "-.1", "Sub 100 Hz");
	m_btnFreqP001 = makeButton(RIGHT + 132, ROW(1),      40, "+.01", "Add 10 Hz");
	m_btnFreqM001 = makeButton(RIGHT + 132, ROW(1) + 20, 40, "-.01", "Sub 10 Hz");

	m_btnFreqR0 =    makeButton(FULLWIDTH - 21, ROW(1),      40, ".000", "Set the frequency to .000");
	m_btnFreqR5 =    makeButton(FULLWIDTH - 21, ROW(1) + 20, 40, ".500", "Set the frequency to .500");
	
	// Name
	m_lblNameC = makeLabelTitle(LEFT, ROW(2), HALFWIDTH, "NAME");
	m_txtName = makeTextbox(LEFT, TEXTROW(2), HALFWIDTH, TEXTHEIGHT, false, "Enter name ...");
	m_txtName->caption(m_marker.name);
	m_txtName->set_accept([&](wchar_t key) {
		return m_txtName->caption().size() < 25 || key == 0x7F || key == 0x08;
	});

	// Type
	m_lblTypeC = makeLabelTitle(LEFT, ROW(3), HALFWIDTH, "TYPE");
	m_cmbType = makeCombox(LEFT, TEXTROW(3), HALFWIDTH, TEXTHEIGHT);
	for (auto type : types) {
		m_cmbType->push_back(type.name);
	}
	m_cmbType->option(m_marker.type);

	API::tabstop(m_cmbType->handle());


	// Modulation
	m_lblModulationC = makeLabelTitle(LEFT, ROW(4), HALFWIDTH, "MODE / PROTOCOL (optional)");
	m_txtModulation = makeTextbox(LEFT, TEXTROW(4), HALFWIDTH, TEXTHEIGHT, false, "Mode/protocol ...");
	m_txtModulation->caption(m_marker.modulation);
	m_txtModulation->set_accept([&](wchar_t key) {
		return m_txtModulation->caption().size() < 25 || key == 0x7F || key == 0x08;
	});

	// Country
	m_lblCountryC = makeLabelTitle(LEFT, ROW(5), HALFWIDTH, "COUNTRY (optional)");
	m_txtCountry = makeTextbox(LEFT, TEXTROW(5), HALFWIDTH - 60, TEXTHEIGHT, false, "ITU code ...");
	m_txtCountry->caption(m_marker.country);
	m_txtCountry->set_accept([&](wchar_t key) {
		return m_txtCountry->caption().size() < 3 || key == 0x7F || key == 0x08;
	});

	m_btnSearch = makeButton(HALFWIDTH - 34, TEXTROW(5) + 4, "SEARCH", "Search for ITU Code");

	// Location
	m_lblLocationC = makeLabelTitle(LEFT, ROW(6), HALFWIDTH, "LOCATION (optional)");
	m_txtLocation = makeTextbox(LEFT, TEXTROW(6), HALFWIDTH, TEXTHEIGHT, false, "Location ...");
	m_txtLocation->caption(m_marker.location);
	m_txtLocation->set_accept([&](wchar_t key) {
		return m_txtLocation->caption().size() < 25 || key == 0x7F || key == 0x08;
	});

	// ### RIGHT

	// Description
	m_lblCommentC = makeLabelTitle(RIGHT, ROW(2), HALFWIDTH, "DESCRIPTION (recommended)");
	m_txtComment = makeTextbox(RIGHT, TEXTROW(2), HALFWIDTH, 204, true, "Description ...");
	m_txtComment->caption(m_marker.comment);
	m_txtComment->set_accept([&](wchar_t key) {
		return m_txtComment->caption().size() < 1000 || key == 0x7F || key == 0x08;
	});

	// ### FULL WIDTH

	// Tuning params
	m_lblTuneC = makeLabelTitle(LEFT, ROW(7), FULLWIDTH, "TUNING PARAMETERS (optional)");
	m_cmbTuneMod = makeCombox(LEFT, TEXTROW(7), QUARTERWIDTH, TEXTHEIGHT);
	m_cmbTuneMod->push_back("None");
	m_cmbTuneMod->push_back("AM");
	m_cmbTuneMod->push_back("SAM / DSB");
	m_cmbTuneMod->push_back("SAM / LSB");
	m_cmbTuneMod->push_back("SAM / USB");
	m_cmbTuneMod->push_back("NFM");
	m_cmbTuneMod->push_back("MFM");
	m_cmbTuneMod->push_back("WFM");
	m_cmbTuneMod->push_back("SWFM");
	m_cmbTuneMod->push_back("DSB");
	m_cmbTuneMod->push_back("LSB");
	m_cmbTuneMod->push_back("USB");
	m_cmbTuneMod->push_back("CW");
	m_cmbTuneMod->push_back("Digital");
	m_cmbTuneMod->option(0);

	m_cmbTuneBw = makeCombox(LEFT2, TEXTROW(7), QUARTERWIDTH, TEXTHEIGHT);
	m_cmbTuneBw->enabled(false);
	m_cmbTuneBw->push_back("None");
	m_cmbTuneBw->option(0);

	m_chkTuneCenter = makeCheckbox(RIGHT, TEXTROW(7) + 5, HALFWIDTH, 20, "Center marker within bandwidth");
	m_chkTuneCenter->enabled(false);
	m_chkTuneCenter->check(false);


	// Cloud sync
	m_lblSharedC = makeLabelTitle(LEFT, ROW(8), HALFWIDTH, "CLOUD SYNC");
	m_chkShared = makeRadio(LEFT, TEXTROW(8) + 3, FULLWIDTH, 20, "Shared marker - are synced with the central server and can be used by other users of the plugin.");
	m_chkLocal = makeRadio(LEFT, TEXTROW(8) + 23, FULLWIDTH, 20, "Local marker - are stored in a local database and are only visible for you.");

	m_grpSync = makeRadioGroup();
	m_grpSync->add(*m_chkShared);
	m_grpSync->add(*m_chkLocal);

	m_chkShared->check(m_marker.synced);
	m_chkLocal->check(!m_marker.synced);


	// ### BUTTONS
	m_btnSave = makeButton(20, 430, "SAVE", "Save Marker");
	m_btnCancel = makeButton(84, 430, "CANCEL", "Cancel editing");
	m_btnSpot = makeButton(RIGHT, 430, "SPOTTED", "Add comment");
	m_btnDel = makeButton(FULLWIDTH + 20 - 54, 430, "DEL", "Delete marker");
	
	// Set values
	updateTuning(m_marker.tune_modulation, true);

	// Events
	m_cmbTuneMod->events().selected([&] {
		updateTuning(m_cmbTuneMod->option(), false);
	});


	m_btnSpot->events().click([&] {
		auto qth = m_dataService.GetQTH();
		if (qth.empty()) {
			nana::msgbox msgbox(*m_form, "Spot station");
			msgbox << "You must enter your QTH in the Settings window!";
			msgbox.icon(msgbox::icon_information);
			msgbox();
			return;
		}

		inputbox inbox(*m_form, "<bold>Please do not spot every station!</> Only add a spot comment, if something noteworthy has happend.", "Station spotted");
		inputbox::text note("What is noteworthy", "");

		inbox.verify([&note](window handle)
			{
				if (note.value().empty())
				{
					nana::msgbox mb(handle, "Invalid input");
					mb << "You must provide a comment!";
					mb.show();
					return false;
				}
				return true;
			});

		if (inbox.show_modal(note)) {		
			if (!m_marker.comment.empty() && m_marker.comment.back() != '\r' && m_marker.comment.back() != '\n')
				m_marker.comment += "\r\n";

			m_marker.comment += spotted(qth, note.value());
			m_txtComment->caption(m_marker.comment);
		}

	});

	m_btnFreqP1->events().click([&] {
		m_marker.frequency += 1000;
		m_txtFreq->caption(marker::format_frequency(m_marker.frequency) + " Hz");
		});
	m_btnFreqM1->events().click([&] {
		m_marker.frequency -= 1000;
		m_txtFreq->caption(marker::format_frequency(m_marker.frequency) + " Hz");
		});
	m_btnFreqP01->events().click([&] {
		m_marker.frequency += 100;
		m_txtFreq->caption(marker::format_frequency(m_marker.frequency) + " Hz");
		});
	m_btnFreqM01->events().click([&] {
		m_marker.frequency -= 100;
		m_txtFreq->caption(marker::format_frequency(m_marker.frequency) + " Hz");
		});
	m_btnFreqP001->events().click([&] {
		m_marker.frequency += 10;
		m_txtFreq->caption(marker::format_frequency(m_marker.frequency) + " Hz");
		});
	m_btnFreqM001->events().click([&] {
		m_marker.frequency -= 10;
		m_txtFreq->caption(marker::format_frequency(m_marker.frequency) + " Hz");
		});
	m_btnFreqP10->events().click([&] {
		m_marker.frequency += 10000;
		m_txtFreq->caption(marker::format_frequency(m_marker.frequency) + " Hz");
		});
	m_btnFreqM10->events().click([&] {
		m_marker.frequency -= 10000;
		m_txtFreq->caption(marker::format_frequency(m_marker.frequency) + " Hz");
		});

	m_btnFreqR0->events().click([&] {
		m_marker.frequency = m_marker.frequency / 1000 * 1000;
		m_txtFreq->caption(marker::format_frequency(m_marker.frequency) + " Hz");
		});
	m_btnFreqR5->events().click([&] {
		m_marker.frequency = m_marker.frequency / 1000 * 1000 + 500;
		m_txtFreq->caption(marker::format_frequency(m_marker.frequency) + " Hz");
		});

	m_btnSearch->events().click([&] {
		CountryWindow countryWindow{ m_form, m_dataService, m_txtCountry->caption() };
		countryWindow.Show(nana::point(m_width / 2 - 200, m_height / 2 - 150));

		std::string ituCode = countryWindow.GetITUCode();
		if (!ituCode.empty())
			m_txtCountry->caption(ituCode);
	});

	m_btnSave->events().click([&] {
		if (m_txtName->caption().empty()) {
			nana::msgbox msgbox(*m_form, "Save marker");
			msgbox << "You must enter a name for this marker!";
			msgbox.icon(msgbox::icon_error);
			msgbox();
			return;
		}

		if (m_marker.lid > 0)
		{
			if (m_marker.synced && m_chkShared->checked())
			{
				nana::msgbox msgbox(*m_form, "Update shared marker", msgbox::yes_no);
				msgbox << "Your changes on this shared marker will be commited to the remote server.\r\nAll other users can see your changes and can also edit them.\r\nOk?";
				msgbox.icon(msgbox::icon_information);
				if (msgbox() == msgbox::pick_no)
					return;
			}
			if (m_marker.synced && m_chkLocal->checked())
			{
				nana::msgbox msgbox(*m_form, "Make local copy" , msgbox::yes_no);
				msgbox << "A local copy of this shared marker will be stored in your local database.\r\nThe shared marker will not change and still exist on the remote server.\r\nOk?";
				msgbox.icon(msgbox::icon_information);
				if (msgbox() == msgbox::pick_no)
					return;
			}
		}

		m_marker.name = m_txtName->caption();
		m_marker.type = m_cmbType->option();
		m_marker.modulation = m_txtModulation->caption();
		m_marker.country = m_txtCountry->caption();
		m_marker.location = m_txtLocation->caption();
		m_marker.comment = m_txtComment->caption();
		m_marker.tune_modulation = m_cmbTuneMod->option();
		m_marker.tune_centered = m_chkTuneCenter->checked();
		m_marker.tune_bandwidth = 0;

		
		if (m_cmbTuneMod->option() != 0)
		{
			std::string bandwidth = m_cmbTuneBw->text(m_cmbTuneBw->option());
			if (!bandwidth.empty() && bandwidth != "None") {
				size_t pos = bandwidth.find(" ");
				m_marker.tune_bandwidth = std::stoi(bandwidth.substr(0, pos));
				if (bandwidth[pos + 1] == 'k') // kHz
					m_marker.tune_bandwidth *= 1000;
			}
		}

		m_dataService.SaveMarker(m_marker, m_chkShared->checked());

		Close();
	});

	m_btnCancel->events().click([&] {
		Close();
	});

	m_btnDel->events().click([&] {
		if (m_marker.deleteable) {
			nana::msgbox msgDel(*m_form, "Delete marker", msgbox::yes_no);
			msgDel.icon(msgbox::icon_question);
			msgDel << "Do you realy want to delete this marker?" << std::endl;
			if (m_marker.synced)
				msgDel << "(This marker is synced and will be deleted locally and in the cloud.)";
			else
				msgDel << "(This marker is not synced and will be deleted only locally.)";

			if (msgDel() == msgbox::pick_no)
				return;

			m_dataService.DeleteMarker(m_marker);		
			Close();
		}
		else
		{
			inputbox inbox(*m_form, "You can not delete this marker because you are not the owner. But you can request the deletion.", "Deletion requested");
			inputbox::text reason("Reason (e.g. Duplicate)", "");

			inbox.verify([&reason](window handle)
			{
				if (reason.value().empty())
				{
					nana::msgbox mb(handle, "Invalid input");
					mb << "You must provide a reason why this marker should be deleted!";
					mb.show();
					return false;
				}
				return true;
			});

			if (inbox.show_modal(reason)) {
				m_marker.comment = "## DELETION REQUESTED\r\nReason: " + reason.value() + "\r\n\r\n" + m_marker.comment;
				m_txtComment->caption(m_marker.comment);
			}
		}
	});

}

std::string EditWindow::spotted(std::string qth, std::string note)
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *gmtime(&now);

	strftime(buf, sizeof(buf), "%y%m%d/%H%M", &tstruct);

	std::stringstream stream;
	stream << "## SPOTTED " << std::uppercase << buf << "@" << qth << "\r\n" << note;
	return stream.str();	
}


void EditWindow::updateTuning(int modulation, bool actual)
{
	m_cmbTuneBw->clear();
	m_cmbTuneBw->enabled(true);
	m_chkTuneCenter->enabled(false);
	switch (modulation) {
	case 0: // None		
		m_cmbTuneBw->enabled(false);
		m_cmbTuneBw->push_back("None");
		m_chkTuneCenter->check(false);
		break;
	case 1: // AM
	case 2: // SAM / DSB
		m_cmbTuneBw->push_back("6000 Hz");
		m_cmbTuneBw->push_back("8000 Hz");
		m_cmbTuneBw->push_back("11 kHz");
		m_cmbTuneBw->push_back("20 kHz");
		m_chkTuneCenter->check(true);
		break;
	case 3: // SAM / LSB
	case 4: // SAM / USB
		m_cmbTuneBw->push_back("2600 Hz");
		m_cmbTuneBw->push_back("3000 Hz");
		m_cmbTuneBw->push_back("4000 Hz");
		m_cmbTuneBw->push_back("5500 Hz");
		m_chkTuneCenter->check(true);
		break;
	case 5: // NFM
		m_cmbTuneBw->push_back("10 kHz");
		m_cmbTuneBw->push_back("12 kHz");
		m_cmbTuneBw->push_back("15 kHz");
		m_cmbTuneBw->push_back("20 kHz");
		m_chkTuneCenter->check(true);
		break;
	case 6: // MFM
		m_cmbTuneBw->push_back("30 kHz");
		m_cmbTuneBw->push_back("36 kHz");
		m_cmbTuneBw->push_back("38 kHz");
		m_cmbTuneBw->push_back("48 kHz");
		m_chkTuneCenter->check(true);
		break;
	case 7: // WFM
	case 8: // SWFM
		m_cmbTuneBw->push_back("60 kHz");
		m_cmbTuneBw->push_back("80 kHz");
		m_cmbTuneBw->push_back("120 kHz");
		m_cmbTuneBw->push_back("192 kHz");
		m_chkTuneCenter->check(true);
		break;
	case 9: // DSB
		m_cmbTuneBw->push_back("4000 Hz");
		m_cmbTuneBw->push_back("4400 Hz");
		m_cmbTuneBw->push_back("4800 Hz");
		m_cmbTuneBw->push_back("5400 Hz");
		m_chkTuneCenter->check(true);
		break;
	case 10: // LSB
	case 11: // USB
		m_cmbTuneBw->push_back("1800 Hz");
		m_cmbTuneBw->push_back("2200 Hz");
		m_cmbTuneBw->push_back("2800 Hz");
		m_cmbTuneBw->push_back("3000 Hz");
		m_chkTuneCenter->enabled(true);
		m_chkTuneCenter->check(false);
		break;
	case 12: // CW
		m_cmbTuneBw->push_back("150 Hz");
		m_cmbTuneBw->push_back("250 Hz");
		m_cmbTuneBw->push_back("500 Hz");
		m_cmbTuneBw->push_back("750 Hz");
		m_chkTuneCenter->check(true);
		break;
	case 13: // Digital
		m_cmbTuneBw->push_back("4000 Hz");
		m_cmbTuneBw->push_back("6000 Hz");
		m_cmbTuneBw->push_back("12 kHz");
		m_cmbTuneBw->push_back("24 kHz");
		m_chkTuneCenter->enabled(true);
		m_chkTuneCenter->check(true);
		break;
	}
	m_cmbTuneBw->option(0);

	if (actual) {
		m_cmbTuneMod->option(modulation);
		m_chkTuneCenter->check(m_marker.tune_centered);

		std::string bw = (m_marker.tune_bandwidth >= 10000) ? std::to_string(m_marker.tune_bandwidth / 1000) + " kHz" : std::to_string(m_marker.tune_bandwidth) + " Hz";
		for (int i = 0; i < m_cmbTuneBw->the_number_of_options(); i++) {
			if (m_cmbTuneBw->text(i) == bw) {
				m_cmbTuneBw->option(i);
				break;
			}
		}
	}
}