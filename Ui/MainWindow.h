#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/listbox.hpp>

#include "../common.h"
#include "../DataService.h"

#include "sdrwindow.h"
#include "sdrbutton.h"

class MainWindow : public sdrwindow
{
public:
	MainWindow(channel_t channel, DataService& dataService);
	~MainWindow();

	void UpdateWindow(const long long vfoFrequency);
	void BeforeClose();

private:
	channel_t m_channel;

	double m_vfoFrequency;
	marker::marker_t m_selectedMarker;

	void createWidgets();
	void updateMarkerInfo();
	void updateSyncInfo();

	nana::label* m_lblFreq;
	nana::label* m_lblSynced;
	nana::label* m_lblVersion;

	nana::label* m_lblMNameC;
	nana::label* m_lblMProtocolC;
	nana::label* m_lblMLocationC;
	nana::label* m_lblMFlagsC;
	nana::label* m_lblMCommentC;
	
	nana::label* m_lblMName;
	nana::label* m_lblMProtocol;
	nana::label* m_lblMLocation;
	nana::label* m_lblMlags;
	nana::label* m_lblMComment;

	nana::listbox* m_lstMarker;

	nana::sdrbutton* m_btnEdit;
	nana::sdrbutton* m_btnVote;
	nana::sdrbutton* m_btnAdd;
	nana::sdrbutton* m_btnSync;
};
