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

	while (m_windowList.size() > 0)
	{
		std::pair<std::thread*, MainWindow*> item = m_windowList.back();
		auto thread = item.first;
		auto window = item.second;

		window->BeforeClose();
		thread->join();

		delete thread;
		delete window;

		m_windowList.pop_back();
	}
}

void WindowManager::UpdateVrxCount()
{
	const unsigned int vrxCount = m_controller.GetVRXCount();

	while (vrxCount > m_windowList.size())
	{
		channel_t channel = m_windowList.size();
		auto window = new MainWindow(channel, m_dataService);
		auto thread = new std::thread(&MainWindow::Run, window);

		window->RegisterCallback([&] {
			m_closeCallback();
		});

		window->UpdateWindow(static_cast<long long>(m_controller.GetVfoFrequency(channel)));

		m_windowList.push_back(std::pair<std::thread*, MainWindow*>(thread, window));
	}

	while (vrxCount < m_windowList.size())
	{
		std::pair<std::thread*, MainWindow*> item = m_windowList.back();
		item.first->join();
		delete item.first;
		delete item.second;

		m_windowList.pop_back();
	}
}

void WindowManager::UpdateWindow(const channel_t channel)
{
	if (channel < m_windowList.size()) {
		m_windowList[channel].second->UpdateWindow(static_cast<long long>(m_controller.GetVfoFrequency(channel)));
	}
}

void WindowManager::UpdateWindows()
{
	for (channel_t channel = 0; channel < m_controller.GetVRXCount(); channel++)
		UpdateWindow(channel);
}

void WindowManager::RegisterCallback(std::function<void()> callback)
{
	m_closeCallback = callback;
}