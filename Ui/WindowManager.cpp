#include "WindowManager.h"

#include <fstream>

WindowManager::WindowManager(IUnoPluginController& controller, DataService& dataService) :
	m_controller(controller),
	m_dataService(dataService)
{	
}

WindowManager::~WindowManager() 
{
	nana::API::exit_all();
}

void WindowManager::UpdateVrxCount()
{
	const unsigned int vrxCount = m_controller.GetVRXCount();

	while (vrxCount > m_windowList.size())
	{
		channel_t channel = m_windowList.size();
		auto window = new Window(channel, m_dataService);
		window->UpdateWindow(static_cast<long long>(m_controller.GetVfoFrequency(channel)));
		m_windowList.push_back(window);
	}

	while (vrxCount < m_windowList.size())
	{
		delete m_windowList.back();
		m_windowList.pop_back();
	}
}

void WindowManager::UpdateWindow(const channel_t channel)
{
	if (channel < m_windowList.size()) {
		m_windowList[channel]->UpdateWindow(static_cast<long long>(m_controller.GetVfoFrequency(channel)));
	}
}

void WindowManager::UpdateWindows()
{
	for (channel_t channel = 0; channel < m_controller.GetVRXCount(); channel++)
		UpdateWindow(channel);
}