#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/listbox.hpp>

#include "sdrwindow.h"

class CountryWindow : public sdrwindow
{
public:
	CountryWindow(nana::form* parent, DataService& dataService, std::string current);
	std::string GetITUCode();

private:
	std::string m_ituCode;

	void createWidgets();
	void updateList(std::string searchText);

	std::unique_ptr<nana::label> m_lblSearchC;
	std::unique_ptr<nana::textbox> m_txtSearch;
	std::unique_ptr<nana::label> m_lblResultsC;
	std::unique_ptr<nana::listbox> m_lstResults;
	std::unique_ptr<nana::sdrbutton> m_btnOk;
	std::unique_ptr<nana::sdrbutton> m_btnCancel;
};

