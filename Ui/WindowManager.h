#pragma once

#include <vector>

#include "Window.h"
#include "../DataService.h"

class WindowManager
{
public:
	WindowManager(IUnoPluginController& controller, DataService& dataService);
	~WindowManager();

	void UpdateVrxCount();
	void UpdateWindow(const channel_t channel);
	void UpdateWindows();

private:
	IUnoPluginController& m_controller;
	DataService& m_dataService;
	std::vector<Window*> m_windowList;
};

