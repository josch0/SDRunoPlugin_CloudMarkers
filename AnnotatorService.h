#pragma once

#include <vector>

#include <iunoplugincontroller.h>
#include <iunoannotator.h>

#include "DataService.h"

class AnnotatorService : public IUnoAnnotator
{
public:
	AnnotatorService(IUnoPluginController& controller, DataService& dataService);
	void AnnotatorProcess(std::vector<IUnoAnnotatorItem>& items);
	void UpdateMarkers();

private:
	IUnoPluginController& m_controller;
	DataService& m_dataService;
	std::vector<marker::marker_t> m_markers;
};

