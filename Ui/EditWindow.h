#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>

#include "../DataService.h"
#include "sdrbutton.h"

class EditWindow
{
public:
	EditWindow(nana::form* parent, DataService& dataService);
	EditWindow(nana::form* parent, DataService& dataService, marker::marker_t marker);
	~EditWindow();

private:
	nana::form* m_parentForm;
	DataService& m_dataService;
	marker::marker_t m_marker;

	void createForm();

	nana::form* m_form;

	nana::sdrbutton* m_btnSave;
	nana::sdrbutton* m_btnCancel;
	nana::sdrbutton* m_btnDel;
};
