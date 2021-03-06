#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/progress.hpp>

#include "../DataService.h"

#include "sdrwindow.h"


class SyncWindow : public sdrwindow
{
public:
	SyncWindow(nana::form* parent, DataService& dataService);

private:
	void createWidgets();

	std::unique_ptr<nana::label> m_lblDownloadC;
	std::unique_ptr<nana::progress> m_pgrDownload;
	std::unique_ptr<nana::label> m_lblImportC;
	std::unique_ptr<nana::progress> m_pgrImport;

};

