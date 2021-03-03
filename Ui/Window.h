#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/listbox.hpp>

#include <thread>

#include "../DataService.h"
#include "sdrbutton.h"

class Window
{
public:
	Window(channel_t channel, DataService& dataService);
	~Window();

	void UpdateWindow(const long long vfoFrequency);

private:
	double m_vfoFrequency;
	DataService& m_dataService;
	std::thread m_thread;
	std::atomic_bool m_windowReady;
	channel_t m_channel;

	marker::marker_t m_selectedMarker;

	void createForm();
	void updateMarkerInfo();
	void updateSyncInfo();

	nana::form* m_form;
	nana::label* m_lblFreq;
	nana::label* m_lblSynced;
	nana::listbox* m_lstMarker;
	nana::label* m_lblMName;
	nana::label* m_lblMProtocol;
	nana::label* m_lblMLocation;
	nana::label* m_lblMlags;
	nana::label* m_lblMComment;

	nana::sdrbutton* m_btnEdit;
	nana::sdrbutton* m_btnVote;
};
