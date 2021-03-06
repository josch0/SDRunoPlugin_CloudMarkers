#include <unoevent.h>

#include "Plugin.h"

Plugin::Plugin(IUnoPluginController& controller) :
	IUnoPlugin(controller),
	m_dataService(controller),
	m_annotatorService(controller, m_dataService),
	m_windowManager(controller, m_dataService)
{
	m_dataService.RegisterCallback(std::bind(&Plugin::DataChangedCallback, this));
	m_dataService.Init();

	m_annotatorService.UpdateMarkers();
	
	m_windowManager.RegisterCallback(std::bind(&Plugin::CloseCallback, this));
	m_windowManager.UpdateVrxCount();

	m_controller.RegisterAnnotator(&m_annotatorService);
};

Plugin::~Plugin()
{
	m_controller.UnregisterAnnotator(&m_annotatorService);
}

void Plugin::HandleEvent(const UnoEvent& event)
{	
	switch (event.GetType())
	{
	case UnoEvent::VRXCountChanged:
		m_windowManager.UpdateVrxCount();
		m_annotatorService.UpdateMarkers();
		break;
	case UnoEvent::FrequencyChanged:
		m_windowManager.UpdateWindow(event.GetChannel());
		m_annotatorService.UpdateMarkers();
		break;
	case UnoEvent::ClosingDown:
		m_controller.RequestUnload(this);
		break;
	}
}

void Plugin::CloseCallback()
{
	m_controller.RequestUnload(this);
}

void Plugin::DataChangedCallback()
{
	m_annotatorService.UpdateMarkers();
	m_windowManager.UpdateWindows();
}
