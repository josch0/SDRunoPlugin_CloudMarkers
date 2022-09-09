#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/checkbox.hpp>
#include <nana/gui/widgets/combox.hpp>

#include "../DataService.h"

#include "sdrwindow.h"
#include "sdrbutton.h"

class EditWindow : public sdrwindow
{
public:
	EditWindow(nana::form* parent, DataService& dataService, marker::marker_t marker);

private:
	marker::marker_t m_marker;

	void createWidgets();
	std::string spotted(std::string qth, std::string note);

	void updateTuning(int modulation, bool actual);

	std::unique_ptr<nana::label> m_lblFreqC;
	std::unique_ptr<nana::label> m_lblNameC;
	std::unique_ptr<nana::label> m_lblTypeC;
	std::unique_ptr<nana::label> m_lblModulationC;
	std::unique_ptr<nana::label> m_lblCountryC;
	std::unique_ptr<nana::label> m_lblLocationC;
	std::unique_ptr<nana::label> m_lblCommentC;
	std::unique_ptr<nana::label> m_lblSharedC;

	std::unique_ptr<nana::textbox> m_txtFreq;
	std::unique_ptr<nana::textbox> m_txtName;
	std::unique_ptr<nana::textbox> m_txtModulation;
	std::unique_ptr<nana::textbox> m_txtCountry;
	std::unique_ptr<nana::textbox> m_txtLocation;
	std::unique_ptr<nana::textbox> m_txtComment;
		
	std::unique_ptr<nana::combox> m_cmbType;
		
	std::unique_ptr<nana::checkbox> m_chkShared;
	std::unique_ptr<nana::checkbox> m_chkLocal;
		
	std::unique_ptr<nana::radio_group> m_grpSync;

	std::unique_ptr<nana::sdrbutton> m_btnFreqP10;
	std::unique_ptr<nana::sdrbutton> m_btnFreqM10;
	std::unique_ptr<nana::sdrbutton> m_btnFreqP1;
	std::unique_ptr<nana::sdrbutton> m_btnFreqM1;
	std::unique_ptr<nana::sdrbutton> m_btnFreqP01;
	std::unique_ptr<nana::sdrbutton> m_btnFreqM01;
	std::unique_ptr<nana::sdrbutton> m_btnFreqP001;
	std::unique_ptr<nana::sdrbutton> m_btnFreqM001;
	std::unique_ptr<nana::sdrbutton> m_btnFreqR0;
	std::unique_ptr<nana::sdrbutton> m_btnFreqR5;

	std::unique_ptr<nana::sdrbutton> m_btnSearch;
	std::unique_ptr<nana::sdrbutton> m_btnSpot;

	std::unique_ptr<nana::label> m_lblTuneC;
	std::unique_ptr<nana::combox> m_cmbTuneMod;
	std::unique_ptr<nana::combox> m_cmbTuneBw;
	std::unique_ptr<nana::checkbox> m_chkTuneCenter;

	std::unique_ptr<nana::sdrbutton> m_btnSave;
	std::unique_ptr<nana::sdrbutton> m_btnCancel;
	std::unique_ptr<nana::sdrbutton> m_btnDel;
};
