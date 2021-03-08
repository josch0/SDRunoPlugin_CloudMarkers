#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/spinbox.hpp>

#include "../DataService.h"
#include "../common.h"

#include "sdrwindow.h"
#include "sdrbutton.h"

class SettingsWindow : public sdrwindow
{
public:
	SettingsWindow(nana::form* parent, DataService& dataService);

private:
	settings::types_t m_types;

	void createWidgets();
	void colorError(std::string name);
	bool validColorKey(int size, wchar_t key);
	bool validLocator(int size, wchar_t key);

	std::unique_ptr<nana::label> m_lblColor0C;
	std::unique_ptr<nana::label> m_lblColor1C;
	std::unique_ptr<nana::label> m_lblColor2C;
	std::unique_ptr<nana::label> m_lblColor3C;
	std::unique_ptr<nana::label> m_lblColor4C;
	std::unique_ptr<nana::label> m_lblColor5C;
	std::unique_ptr<nana::label> m_lblColor6C;
	std::unique_ptr<nana::label> m_lblColor8C;

	std::unique_ptr<nana::textbox> m_txtColor0;
	std::unique_ptr<nana::textbox> m_txtColor1;
	std::unique_ptr<nana::textbox> m_txtColor2;
	std::unique_ptr<nana::textbox> m_txtColor3;
	std::unique_ptr<nana::textbox> m_txtColor4;
	std::unique_ptr<nana::textbox> m_txtColor5;
	std::unique_ptr<nana::textbox> m_txtColor6;
	std::unique_ptr<nana::textbox> m_txtColor8;

	std::unique_ptr<nana::label> m_lblOffset;
	std::unique_ptr<nana::spinbox> m_spnOffset;

	std::unique_ptr<nana::label> m_lblQthC;
	std::unique_ptr<nana::textbox> m_txtQth0;

	std::unique_ptr<nana::sdrbutton> m_btnSave;
	std::unique_ptr<nana::sdrbutton> m_btnCancel;
};

