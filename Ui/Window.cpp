#include <iomanip>

#include <nana/gui/widgets/picture.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/dragger.hpp>
#include <nana/gui/msgbox.hpp>

#include "sdrbutton.h"
#include "../common.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include "../resource.h"
#include "Window.h"
#include "EditWindow.h"

// Shouldn't need to change these
#define topBarHeight (27)
#define bottomBarHeight (8)
#define sideBorderWidth (8)

// TODO: Change these numbers to the height and width of your form
#define margin (20)
#define formWidth (567)
#define formHeight (470)
#define contentWidth (formWidth - 2 * margin)

void imageFromResource(HMODULE* hModule, HDC* hdc, int resourceId, nana::paint::image* img_border) {
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


Window::Window(channel_t channel, DataService& dataService) :
	m_channel(channel),
	m_dataService(dataService),
	m_windowReady(false),
	m_selectedMarker({ 0 }),
	m_thread(std::thread(&Window::createForm, this))
{
}

Window::~Window()
{
	m_form->close();
	m_thread.join();
}

void Window::createForm()
{
	HMODULE hModule = GetModuleHandle(L"SDRunoPlugin_CloudMarkers");
	HDC hdc = GetDC(NULL);

	nana::paint::image bg1Image{};
	nana::paint::image bg2Image{};
	nana::paint::image headImage{};

	imageFromResource(&hModule, &hdc, IDB_BITMAP_BG1, &bg1Image);
	imageFromResource(&hModule, &hdc, IDB_BITMAP_BG2, &bg2Image);
	imageFromResource(&hModule, &hdc, IDB_BITMAP_HEAD, &headImage);

	ReleaseDC(NULL, hdc);
	
	using namespace nana;

	paint::font fontSmall{ "Verdana", 8 };
	paint::font fontNormal{ "Verdana", 10 };
	paint::font fontLarge{ "Verdana", 12 };
	paint::font fontTitle{ "Verdana", 6 , paint::font::font_style(700) };

	m_form = new form(API::make_center(formWidth, formHeight), appearance(false, true, false, false, true, false, false));
	m_form->size(size(formWidth, formHeight));
	m_form->caption("CloudMarkers VRX" + std::to_string(m_channel));
	m_form->show();
		
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
	title.caption("MARKERS");

	label lblDragger{ *m_form, nana::rectangle(0, 0, formWidth, formHeight) };
	lblDragger.transparent(true);

	dragger dragger;
	dragger.target(*m_form);
	dragger.trigger(lblDragger);

	m_lblFreq = new label(*m_form, rectangle(150, 40, formWidth - 170, 20));
	m_lblFreq->transparent(true);
	m_lblFreq->typeface(fontLarge);
	m_lblFreq->fgcolor(colors::white);
	m_lblFreq->text_align(align::right);

	m_lstMarker = new listbox(*m_form, rectangle(20, 70, formWidth - 40, 200));
	m_lstMarker->borderless(true);
	m_lstMarker->fgcolor(colors::white);
	m_lstMarker->bgcolor(color_rgb(0x1D3C4C));
	m_lstMarker->typeface(fontSmall);
	m_lstMarker->sortable(false);
	m_lstMarker->checkable(true);
	m_lstMarker->append_header("FREQUENCY", 110);
	m_lstMarker->append_header("NAME", 180);
	m_lstMarker->append_header("MODULATION", 100);
	m_lstMarker->append_header("COUNTRY", 68);
	m_lstMarker->append_header("FLAGS", 60);

	m_lstMarker->column_movable(false);
	m_lstMarker->column_resizable(false);
	m_lstMarker->scheme().header_bgcolor = color_rgb(0xFFFFFF);
	m_lstMarker->scheme().header_grabbed = color_rgb(0xFFFFFF);
	m_lstMarker->scheme().header_floated = color_rgb(0xFFFFFF);
	m_lstMarker->scheme().item_highlighted = color_rgb(0x3B7999);
	m_lstMarker->scheme().item_selected = color_rgb(0x2C5B72);
	m_lstMarker->scheme().selection_box = color_rgb(0x3B7999);
	m_lstMarker->scheme().header_splitter_area_after = 0;
	m_lstMarker->scheme().header_splitter_area_before = 0;

	
	sdrbutton btnAdd( *m_form, point(margin, 44), 54 );
	btnAdd.caption("ADD");
	btnAdd.tooltip("Add a new marker");
	btnAdd.typeface(fontTitle);

	sdrbutton btnSync(*m_form, point(margin + 54 + 10, 44), 54);
	btnSync.caption("SYNC");
	btnSync.tooltip("Sync local DB with cloud DB");
	btnSync.typeface(fontTitle);
	
	m_lblSynced = new label(*m_form, rectangle(145, 37, 200, 26));
	m_lblSynced->transparent(true);
	m_lblSynced->typeface(fontSmall);
	m_lblSynced->fgcolor(color_rgb(0xaaaaaa));

	updateSyncInfo();

	// Marker Info
	
	// Left Column

	label lblMNameC(*m_form, rectangle(margin, 280, contentWidth / 2 - 5, 10));
	lblMNameC.transparent(true);
	lblMNameC.typeface(fontTitle);
	lblMNameC.fgcolor(color_rgb(0x777777));
	lblMNameC.caption("NAME");

	m_lblMName = new label(*m_form, rectangle(margin, 290, contentWidth / 2 - 5, 19));
	m_lblMName->transparent(true);
	m_lblMName->typeface(fontNormal);
	m_lblMName->fgcolor(colors::white);
	m_lblMName->caption("Select marker ...");

	label lblMProtocolC(*m_form, rectangle(margin, 315, contentWidth / 2 - 5, 10));
	lblMProtocolC.transparent(true);
	lblMProtocolC.typeface(fontTitle);
	lblMProtocolC.fgcolor(color_rgb(0x777777));
	lblMProtocolC.caption("TYPE / MODULATION");

	m_lblMProtocol = new label(*m_form, rectangle(margin, 325, contentWidth / 2 - 5, 19), false);
	m_lblMProtocol->transparent(true);
	m_lblMProtocol->typeface(fontNormal);
	m_lblMProtocol->fgcolor(colors::white);

	label lblMLocationC(*m_form, rectangle(margin, 350, contentWidth / 2 - 5, 10));
	lblMLocationC.transparent(true);
	lblMLocationC.typeface(fontTitle);
	lblMLocationC.fgcolor(color_rgb(0x777777));
	lblMLocationC.caption("COUNTRY / LOCATION");

	m_lblMLocation = new label(*m_form, rectangle(margin, 360, contentWidth / 2 - 5, 19), false);
	m_lblMLocation->transparent(true);
	m_lblMLocation->typeface(fontNormal);
	m_lblMLocation->fgcolor(colors::white);

	label lblMFlagsC(*m_form, rectangle(margin, 385, contentWidth / 2 - 5, 10));
	lblMFlagsC.transparent(true);
	lblMFlagsC.typeface(fontTitle);
	lblMFlagsC.fgcolor(color_rgb(0x777777));
	lblMFlagsC.caption("SCORE / FLAGS");

	m_lblMlags = new label(*m_form, rectangle(margin, 395, contentWidth / 2 - 5, 19), false);
	m_lblMlags->transparent(true);
	m_lblMlags->typeface(fontSmall);
	m_lblMlags->fgcolor(colors::white);

	m_btnEdit = new sdrbutton(*m_form, point(margin, 430), 54);
	m_btnEdit->caption("EDIT");
	m_btnEdit->tooltip("Edit selected marker");
	m_btnEdit->typeface(fontTitle);
	m_btnEdit->enabled(false);

	m_btnVote = new sdrbutton(*m_form, point(margin + 54 + 10, 430), 54);
	m_btnVote->caption("VOTE");
	m_btnVote->tooltip("Up-/DownVote selected marker");
	m_btnVote->typeface(fontTitle);
	m_btnVote->enabled(false);

	label lblVersion(*m_form, rectangle(150, 425, contentWidth / 2 - 5 - 150, 26));
	lblVersion.transparent(true);
	lblVersion.typeface(fontSmall);
	lblVersion.fgcolor(color_rgb(0xaaaaaa));
	lblVersion.caption("www.markers.cloud\nVersion 0.0.1");

	// Right Column

	label lblMCommentC(*m_form, rectangle(formWidth / 2, 280, contentWidth, 10));
	lblMCommentC.transparent(true);
	lblMCommentC.typeface(fontTitle);
	lblMCommentC.fgcolor(color_rgb(0x777777));
	lblMCommentC.caption("COMMENTS");

	m_lblMComment = new label(*m_form, rectangle(formWidth / 2, 290, contentWidth, 125), false);
	m_lblMComment->transparent(true);
	m_lblMComment->typeface(fontSmall);
	m_lblMComment->fgcolor(colors::white);

	// Events

	btnSync.events().click([&] {
		m_dataService.SyncMarkers();
		updateSyncInfo();
	});

	m_btnEdit->events().click([&] {
		EditWindow editWindow(m_form, m_dataService, m_selectedMarker);

		updateSyncInfo();
	});

	btnAdd.events().click([&] {
		marker::marker_t newMarker;
		newMarker.frequency = m_vfoFrequency;
		newMarker.lid = 0;
		newMarker.cid = 0;
		newMarker.oid = m_dataService.GetMyOid();
		newMarker.type = 0;
		newMarker.synced = true;
		newMarker.owner = true;
		newMarker.readOnly = false;
		newMarker.show = false;
		newMarker.deleteable = false;

		EditWindow editWindow(m_form, m_dataService, newMarker);

		updateSyncInfo();
	});

	m_btnVote->events().click([&] {
		msgbox msgVote(*m_form, "Vote for this marker", msgbox::yes_no_cancel);
		msgVote.icon(msgbox::icon_question);
		msgVote << "\"Yes\" vote +1" << std::endl << "\"No\" vote -1" << std::endl << "\"Cancel\" no vote";
		auto answer = msgVote();

		int vote = 0;
		if (answer == msgbox::pick_yes)
			vote = 1;
		if (answer == msgbox::pick_no)
			vote = -1;

		m_selectedMarker.vote = vote;

		m_dataService.SaveMarkerVote(m_selectedMarker);
	});

	m_lstMarker->events().checked([&](const arg_listbox& arg) {
		if (m_windowReady) {
			auto marker = arg.item.value<marker::marker_t>();
			marker.show = arg.item.checked();
			m_dataService.SaveMarkerShow(marker);
		}		
	});

	m_lstMarker->events().selected([&](const arg_listbox& arg) {
		if (m_windowReady) {
			m_selectedMarker = arg.item.selected() ? arg.item.value<marker::marker_t>() : marker::marker_t{ 0 };
			updateMarkerInfo();
		}
	});

	m_lstMarker->events().dbl_click([&]() {
		if (m_windowReady) {
			EditWindow editWindow(m_form, m_dataService, m_selectedMarker);
		}
		});

	m_windowReady = true;

	exec();
}

void Window::updateSyncInfo()
{
	auto syncinfo = m_dataService.GetLastSync();
	auto markercount = std::to_string(m_dataService.GetMarkerCount());
	auto oid = std::to_string(m_dataService.GetMyOid());

	m_lblSynced->caption("Last: " + syncinfo.time + "\r\nCommit: " + syncinfo.commit + " Marker: " + markercount + " (" + syncinfo.count + ")");
	m_lblSynced->tooltip("MyOid: " + oid);
}

void Window::updateMarkerInfo() {
	if (m_selectedMarker.lid > 0) {
		m_lblMName->caption(m_selectedMarker.name);
		m_lblMLocation->caption(m_selectedMarker.country + " " + m_selectedMarker.location);
		m_lblMLocation->show();
		m_lblMProtocol->caption(std::string(marker::type_names[m_selectedMarker.type]) + " " + m_selectedMarker.modulation);
		m_lblMProtocol->show();
		
		if (m_selectedMarker.synced)
			m_lblMlags->caption((m_selectedMarker.score > 0 ? "+" : "") + std::to_string(m_selectedMarker.score) + m_selectedMarker.flagsl);
		else
			m_lblMlags->caption(m_selectedMarker.flagsl.substr(1));

		m_lblMlags->show();
		m_lblMComment->caption(m_selectedMarker.comment);
		m_lblMComment->show();

		if (!m_selectedMarker.readOnly)
			m_btnEdit->enabled(true);
		
		if (!m_selectedMarker.owner && m_selectedMarker.synced)
			m_btnVote->enabled(true);
	}
	else
	{
		m_lblMName->caption("Select marker ...");
		m_lblMLocation->hide();
		m_lblMProtocol->hide();
		m_lblMlags->hide();
		m_lblMComment->hide();

		m_btnEdit->enabled(false);
		m_btnVote->enabled(false);
	}
}

nana::listbox::oresolver& operator<<(nana::listbox::oresolver& ores, const marker::marker_t& marker)
{
	return ores << marker::format_frequency(marker.frequency) << marker.name << marker.modulation << marker.country << marker.flags;
}

void Window::UpdateWindow(const long long vfoFrequency)
{
	m_vfoFrequency = vfoFrequency;

	while (!m_windowReady)
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

	m_windowReady = false;

	m_lblFreq->caption("VRX" + std::to_string(m_channel) + ": " + marker::format_frequency(vfoFrequency) + " Hz");
	
	m_lstMarker->auto_draw(false);
	m_lstMarker->clear();
	auto category = m_lstMarker->at(0);
	for (auto marker : m_dataService.GetMarkers(vfoFrequency, 10000)) {
		auto item = category.append(marker, true);

		item.check(marker.show);
		item.fgcolor(nana::color_rgb(marker::type_colors[marker.type]));

		if (m_selectedMarker.lid == marker.lid) {
			item.select(true);
			m_selectedMarker = marker;
		}
	}
	m_lstMarker->auto_draw(true);

	if (m_lstMarker->selected().size() == 0) {
		m_selectedMarker = marker::marker_t{ 0 };
	}

	updateMarkerInfo();

	m_windowReady = true;
}