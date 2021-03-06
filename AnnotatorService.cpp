#include "AnnotatorService.h"
#include "common.h"

AnnotatorService::AnnotatorService(IUnoPluginController& controller, DataService& dataService):
	IUnoAnnotator(),
	m_controller(controller),
	m_dataService(dataService)
{
}

void AnnotatorService::AnnotatorProcess(std::vector<IUnoAnnotatorItem>& items)
{
	auto selected = m_dataService.getSelectedMarker();

	int y = 0;
	for (const marker::marker_t& current : m_markers)
	{
		items.push_back(*makeAnnotation(current, y++, selected.lid == current.lid));
		y %= 6;
	}

	if (selected.lid > 0 && !selected.show) {
		items.push_back(*makeAnnotation(selected, y++, true));
	}
}

IUnoAnnotatorItem* AnnotatorService::makeAnnotation(marker::marker_t marker, int y, bool selected)
{
	auto types = m_dataService.GetTypeSettings();

	IUnoAnnotatorItem* item = new IUnoAnnotatorItem();
	item->frequency = marker.frequency;
	item->text = " " + marker.name;

	if (marker.type < 8) {
		if (selected) 
			item->style = IUnoAnnotatorStyle::AnnotatorStyleMarkerAndLine;
		else
			item->style = IUnoAnnotatorStyle::AnnotatorStyleFlag;
		item->power = -40 - (y++ * 6);
	}
	else
	{
		item->style = IUnoAnnotatorStyle::AnnotatorStyleMarkerAndLine;
		item->lineToFreq = item->frequency;
		item->power = -30;
		item->lineToPower = -160;
	}

	item->rgb = types[marker.type].color;

	return item;
}

void AnnotatorService::UpdateMarkers()
{
	std::set<marker::marker_t> markers;
	long offset = 10000000;

	do {
		markers.clear();
		for (channel_t channel = 0; channel < m_controller.GetVRXCount(); channel++) {
			auto channelMarkers = m_dataService.GetMarkers(m_controller.GetVfoFrequency(channel), offset, true);
			markers.insert(channelMarkers.begin(), channelMarkers.end());
		}
		offset -= (offset / 10);
	} while (markers.size() > 64);

	m_markers = markers;
}