#pragma once

#include <set>

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
	std::set<marker::marker_t> m_markers;

	IUnoAnnotatorItem* makeAnnotation(marker::marker_t marker, int y, bool selected = false);
};

