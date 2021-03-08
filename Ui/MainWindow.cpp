#include <nana/gui.hpp>
#include <nana/gui/msgbox.hpp>

#include "MainWindow.h"
#include "EditWindow.h"
#include "SyncWindow.h"

MainWindow::MainWindow(channel_t channel, DataService& dataService) :
	sdrwindow("MARKERS" , 567, 470, dataService),
	m_channel(channel),
	m_selectedMarker()
{
}

void MainWindow::BeforeClose()
{
	m_dataService.SaveWindowPos(m_channel, m_form->pos());
}

MainWindow::~MainWindow()
{
	delete m_lblFreq;
	delete m_lblSynced;
	delete m_lblVersion;

	delete m_lblMNameC;
	delete m_lblMProtocolC;
	delete m_lblMLocationC;
	delete m_lblMFlagsC;
	delete m_lblMCommentC;

	delete m_lblMName;
	delete m_lblMProtocol;
	delete m_lblMLocation;
	delete m_lblMlags;
	delete m_lblMComment;

	delete m_lstMarker;

	delete m_btnEdit;
	delete m_btnVote;
	delete m_btnAdd;
	delete m_btnSync;
}

void MainWindow::createWidgets()
{
	using namespace nana;

	showVrxNo(m_channel);

	m_lblFreq = new label(*m_form, rectangle(150, 40, m_width - 170, 20));
	m_lblFreq->transparent(true);
	m_lblFreq->typeface(m_fontLarge);
	m_lblFreq->fgcolor(colors::white);
	m_lblFreq->text_align(align::right);

	m_lstMarker = new listbox(*m_form, rectangle(20, 70, m_width - 40, 200));
	m_lstMarker->borderless(true);
	m_lstMarker->fgcolor(colors::white);
	m_lstMarker->bgcolor(color_rgb(0x1D3C4C));
	m_lstMarker->typeface(m_fontSmall);
	m_lstMarker->sortable(false);
	m_lstMarker->checkable(true);
	m_lstMarker->append_header("FREQUENCY", 110);
	m_lstMarker->append_header("NAME", 180);
	m_lstMarker->append_header("MODULATION", 90);
	m_lstMarker->append_header("COUNTRY", 68);
	m_lstMarker->append_header("FLAGS", 50);

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
		
	m_btnAdd = new sdrbutton( *m_form, point(20, 44), 54 );
	m_btnAdd->caption("ADD");
	m_btnAdd->tooltip("Add a new marker");
	m_btnAdd->typeface(m_fontTitle);
	
	m_btnSync = new sdrbutton(*m_form, point(20 + 54 + 10, 44), 54);
	m_btnSync->caption("SYNC");
	m_btnSync->tooltip("Sync local DB with cloud DB");
	m_btnSync->typeface(m_fontTitle);
	
	m_lblSynced = new label(*m_form, rectangle(145, 37, 200, 26));
	m_lblSynced->transparent(true);
	m_lblSynced->typeface(m_fontSmall);
	m_lblSynced->fgcolor(color_rgb(0xaaaaaa));

	updateSyncInfo();

	// Marker Info
	int contentWidth = m_width - 40;

	// Left Column

	m_lblMNameC = new label(*m_form, rectangle(20, 280, contentWidth / 2 - 5, 10));
	m_lblMNameC->transparent(true);
	m_lblMNameC->typeface(m_fontTitle);
	m_lblMNameC->fgcolor(color_rgb(0x777777));
	m_lblMNameC->caption("NAME");

	m_lblMName = new label(*m_form, rectangle(20, 290, contentWidth / 2 - 5, 19));
	m_lblMName->transparent(true);
	m_lblMName->typeface(m_fontNormal);
	m_lblMName->fgcolor(colors::white);
	m_lblMName->caption("Select marker ...");

	m_lblMProtocolC = new label(*m_form, rectangle(20, 315, contentWidth / 2 - 5, 10));
	m_lblMProtocolC->transparent(true);
	m_lblMProtocolC->typeface(m_fontTitle);
	m_lblMProtocolC->fgcolor(color_rgb(0x777777));
	m_lblMProtocolC->caption("TYPE / MODULATION");

	m_lblMProtocol = new label(*m_form, rectangle(20, 325, contentWidth / 2 - 5, 19), false);
	m_lblMProtocol->transparent(true);
	m_lblMProtocol->typeface(m_fontNormal);
	m_lblMProtocol->fgcolor(colors::white);

	m_lblMLocationC = new label(*m_form, rectangle(20, 350, contentWidth / 2 - 5, 10));
	m_lblMLocationC->transparent(true);
	m_lblMLocationC->typeface(m_fontTitle);
	m_lblMLocationC->fgcolor(color_rgb(0x777777));
	m_lblMLocationC->caption("COUNTRY / LOCATION");

	m_lblMLocation = new label(*m_form, rectangle(20, 360, contentWidth / 2 - 5, 19), false);
	m_lblMLocation->transparent(true);
	m_lblMLocation->typeface(m_fontNormal);
	m_lblMLocation->fgcolor(colors::white);

	m_lblMFlagsC = new label(*m_form, rectangle(20, 385, contentWidth / 2 - 5, 10));
	m_lblMFlagsC->transparent(true);
	m_lblMFlagsC->typeface(m_fontTitle);
	m_lblMFlagsC->fgcolor(color_rgb(0x777777));
	m_lblMFlagsC->caption("SCORE / FLAGS");

	m_lblMlags = new label(*m_form, rectangle(20, 395, contentWidth / 2 - 5, 19), false);
	m_lblMlags->transparent(true);
	m_lblMlags->typeface(m_fontSmall);
	m_lblMlags->fgcolor(colors::white);

	m_btnEdit = new sdrbutton(*m_form, point(20, 430), 54);
	m_btnEdit->caption("EDIT");
	m_btnEdit->tooltip("Edit selected marker");
	m_btnEdit->typeface(m_fontTitle);
	m_btnEdit->enabled(false);

	m_btnVote = new sdrbutton(*m_form, point(20 + 54 + 10, 430), 54);
	m_btnVote->caption("VOTE");
	m_btnVote->tooltip("Up-/DownVote selected marker");
	m_btnVote->typeface(m_fontTitle);
	m_btnVote->enabled(false);

	m_lblVersion = new label(*m_form, rectangle(150, 425, contentWidth / 2 - 5 - 150, 26));
	m_lblVersion->transparent(true);
	m_lblVersion->typeface(m_fontSmall);
	m_lblVersion->fgcolor(color_rgb(0xaaaaaa));
	m_lblVersion->text_align(nana::align::right);
	m_lblVersion->caption("www.markers.cloud\nv" + version::version);

	// Right Column

	m_lblMCommentC = new label(*m_form, rectangle(m_width / 2, 280, contentWidth, 10));
	m_lblMCommentC->transparent(true);
	m_lblMCommentC->typeface(m_fontTitle);
	m_lblMCommentC->fgcolor(color_rgb(0x777777));
	m_lblMCommentC->caption("DESCRIPTION");

	m_lblMComment = new label(*m_form, rectangle(m_width / 2, 290, contentWidth, 125), false);
	m_lblMComment->transparent(true);
	m_lblMComment->typeface(m_fontSmall);
	m_lblMComment->fgcolor(colors::white);

	// Events

	m_btnSync->events().click([&] {
		SyncWindow syncWindow(m_form, m_dataService);
		syncWindow.Show();

		updateSyncInfo();
	});

	m_btnEdit->events().click([&] {
		EditWindow editWindow(m_form, m_dataService, m_selectedMarker);
		editWindow.Show();

		updateSyncInfo();
	});

	m_btnAdd->events().click([&] {
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
		editWindow.Show();

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
		if (isWindowReady()) {
			auto marker = arg.item.value<marker::marker_t>();
			marker.show = arg.item.checked();
			m_dataService.SaveMarkerShow(marker);

			if (marker.lid == m_selectedMarker.lid) {
				m_selectedMarker.show = marker.show;
				m_dataService.setSelectedMarker(m_selectedMarker);
			}
		}		
	});

	m_lstMarker->events().selected([&](const arg_listbox& arg) {
		if (isWindowReady()) {
			m_selectedMarker = arg.item.selected() ? arg.item.value<marker::marker_t>() : marker::marker_t{};
			m_dataService.setSelectedMarker(m_selectedMarker);
			updateMarkerInfo();
		}
	});

	m_lstMarker->events().dbl_click([&]() {
		if (isWindowReady() && !m_selectedMarker.readOnly) {
			EditWindow editWindow(m_form, m_dataService, m_selectedMarker);
			editWindow.Show();
		}
	});

	m_form->move(m_dataService.LoadWindowPos(m_channel));

	if (m_dataService.UpdateAvailable()) {
		msgbox msgbox(*m_form, "CloudMarkers Plugin");
		msgbox << "A new version of this plugin is available!" << std::endl << "Please update from http://markers.cloud!";
		msgbox.icon(msgbox::icon_information);
		msgbox();
	}

}

void MainWindow::updateSyncInfo()
{
	auto syncinfo = m_dataService.GetLastSync();
	auto markercount = std::to_string(m_dataService.GetMarkerCount());
	auto oid = std::to_string(m_dataService.GetMyOid());

	m_lblSynced->caption("Last: " + syncinfo.time + " (" + syncinfo.count + ")\r\nCommitID: " + syncinfo.commit + " Markers: " + markercount);
	m_lblSynced->tooltip("MyOid: " + oid);
}

void MainWindow::updateMarkerInfo() {
	if (m_selectedMarker.lid > 0) {
		auto types = m_dataService.GetTypeSettings();

		m_lblMName->caption(m_selectedMarker.name);
		m_lblMLocation->caption(m_selectedMarker.country + " " + m_selectedMarker.location);
		m_lblMLocation->show();
		m_lblMProtocol->caption(types[m_selectedMarker.type].name + " " + m_selectedMarker.modulation);
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

void MainWindow::UpdateWindow(const long long vfoFrequency)
{
	m_vfoFrequency = vfoFrequency;

	while (!isWindowReady())
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

	setWindowReady(false);

	auto types = m_dataService.GetTypeSettings();
	auto offset = m_dataService.GetVfoOffset() * 1000;

	m_lblFreq->caption("VFO: " + marker::format_frequency(vfoFrequency) + " Hz");
	
	m_lstMarker->auto_draw(false);
	m_lstMarker->clear();
	auto category = m_lstMarker->at(0);
	for (auto marker : m_dataService.GetMarkers(vfoFrequency, offset)) {
		auto item = category.append(marker, true);

		item.check(marker.show);
		item.fgcolor(nana::color_rgb(types[marker.type].color));

		if (m_selectedMarker.lid == marker.lid) {
			item.select(true);
			m_selectedMarker = marker;
		}
	}
	m_lstMarker->auto_draw(true);

	if (m_lstMarker->selected().size() == 0) {
		m_selectedMarker = marker::marker_t{};
		m_dataService.setSelectedMarker(m_selectedMarker);
	}

	updateMarkerInfo();
	setWindowReady(true);
}