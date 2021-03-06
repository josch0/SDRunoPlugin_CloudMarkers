#include <nana/gui.hpp>
#include <nana/gui/msgbox.hpp>

#include "EditWindow.h"

EditWindow::EditWindow(nana::form* parent, DataService& dataService, marker::marker_t marker) :
	sdrwindow(parent, marker.lid > 0 ? "EDIT" : "ADD", 567, 440, dataService),
	m_marker(marker)
{
}

void EditWindow::createWidgets()
{
	auto types = m_dataService.GetTypeSettings();

	using namespace nana;

	const int contentWidth = m_width - 40;
	const int half = contentWidth / 2 - 6;
	const int full = contentWidth;
	const int posleft = 20;
	const int posright = m_width / 2 + 6;

	m_lblFreq = makeLabelLarge(posleft, 40, contentWidth, 20);
	m_lblFreq->text_align(align::right);
	m_lblFreq->caption("Frequency: " + marker::format_frequency(m_marker.frequency) + " Hz");

	// ### LEFT

	// Name
	m_lblNameC = makeLabelTitle(posleft, 65, half, "NAME");
	m_txtName = makeTextbox(posleft, 77, half, 24, false, "Enter name ...");
	m_txtName->caption(m_marker.name);
	m_txtName->set_accept([&](wchar_t key) {
		return m_txtName->caption().size() < 25 || key == 0x7F || key == 0x08;
	});

	// Type
	m_lblTypeC = makeLabelTitle(posleft, 111, half, "TYPE");
	m_cmbType = makeCombox(posleft, 123, half, 24);
	for (auto type : types) {
		m_cmbType->push_back(type.name);
	}
	m_cmbType->option(m_marker.type);

	// Modulation
	m_lblModulationC = makeLabelTitle(posleft, 157, half, "MODULATION");
	m_txtModulation = makeTextbox(posleft, 169, half, 24, false, "Modulation/protocol (optional) ...");
	m_txtModulation->caption(m_marker.modulation);
	m_txtModulation->set_accept([&](wchar_t key) {
		return m_txtModulation->caption().size() < 25 || key == 0x7F || key == 0x08;
	});

	// Country
	m_lblCountryC = makeLabelTitle(posleft, 203, half, "COUNTRY");
	m_txtCountry = makeTextbox(posleft, 215, half, 24, false, "Transmitter ITU code (optional) ...");
	m_txtCountry->caption(m_marker.country);
	m_txtCountry->set_accept([&](wchar_t key) {
		return m_txtCountry->caption().size() < 3 || key == 0x7F || key == 0x08;
	});

	// Location
	m_lblLocationC = makeLabelTitle(posleft, 249, half, "LOCATION");
	m_txtLocation = makeTextbox(posleft, 261, half, 24, false, "Transmitter location (optional) ...");
	m_txtLocation->caption(m_marker.location);
	m_txtLocation->set_accept([&](wchar_t key) {
		return m_txtLocation->caption().size() < 25 || key == 0x7F || key == 0x08;
	});

	// ### RIGHT

	// Description
	m_lblCommentC = makeLabelTitle(posright, 65, half, "DESCRIPTION");
	m_txtComment = makeTextbox(posright, 77, half, 208, true, "Enter description ...");
	m_txtComment->caption(m_marker.comment);
	m_txtComment->set_accept([&](wchar_t key) {
		return m_txtComment->caption().size() < 1000 || key == 0x7F || key == 0x08;
	});

	// ### FULL WIDTH

	// Cloud sync
	m_lblSharedC = makeLabelTitle(posleft, 300, half, "CLOUD SYNC");
	m_chkShared = makeRadio(posleft, 320, full, 30, "Shared marker\r\nAre synced with the central server and can be used by other users of the plugin.");
	m_chkLocal = makeRadio(posleft, 355, full, 30, "Local marker\r\nAre stored in a local database and are only visible for you.");

	m_grpSync = makeRadioGroup();
	m_grpSync->add(*m_chkShared);
	m_grpSync->add(*m_chkLocal);

	m_chkShared->check(m_marker.synced);
	m_chkLocal->check(!m_marker.synced);


	// ### BUTTONS
	m_btnSave = makeButton(20, 400, "SAVE", "Save Marker");
	m_btnCancel = makeButton(84, 400, "CANCEL", "Cancel editing");
	m_btnDel = makeButton(contentWidth + 20 - 54, 400, "DEL", "Delete marker");
	m_btnDel->enabled(m_marker.deleteable);

	// Events
	m_btnSave->events().click([&] {
		if (m_txtName->caption().empty()) {
			msgbox msgbox(*m_form, "Save marker");
			msgbox << "You must enter a name for this marker!";
			msgbox.icon(msgbox::icon_error);
			msgbox();
			return;
		}

		if (m_marker.lid > 0)
		{
			if (m_marker.synced && m_chkShared->checked())
			{
				msgbox msgbox(*m_form, "Update shared marker", msgbox::yes_no);
				msgbox << "Your changes on this shared marker will be commited to the remote server.\r\nAll other users can see your changes and can also edit them.\r\nOk?";
				msgbox.icon(msgbox::icon_information);
				if (msgbox() == msgbox::pick_no)
					return;
			}
			if (m_marker.synced && m_chkLocal->checked())
			{
				msgbox msgbox(*m_form, "Make local copy" , msgbox::yes_no);
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

		m_dataService.SaveMarker(m_marker, m_chkShared->checked());

		Close();
	});

	m_btnCancel->events().click([&] {
		Close();
	});

	m_btnDel->events().click([&] {
		msgbox msgDel(*m_form, "Delete marker", msgbox::yes_no);
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
	});

}
