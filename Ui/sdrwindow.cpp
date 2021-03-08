#include "sdrwindow.h"

#include <nana/gui/widgets/picture.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/dragger.hpp>

#include "SettingsWindow.h"

void sdrwindow::imageFromResource(HMODULE* hModule, HDC* hdc, int resourceId, nana::paint::image* img_border) {
	HRSRC resource = NULL;
	HBITMAP bitmap = NULL;
	BITMAPINFO bitmap_info = { 0 };
	BITMAPFILEHEADER bitmap_header = { 0 };
	BYTE* pixels = NULL;

	const unsigned int rawDataOffset = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);
	resource = FindResource(*hModule, MAKEINTRESOURCE(resourceId), RT_BITMAP);
	bitmap = (HBITMAP)LoadImage(*hModule, MAKEINTRESOURCE(resourceId), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
	GetDIBits(*hdc, bitmap, 0, 0, NULL, &bitmap_info, DIB_RGB_COLORS);
	bitmap_info.bmiHeader.biCompression = BI_RGB;
	bitmap_header.bfOffBits = rawDataOffset;
	bitmap_header.bfSize = bitmap_info.bmiHeader.biSizeImage;
	bitmap_header.bfType = 0x4D42;
	pixels = new BYTE[bitmap_info.bmiHeader.biSizeImage + rawDataOffset];
	*(BITMAPFILEHEADER*)pixels = bitmap_header;
	*(BITMAPINFO*)(pixels + sizeof(BITMAPFILEHEADER)) = bitmap_info;
	GetDIBits(*hdc, bitmap, 0, bitmap_info.bmiHeader.biHeight, (LPVOID)(pixels + rawDataOffset), &bitmap_info, DIB_RGB_COLORS);

	img_border->open(pixels, bitmap_info.bmiHeader.biSizeImage);

	delete[] pixels;
}

sdrwindow::sdrwindow(std::string caption, int w, int h, DataService& dataservice)
	: m_caption(caption), m_width(w), m_height(h), m_dataService(dataservice), m_parent(nullptr), m_windowReady(false), m_modal(false)
{
}

sdrwindow::sdrwindow(nana::form* parent, std::string caption, int w, int h, DataService& dataservice)
	: m_caption(caption), m_width(w), m_height(h), m_dataService(dataservice), m_parent(parent), m_windowReady(false), m_modal(true)
{
}

sdrwindow::~sdrwindow()
{
	m_form->close();
}

void sdrwindow::Close() {
	m_form->close();
}

void sdrwindow::Show()
{
	Show({ 0, 0 });
}

void sdrwindow::Show(nana::point point)
{
	HMODULE hModule = GetModuleHandle(L"SDRunoPlugin_CloudMarkers");
	HDC hdc = GetDC(NULL);

	nana::paint::image bg1Image{};
	nana::paint::image bg2Image{};
	nana::paint::image headImage{};
	nana::paint::image minImage{};
	nana::paint::image minImageDn{};
	nana::paint::image closeImage{};
	nana::paint::image closeImageDn{};
	nana::paint::image settImage{};
	nana::paint::image settImageDn{};

	imageFromResource(&hModule, &hdc, IDB_BITMAP_BG1, &bg1Image);
	imageFromResource(&hModule, &hdc, IDB_BITMAP_BG2, &bg2Image);
	imageFromResource(&hModule, &hdc, IDB_BITMAP_HEAD, &headImage);
	imageFromResource(&hModule, &hdc, IDB_BITMAP_MIN, &minImage);
	imageFromResource(&hModule, &hdc, IDB_BITMAP_MIN_DN, &minImageDn);
	imageFromResource(&hModule, &hdc, IDB_BITMAP_CLOSE, &closeImage);
	imageFromResource(&hModule, &hdc, IDB_BITMAP_CLOSE_DN, &closeImageDn);
	imageFromResource(&hModule, &hdc, IDB_BITMAP_SET, &settImage);
	imageFromResource(&hModule, &hdc, IDB_BITMAP_SET_DN, &settImageDn);

	ReleaseDC(NULL, hdc);

	using namespace nana;

	if (m_modal) {
		m_form = new form(*m_parent, size(m_width, m_height), appearance(false, false, true, false, false, false, false));
		m_form->move(point);
		m_form->caption(m_caption);
		m_form->show();
	}
	else
	{
		m_form = new form(API::make_center(m_width, m_height), appearance(false, true, true, false, true, false, false));
		m_form->size(size(m_width, m_height));
	}

	m_form->caption(m_caption);

	picture bg1{ *m_form, rectangle(0, 0, m_width, m_height) };
	bg1.load(bg1Image, rectangle(0, 0, 590, 340));
	bg1.stretchable(0, 0, 0, 0);

	picture bg2{ bg1, rectangle(8, 27, m_width - (2 * 8), m_height - 27 - 8) };
	bg2.load(bg2Image, rectangle(0, 0, 582, 299));
	bg2.stretchable(8, 0, 8, 8);

	picture head{ *m_form, rectangle((m_width / 2) - 61, 5, 122, 20) };
	head.load(headImage, rectangle(0, 0, 122, 20));
	head.stretchable(0, 0, 0, 0);

	label title{ *m_form, rectangle((m_width / 2) - 5, 9, 65, 12) };
	title.transparent(true);
	title.typeface(m_fontTitle);
	title.fgcolor(colors::black);
	title.text_align(align::center, align_v::center);
	title.caption(m_caption);

	label lblDragger{ *m_form, nana::rectangle(0, 0, m_width, m_height) };
	lblDragger.transparent(true);

	dragger dragger;
	dragger.target(*m_form);
	dragger.trigger(lblDragger);

	if (!m_modal) {
		m_picMinIcon = std::unique_ptr<nana::picture>(new picture(*m_form, nana::rectangle(0, 0, 20, 15)));
		m_picMinIcon->load(minImage, nana::rectangle(0, 0, 20, 15));
		m_picMinIcon->bgcolor(nana::color_rgb(0x000000));
		m_picMinIcon->move(nana::point(m_width - 55, 8));
		m_picMinIcon->transparent(true);
		m_picMinIcon->tooltip("Minimize Window");
		m_picMinIcon->events().mouse_down([&] { m_picMinIcon->load(minImageDn, nana::rectangle(0, 0, 20, 15)); });
		m_picMinIcon->events().mouse_up([&] { m_picMinIcon->load(minImage, nana::rectangle(0, 0, 20, 15)); nana::API::zoom_window(m_form->handle(), false); });
		m_picMinIcon->events().mouse_leave([&] { m_picMinIcon->load(minImage, nana::rectangle(0, 0, 20, 15)); });

		m_picCloseIcon = std::unique_ptr<nana::picture>(new picture(*m_form, nana::rectangle(0, 0, 20, 15)));
		m_picCloseIcon->load(closeImage, nana::rectangle(0, 0, 20, 15));
		m_picCloseIcon->bgcolor(nana::color_rgb(0x000000));
		m_picCloseIcon->move(nana::point(m_width - 30, 8));
		m_picCloseIcon->transparent(true);
		m_picCloseIcon->tooltip("Exit plugin");
		m_picCloseIcon->events().mouse_down([&] { m_picCloseIcon->load(closeImageDn, nana::rectangle(0, 0, 20, 15)); });
		m_picCloseIcon->events().mouse_up([&] { m_picCloseIcon->load(closeImage, nana::rectangle(0, 0, 20, 15)); m_closeCallback(); });
		m_picCloseIcon->events().mouse_leave([&] { m_picCloseIcon->load(closeImage, nana::rectangle(0, 0, 20, 15)); });

		m_picSettIcon = std::unique_ptr<nana::picture>(new picture(*m_form, nana::rectangle(0, 0, 40, 15)));
		m_picSettIcon->load(settImage, nana::rectangle(0, 0, 40, 15));
		m_picSettIcon->bgcolor(nana::color_rgb(0x000000));
		m_picSettIcon->move(nana::point(10, 8));
		m_picSettIcon->transparent(true);
		m_picSettIcon->tooltip("Show settings window");
		m_picSettIcon->events().mouse_down([&] { m_picSettIcon->load(settImageDn, nana::rectangle(0, 0, 40, 15)); });
		m_picSettIcon->events().mouse_leave([&] { m_picSettIcon->load(settImage, nana::rectangle(0, 0, 40, 15)); });
		m_picSettIcon->events().mouse_up([&] {
			m_picSettIcon->load(settImage, nana::rectangle(0, 0, 40, 15));
			SettingsWindow settingsWindow{ m_form, m_dataService };
			settingsWindow.Show();
		});
	}

	createWidgets();

	m_windowReady = true;
	
	if (m_modal)
	{
		m_form->modality();
		m_form->show();
	}
	else
	{
		m_form->show();
		nana::exec();

		m_windowReady = false;
	}
}

std::unique_ptr<nana::label> sdrwindow::makeLabelLarge(int x, int y, int w)
{
	auto label = std::unique_ptr<nana::label>(new nana::label(*m_form, nana::rectangle(x, y, w, 20)));
	label->transparent(true);
	label->typeface(m_fontLarge);
	label->fgcolor(nana::colors::white);
	return label;
}

std::unique_ptr<nana::label> sdrwindow::makeLabelTitle(int x, int y, int w, std::string caption)
{
	auto label = std::unique_ptr<nana::label>(new nana::label(*m_form, nana::rectangle(x, y, w, 10)));
	label->transparent(true);
	label->caption(caption);
	label->typeface(m_fontTitle);
	label->fgcolor(nana::color_rgb(0xaaaaaa));
	return label;
}

std::unique_ptr<nana::label> sdrwindow::makeLabel(int x, int y, int w, int h)
{
	auto label = std::unique_ptr<nana::label>(new nana::label(*m_form, nana::rectangle(x, y, w, h)));
	label->transparent(true);
	label->typeface(m_fontNormal);
	label->fgcolor(nana::colors::white);
	return label;
}

std::unique_ptr<nana::textbox> sdrwindow::makeTextbox(int x, int y, int w, int h, bool multiline, std::string placeholder)
{
	auto textbox = std::unique_ptr<nana::textbox>(new nana::textbox(*m_form, nana::rectangle(x, y, w, h)));
	textbox->fgcolor(nana::colors::white);
	textbox->bgcolor(nana::color_rgb(0x1F3F50));
	textbox->typeface(multiline ? m_fontSmall : m_fontNormal);
	textbox->multi_lines(multiline);
	textbox->tip_string(placeholder);

	nana::API::effects_edge_nimbus(*textbox, nana::effects::edge_nimbus::none);

	return textbox;
}

std::unique_ptr<nana::spinbox> sdrwindow::makeSpinbox(int x, int y, int w, int h)
{
	auto spinbox = std::unique_ptr<nana::spinbox>(new nana::spinbox(*m_form, nana::rectangle(x, y, w, h)));
	spinbox->fgcolor(nana::colors::white);
	spinbox->bgcolor(nana::color_rgb(0x1F3F50));
	spinbox->typeface(m_fontNormal);
	spinbox->editable(false);

	nana::API::effects_edge_nimbus(*spinbox, nana::effects::edge_nimbus::none);

	return spinbox;
}

std::unique_ptr<nana::combox> sdrwindow::makeCombox(int x, int y, int w, int h)
{
	auto combox = std::unique_ptr<nana::combox>(new nana::combox(*m_form, nana::rectangle(x, y, w, h)));
	combox->typeface(m_fontNormal);
	combox->editable(false);

	nana::API::effects_edge_nimbus(*combox, nana::effects::edge_nimbus::none);
	
	return combox;
}

std::unique_ptr<nana::checkbox> sdrwindow::makeRadio(int x, int y, int w, int h, std::string caption)
{
	auto radio = std::unique_ptr<nana::checkbox>(new nana::checkbox(*m_form, nana::rectangle(x, y, w, h)));
	radio->caption(caption);
	radio->transparent(true);
	radio->fgcolor(nana::colors::white);
	radio->radio(true);
	return radio;
}

std::unique_ptr<nana::radio_group> sdrwindow::makeRadioGroup()
{
	return std::unique_ptr<nana::radio_group>(new nana::radio_group);
}

std::unique_ptr<nana::sdrbutton> sdrwindow::makeButton(int x, int y, int w, std::string caption, std::string tooltip)
{
	auto button = std::unique_ptr<nana::sdrbutton>(new nana::sdrbutton(*m_form, nana::point(x, y), w));
	button->caption(caption);
	button->tooltip(tooltip);
	button->typeface(m_fontTitle);
	return button;
}

std::unique_ptr<nana::listbox> sdrwindow::makeListbox(int x, int y, int w, int h)
{
	auto listbox = std::unique_ptr<nana::listbox>(new nana::listbox(*m_form, nana::rectangle(x, y, w, h)));
	listbox->borderless(true);
	listbox->fgcolor(nana::colors::white);
	listbox->bgcolor(nana::color_rgb(0x1F3F50));
	listbox->typeface(m_fontSmall);
	listbox->sortable(false);
	listbox->checkable(false);
	listbox->column_movable(false);
	listbox->column_resizable(false);
	listbox->scheme().header_bgcolor = nana::color_rgb(0xFFFFFF);
	listbox->scheme().header_grabbed = nana::color_rgb(0xFFFFFF);
	listbox->scheme().header_floated = nana::color_rgb(0xFFFFFF);
	listbox->scheme().item_highlighted = nana::color_rgb(0x3B7999);
	listbox->scheme().item_selected = nana::color_rgb(0x2C5B72);
	listbox->scheme().selection_box = nana::color_rgb(0x3B7999);
	listbox->scheme().header_splitter_area_after = 0;
	listbox->scheme().header_splitter_area_before = 0;
	return listbox;
}

std::unique_ptr<nana::sdrbutton> sdrwindow::makeButton(int x, int y, std::string caption, std::string tooltip)
{
	return makeButton(x, y, 54, caption, tooltip);
}

std::unique_ptr<nana::progress> sdrwindow::makeProgress(int x, int y, int w, int h)
{
	auto progress = std::unique_ptr<nana::progress>(new nana::progress(*m_form, nana::rectangle(x, y, w, h)));
	progress->amount(100);
	return progress;
}

void sdrwindow::RegisterCallback(std::function<void()> callback)
{
	m_closeCallback = callback;
}

void sdrwindow::showVrxNo(channel_t channel)
{
	m_vrxLabel = std::unique_ptr<nana::label>(new nana::label(*m_form, nana::rectangle(m_width - 110, 5, 50, 20)));
	m_vrxLabel->typeface(m_fontVrx);
	m_vrxLabel->fgcolor(nana::color_rgb(0x00d1d1));
	m_vrxLabel->bgcolor(nana::colors::black);
	m_vrxLabel->text_align(nana::align::center, nana::align_v::top);
	m_vrxLabel->caption("0-" + ((channel < 10 ? "0" : "") + std::to_string(channel)));

}

