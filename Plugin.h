#pragma once

#include <iunoplugincontroller.h>
#include <iunoplugin.h>

#include "Ui/WindowManager.h"
#include "DataService.h"
#include "AnnotatorService.h"

class Plugin : public IUnoPlugin
{

public:
	Plugin(IUnoPluginController& controller);
	~Plugin();

	const char* GetPluginName() const { return "CloudMarkers"; }
	void HandleEvent(const UnoEvent& event);
	void DataChangedCallback();
	void CloseCallback();

private:
	AnnotatorService m_annotatorService;
	DataService m_dataService;
	WindowManager m_windowManager;
};