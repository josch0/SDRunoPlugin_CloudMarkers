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
	int y = 0;
	for (const marker::marker_t& marker : m_markers)
	{
		IUnoAnnotatorItem* item = new IUnoAnnotatorItem();
		item->frequency = marker.frequency;
		item->text = " " + marker.name;

		if (marker.type < 8) {
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
		
		item->rgb = marker::type_colors[marker.type];
		items.push_back(*item);
		y %= 6;
	}
}

void AnnotatorService::UpdateMarkers()
{
	std::vector<marker::marker_t> markers;
	long offset = 10000000;

	do {
		markers.clear();
		for (channel_t channel = 0; channel < m_controller.GetVRXCount(); channel++) {
			auto channelMarkers = m_dataService.GetMarkers(m_controller.GetVfoFrequency(channel), offset, true);
			markers.insert(markers.end(), channelMarkers.begin(), channelMarkers.end());
		}
		offset -= (offset / 10);
	} while (markers.size() > 64);

	m_markers = markers;
}