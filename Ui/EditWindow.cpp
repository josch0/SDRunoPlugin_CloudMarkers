#include <iomanip>

#include <nana/gui/widgets/picture.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/checkbox.hpp>
#include <nana/gui/widgets/combox.hpp>
#include <nana/gui/dragger.hpp>
#include <nana/gui/msgbox.hpp>

#include "sdrbutton.h"
#include "../common.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include "../resource.h"
#include "EditWindow.h"

// Shouldn't need to change these
#define topBarHeight (27)
#define bottomBarHeight (8)
#define sideBorderWidth (8)

// TODO: Change these numbers to the height and width of your form
#define margin (20)
#define formWidth (567)
#define formHeight (440)
#define contentWidth (formWidth - 2 * margin)
#define half (contentWidth / 2 - 6)
#define full (contentWidth)
#define posleft (margin)
#define posright (formWidth / 2 + 6)

void imageFromResource2(HMODULE* hModule, HDC* hdc, int resourceId, nana::paint::image* img_border) {
	HRSRC rc_border = NULL;
	HBITMAP bm_border = NULL;
	BITMAPINFO bmInfo_border = { 0 };
	BITMAPFILEHEADER borderHeader = { 0 };
	BYTE* borderPixels = NULL;

	const unsigned int rawDataOffset = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);
	rc_border = FindResource(*hModule, MAKEINTRESOURCE(resourceId), RT_BITMAP);
	bm_border = (HBITMAP)LoadImage(*hModule, MAKEINTRESOURCE(resourceId), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bmInfo_border.bmiHeader.biSize = sizeof(bmInfo_border.bmiHeader);
	GetDIBits(*hdc, bm_border, 0, 0, NULL, &bmInfo_border, DIB_RGB_COLORS);
	bmInfo_border.bmiHeader.biCompression = BI_RGB;
	borderHeader.bfOffBits = rawDataOffset;
	borderHeader.bfSize = bmInfo_border.bmiHeader.biSizeImage;
	borderHeader.bfType = 0x4D42;
	borderPixels = new BYTE[bmInfo_border.bmiHeader.biSizeImage + rawDataOffset];
	*(BITMAPFILEHEADER*)borderPixels = borderHeader;
	*(BITMAPINFO*)(borderPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_border;
	GetDIBits(*hdc, bm_border, 0, bmInfo_border.bmiHeader.biHeight, (LPVOID)(borderPixels + rawDataOffset), &bmInfo_border, DIB_RGB_COLORS);
	img_border->open(borderPixels, bmInfo_border.bmiHeader.biSizeImage);
	delete[] borderPixels;
}


EditWindow::EditWindow(nana::form* parent, DataService& dataService) :
	EditWindow(parent, dataService, { 0 })
{
}

EditWindow::EditWindow(nana::form* parent, DataService& dataService, marker::marker_t marker) :
	m_dataService(dataService),
	m_marker(marker),
	m_parentForm(parent)
{
	m_form = nullptr;
	m_btnDel = nullptr;
	m_btnSave = nullptr;
	m_btnCancel = nullptr;

	createForm();
}

EditWindow::~EditWindow()
{
	m_form->close();
}

void EditWindow::createForm()
{
	HMODULE hModule = GetModuleHandle(L"SDRunoPlugin_CloudMarkers");
	HDC hdc = GetDC(NULL);

	nana::paint::image bg1Image{};
	nana::paint::image bg2Image{};
	nana::paint::image headImage{};

	imageFromResource2(&hModule, &hdc, IDB_BITMAP_BG1, &bg1Image);
	imageFromResource2(&hModule, &hdc, IDB_BITMAP_BG2, &bg2Image);
	imageFromResource2(&hModule, &hdc, IDB_BITMAP_HEAD, &headImage);

	ReleaseDC(NULL, hdc);

	using namespace nana;

	paint::font fontSmall{ "Verdana", 8 };
	paint::font fontNormal{ "Verdana", 10 };
	paint::font fontLarge{ "Verdana", 12 };
	paint::font fontTitle{ "Verdana", 6 , paint::font::font_style(700) };

	m_form = new form(*m_parentForm, size(formWidth, formHeight), appearance(false, false, false, false, false, false, false));
	m_form->move(nana::point(0, 0));
	m_form->caption(m_marker.lid > 0 ? "Edit Marker" : "Add Marker");

	picture bg1{ *m_form, rectangle(0, 0, formWidth, formHeight) };
	bg1.load(bg1Image, rectangle(0, 0, 590, 340));
	bg1.stretchable(0, 0, 0, 0);

	picture bg2{ bg1, rectangle(sideBorderWidth, topBarHeight, formWidth - (2 * sideBorderWidth), formHeight - topBarHeight - bottomBarHeight) };
	bg2.load(bg2Image, rectangle(0, 0, 582, 299));
	bg2.stretchable(sideBorderWidth, 0, sideBorderWidth, bottomBarHeight);

	picture head{ *m_form, rectangle((formWidth / 2) - 61, 5, 122, 20) };
	head.load(headImage, rectangle(0, 0, 122, 20));
	head.stretchable(0, 0, 0, 0);

	label title{ *m_form, rectangle((formWidth / 2) - 5, 9, 65, 12) };
	title.transparent(true);
	title.typeface(fontTitle);
	title.fgcolor(colors::black);
	title.text_align(align::center, align_v::center);
	title.caption(m_marker.lid > 0 ? "EDIT" : "ADD");

	label lblDragger{ *m_form, nana::rectangle(0, 0, formWidth, formHeight) };
	lblDragger.transparent(true);

	dragger dragger;
	dragger.target(*m_form);
	dragger.trigger(lblDragger);

	// Form

	// Left

	label lblFreq(*m_form, rectangle(posleft, 40, contentWidth, 20));
	lblFreq.transparent(true);
	lblFreq.typeface(fontLarge);
	lblFreq.fgcolor(colors::white);
	lblFreq.text_align(align::right);
	lblFreq.caption("Frequency: " + marker::format_frequency(m_marker.frequency) + " Hz");
	
	label lblNameC(*m_form, rectangle(posleft, 65, half, 10));
	lblNameC.transparent(true);
	lblNameC.typeface(fontTitle);
	lblNameC.fgcolor(color_rgb(0x777777));
	lblNameC.caption("NAME");

	textbox txtName(*m_form, rectangle(posleft, 77, half, 24));
	txtName.fgcolor(colors::white);
	txtName.bgcolor(color_rgb(0x1D3C4C));
	txtName.typeface(fontNormal);
	txtName.multi_lines(false);
	txtName.caption(m_marker.name);
	txtName.tip_string("Enter name ...");
	API::effects_edge_nimbus(txtName, effects::edge_nimbus::none);
	txtName.set_accept([&](wchar_t key) {
		return txtName.caption().size() < 25 || key == 0x7F || key == 0x08;
	});

	label lblTypeC(*m_form, rectangle(posleft, 111, half, 10));
	lblTypeC.transparent(true);
	lblTypeC.typeface(fontTitle);
	lblTypeC.fgcolor(color_rgb(0x777777));
	lblTypeC.caption("TYPE");

	combox cmbType(*m_form, rectangle(posleft, 123, half, 24));
	//cmbType.fgcolor(colors::white);
	//cmbType.bgcolor(color_rgb(0x1D3C4C));
	cmbType.typeface(fontNormal);
	cmbType.editable(false);
	API::effects_edge_nimbus(cmbType, effects::edge_nimbus::none);

	for (auto type : marker::type_names) {
		cmbType.push_back(std::string(type));
	}

	cmbType.option(m_marker.type);

	label lblModulationC(*m_form, rectangle(posleft, 157, half, 10));
	lblModulationC.transparent(true);
	lblModulationC.typeface(fontTitle);
	lblModulationC.fgcolor(color_rgb(0x777777));
	lblModulationC.caption("MODULATION");

	textbox txtModulation(*m_form, rectangle(posleft, 169, half, 24));
	txtModulation.fgcolor(colors::white);
	txtModulation.bgcolor(color_rgb(0x1D3C4C));
	txtModulation.typeface(fontNormal);
	txtModulation.multi_lines(false);
	txtModulation.caption(m_marker.modulation);
	txtModulation.tip_string("Modulation/protocol (optional) ...");
	API::effects_edge_nimbus(txtModulation, effects::edge_nimbus::none);
	txtModulation.set_accept([&](wchar_t key) {
		return txtModulation.caption().size() < 25 || key == 0x7F || key == 0x08;
	});

	label lblCountryC(*m_form, rectangle(posleft, 203, half, 10));
	lblCountryC.transparent(true);
	lblCountryC.typeface(fontTitle);
	lblCountryC.fgcolor(color_rgb(0x777777));
	lblCountryC.caption("COUNTRY");

	textbox txtCountry(*m_form, rectangle(posleft, 215, half, 24));
	txtCountry.fgcolor(colors::white);
	txtCountry.bgcolor(color_rgb(0x1D3C4C));
	txtCountry.typeface(fontNormal);
	txtCountry.multi_lines(false);
	txtCountry.caption(m_marker.country);
	txtCountry.tip_string("Transmitter ITU code (optional) ...");
	API::effects_edge_nimbus(txtCountry, effects::edge_nimbus::none);
	txtCountry.set_accept([&](wchar_t key) {
		return txtCountry.caption().size() < 3 || key == 0x7F || key == 0x08;
	});

	label lblLocationC(*m_form, rectangle(posleft, 249, half, 10));
	lblLocationC.transparent(true);
	lblLocationC.typeface(fontTitle);
	lblLocationC.fgcolor(color_rgb(0x777777));
	lblLocationC.caption("LOCATION");

	textbox txtLocation(*m_form, rectangle(posleft, 261, half, 24));
	txtLocation.fgcolor(colors::white);
	txtLocation.bgcolor(color_rgb(0x1D3C4C));
	txtLocation.typeface(fontNormal);
	txtLocation.multi_lines(false);
	txtLocation.caption(m_marker.location);
	txtLocation.tip_string("Transmitter location (optional) ...");
	API::effects_edge_nimbus(txtLocation, effects::edge_nimbus::none);
	txtLocation.set_accept([&](wchar_t key) {
		return txtLocation.caption().size() < 25 || key == 0x7F || key == 0x08;
	});

	// right
	label lblCommentC(*m_form, rectangle(posright, 65, half, 10));
	lblCommentC.transparent(true);
	lblCommentC.typeface(fontTitle);
	lblCommentC.fgcolor(color_rgb(0x777777));
	lblCommentC.caption("DESCRIPTION");

	textbox txtComment(*m_form, rectangle(posright, 77, half, 208));
	txtComment.fgcolor(colors::white);
	txtComment.bgcolor(color_rgb(0x1D3C4C));
	txtComment.typeface(fontSmall);
	txtComment.multi_lines(true);
	//txtComment.line_wrapped(true);
	txtComment.caption(m_marker.comment);
	txtComment.tip_string("Enter description ...");
	API::effects_edge_nimbus(txtComment, effects::edge_nimbus::none);
	txtComment.set_accept([&](wchar_t key) {
		return txtComment.caption().size() < 1000 || key == 0x7F || key == 0x08;
	});

	// full

	label lblSharedC(*m_form, rectangle(posleft, 300, full, 10));
	lblSharedC.transparent(true);
	lblSharedC.typeface(fontTitle);
	lblSharedC.fgcolor(color_rgb(0x777777));
	lblSharedC.caption("CLOUD SYNC");

	checkbox chkShared(*m_form, rectangle(posleft, 320, full, 30));
	chkShared.caption("Shared marker\r\nAre synced with the central server and can be used by other users of the plugin.");
	chkShared.transparent(true);
	chkShared.fgcolor(colors::white);
	chkShared.radio(true);

	checkbox chkLocal(*m_form, rectangle(posleft, 355, full, 30));
	chkLocal.caption("Local marker\r\nAre stored in a local database and are only visible for you.");
	chkLocal.transparent(true);
	chkLocal.fgcolor(colors::white);
	chkLocal.radio(true);

	radio_group grpSync;
	grpSync.add(chkShared);
	grpSync.add(chkLocal);

	chkShared.check(m_marker.synced);
	chkLocal.check(!m_marker.synced);


	// End Form

	m_btnSave = new sdrbutton(*m_form, point(margin, 400), 54);
	m_btnSave->caption("SAVE");
	m_btnSave->tooltip("Save marker");
	m_btnSave->typeface(fontTitle);

	m_btnCancel = new sdrbutton(*m_form, point(margin + 64, 400), 54);
	m_btnCancel->caption("CANCEL");
	m_btnCancel->tooltip("Cancel");
	m_btnCancel->typeface(fontTitle);

	m_btnDel = new sdrbutton(*m_form, point(contentWidth + margin - 54, 400), 54);
	m_btnDel->caption("DEL");
	m_btnDel->tooltip("Delete marker");
	m_btnDel->typeface(fontTitle);
	m_btnDel->enabled(m_marker.deleteable);

	// Events
	m_btnSave->events().click([&] {
		if (txtName.caption().empty()) {
			msgbox msgbox(*m_form, "Save marker");
			msgbox << "You must enter a name for this marker!";
			msgbox.icon(msgbox::icon_error);
			msgbox();
			return;
		}

		if (m_marker.lid > 0)
		{
			if (m_marker.synced && chkShared.checked())
			{
				msgbox msgbox(*m_form, "Update shared marker", msgbox::yes_no);
				msgbox << "Your changes on this shared marker will be commited to the remote server.\r\nAll other users can see your changes and can also edit them.\r\nOk?";
				msgbox.icon(msgbox::icon_information);
				if (msgbox() == msgbox::pick_no)
					return;
			}
			if (m_marker.synced && chkLocal.checked())
			{
				msgbox msgbox(*m_form, "Make local copy" , msgbox::yes_no);
				msgbox << "A local copy of this shared marker will be stored in your local database.\r\nThe shared marker will not change and still exist on the remote server.\r\nOk?";
				msgbox.icon(msgbox::icon_information);
				if (msgbox() == msgbox::pick_no)
					return;
			}
		}

		m_marker.name = txtName.caption();
		m_marker.type = cmbType.option();
		m_marker.modulation = txtModulation.caption();
		m_marker.country = txtCountry.caption();
		m_marker.location = txtLocation.caption();
		m_marker.comment = txtComment.caption();

		m_dataService.SaveMarker(m_marker, chkShared.checked());

		m_form->close();
	});

	m_btnCancel->events().click([&] {
		m_form->close();
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
		m_form->close();		
	});

	m_form->modality();
	m_form->show();
}
