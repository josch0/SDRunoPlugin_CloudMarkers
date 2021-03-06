#pragma once

#include <vector>
#include <thread>

#include "MainWindow.h"
#include "../DataService.h"

class WindowManager
{
public:
	WindowManager(IUnoPluginController& controller, DataService& dataService);
	~WindowManager();

	void UpdateVrxCount();
	void UpdateWindow(const channel_t channel);
	void UpdateWindows();

	void RegisterCallback(std::function<void()> callback);

private:
	IUnoPluginController& m_controller;
	DataService& m_dataService;
	std::vector<std::pair<std::thread*, MainWindow*>> m_windowList;

	std::function<void()> m_closeCallback;

};

