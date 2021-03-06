#include "SyncWindow.h"

SyncWindow::SyncWindow(nana::form* parent, DataService& dataService) :
	sdrwindow(parent, "SYNC", 300, 150, dataService)
{
}

void SyncWindow::createWidgets()
{
	using namespace nana;

	m_lblDownloadC = makeLabelTitle(20, 40, m_width - 40, "DOWNLOAD PROGRESS");
	m_pgrDownload = makeProgress(20, 55, m_width - 40, 20);
	m_lblImportC = makeLabelTitle(20, 85, m_width - 40, "IMPORT PROGRESS");
	m_pgrImport = makeProgress(20, 100, m_width - 40, 20);

	m_dataService.SyncMarkers([&](bool download, int progress) {
		if (download) {
			m_pgrDownload->value(progress);
			m_lblDownloadC->caption("DOWNLOAD PROGRESS (" + std::to_string(progress) + "%)");
		}
		else
		{
			m_pgrImport->value(progress);
			m_lblImportC->caption("IMPORT PROGRESS (" + std::to_string(progress) + "%)");
		}
	});

	Close();
}