#include <nana/gui.hpp>
#include <nana/gui/msgbox.hpp>

#include "SettingsWindow.h"

SettingsWindow::SettingsWindow(nana::form* parent, DataService& dataService) :
	sdrwindow(parent, "SETTINGS", 567, 440, dataService)
{
}

void SettingsWindow::createWidgets()
{
	using namespace nana;

	const int contentWidth = m_width - 40;

	const int qw = (contentWidth - 36) / 4;
	const int q1 = 20;
	const int q2 = q1 + qw + 12;
	const int q3 = q2 + qw + 12;
	const int q4 = q3 + qw + 12;

	const int r1 = 40;
	const int r1t = r1 + 12;
	const int r2 = r1 + 45;
	const int r2t = r2 + 12;
	const int r3 = r2 + 45;
	const int r3t = r3 + 12;

	// ### ROW 1		
	m_lblColor0C = makeLabelTitle(q1, r1, qw, "COLOR UNKNOWN");
	m_txtColor0 = makeTextbox(q1, r1t, qw, 20, false, "");
	m_lblColor1C = makeLabelTitle(q2, r1, qw, "COLOR TIMESIGNAL");
	m_txtColor1 = makeTextbox(q2, r1t, qw, 20, false, "");
	m_lblColor2C = makeLabelTitle(q3, r1, qw, "COLOR MORSE");
	m_txtColor2 = makeTextbox(q3, r1t, qw, 20, false, "");
	m_lblColor3C = makeLabelTitle(q4, r1, qw, "COLOR DIGIMODE");
	m_txtColor3 = makeTextbox(q4, r1t, qw, 20, false, "");

	// ### ROW 2		
	m_lblColor4C = makeLabelTitle(q1, r2, qw, "COLOR VOICE");
	m_txtColor4 = makeTextbox(q1, r2t, qw, 20, false, "");
	m_lblColor5C = makeLabelTitle(q2, r2, qw, "COLOR BROADCAST");
	m_txtColor5 = makeTextbox(q2, r2t, qw, 20, false, "");
	m_lblColor6C = makeLabelTitle(q3, r2, qw, "COLOR NOISE / CARRIER");
	m_txtColor6 = makeTextbox(q3, r2t, qw, 20, false, "");
	m_lblColor8C = makeLabelTitle(q4, r2, qw, "COLOR BANDMARKER");
	m_txtColor8 = makeTextbox(q4, r2t, qw, 20, false, "");

	// ### ROW 2
	m_lblOffset = makeLabelTitle(q1, r3, qw, "MARKER VFO OFFSET (kHz)");
	m_spnOffset = makeSpinbox(q1, r3t, qw, 20);
	m_spnOffset->range(10, 500, 10);

	m_txtColor0->set_accept([&](wchar_t key) {	return validColorKey(m_txtColor0->caption().size(), key);});
	m_txtColor1->set_accept([&](wchar_t key) {	return validColorKey(m_txtColor1->caption().size(), key);});
	m_txtColor2->set_accept([&](wchar_t key) {	return validColorKey(m_txtColor2->caption().size(), key);});
	m_txtColor3->set_accept([&](wchar_t key) {	return validColorKey(m_txtColor3->caption().size(), key);});
	m_txtColor4->set_accept([&](wchar_t key) {	return validColorKey(m_txtColor4->caption().size(), key);});
	m_txtColor5->set_accept([&](wchar_t key) {	return validColorKey(m_txtColor5->caption().size(), key);});
	m_txtColor6->set_accept([&](wchar_t key) {	return validColorKey(m_txtColor6->caption().size(), key);});
	m_txtColor8->set_accept([&](wchar_t key) {	return validColorKey(m_txtColor8->caption().size(), key);});

	// Set current values
	m_types = m_dataService.GetTypeSettings();

	m_txtColor0->caption(m_types[0].hex_color());
	m_txtColor1->caption(m_types[1].hex_color());
	m_txtColor2->caption(m_types[2].hex_color());
	m_txtColor3->caption(m_types[3].hex_color());
	m_txtColor4->caption(m_types[4].hex_color());
	m_txtColor5->caption(m_types[5].hex_color());
	m_txtColor6->caption(m_types[6].hex_color());
	m_txtColor8->caption(m_types[8].hex_color());

	m_spnOffset->value(std::to_string(m_dataService.GetVfoOffset()));

	// ### BUTTONS
	m_btnSave = makeButton(20, 400, "SAVE", "Save Marker");
	m_btnCancel = makeButton(84, 400, "CANCEL", "Cancel editing");

	m_btnSave->events().click([&] {
		if (!m_types[0].hex_color(m_txtColor0->caption())) {
			colorError(m_types[0].name);
			return;
		}
		if (!m_types[1].hex_color(m_txtColor1->caption())) {
			colorError(m_types[1].name);
			return;
		}
		if (!m_types[2].hex_color(m_txtColor2->caption())) {
			colorError(m_types[2].name);
			return;
		}
		if (!m_types[3].hex_color(m_txtColor3->caption())) {
			colorError(m_types[3].name);
			return;
		}
		if (!m_types[4].hex_color(m_txtColor4->caption())) {
			colorError(m_types[4].name);
			return;
		}
		if (!m_types[5].hex_color(m_txtColor5->caption())) {
			colorError(m_types[5].name);
			return;
		}
		if (!m_types[6].hex_color(m_txtColor6->caption())) {
			colorError(m_types[6].name);
			return;
		}
		if (!m_types[8].hex_color(m_txtColor8->caption())) {
			colorError(m_types[7].name);
			return;
		}
		m_dataService.SetTypeSettings(m_types);

		m_dataService.SetVfoOffset(std::stoi(m_spnOffset->value()));

		Close();
	});

	m_btnCancel->events().click([&] {
		Close();
	});
}

void SettingsWindow::colorError(std::string name)
{
	nana::msgbox msgbox(*m_form, "Save settings");
	msgbox << "The color code for \"" << name << "\" is incorrect!";
	msgbox.icon(nana::msgbox::icon_error);
	msgbox();
}

bool SettingsWindow::validColorKey(int size, wchar_t key)
{
	if (key == 0x7F || key == 0x08)
		return true;

	if (size < 7 && ((key >= '0' && key <= '9') || (key >= 'a' && key <= 'f') || (key >= 'A' && key <= 'F')))
		return true;

	return false;
}