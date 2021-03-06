#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/checkbox.hpp>
#include <nana/gui/widgets/combox.hpp>
#include <nana/gui/widgets/progress.hpp>
#include <nana/gui/widgets/picture.hpp>
#include <nana/gui/widgets/spinbox.hpp>

#include <Windows.h>
#include <thread>	


#include "sdrbutton.h"
#include "../resource.h"
#include "../DataService.h"

class sdrwindow
{
public:
	sdrwindow(std::string caption, int w, int h, DataService& dataService);
	sdrwindow(nana::form* parent, std::string caption, int w, int h, DataService& dataService);
	~sdrwindow();

	void Run();
	void Close();
	void RegisterCallback(std::function<void()> callback);

protected:
	const nana::paint::font m_fontSmall{ "Verdana", 8 };
	const nana::paint::font m_fontNormal{ "Verdana", 10 };
	const nana::paint::font m_fontLarge{ "Verdana", 12 };
	const nana::paint::font m_fontTitle{ "Verdana", 6 , nana::paint::font::font_style(700) };

	DataService& m_dataService;
	std::string m_caption;
	nana::form* m_form;
	nana::form* m_parent;
	int m_width;
	int m_height;

	virtual void createWidgets() = 0;
	bool isWindowReady() { return m_windowReady; };
	void setWindowReady(bool ready) { m_windowReady = ready; };

	std::unique_ptr<nana::label> makeLabel(int x, int y, int w, int h);
	std::unique_ptr<nana::label> makeLabelLarge(int x, int y, int w, int h);
	std::unique_ptr<nana::label> makeLabelTitle(int x, int y, int w, std::string caption);
	std::unique_ptr<nana::textbox> makeTextbox(int x, int y, int w, int h, bool multiline, std::string placeholder);
	std::unique_ptr<nana::combox> makeCombox(int x, int y, int w, int h);
	std::unique_ptr<nana::checkbox> makeRadio(int x, int y, int w, int h, std::string caption);
	std::unique_ptr<nana::radio_group> makeRadioGroup();
	std::unique_ptr<nana::sdrbutton> makeButton(int x, int y, std::string caption, std::string tooltip);
	std::unique_ptr<nana::progress> makeProgress(int x, int y, int w, int h);
	std::unique_ptr<nana::spinbox> makeSpinbox(int x, int y, int w, int h);



private:
	std::function<void()> m_closeCallback;
	std::atomic_bool m_windowReady;
	bool m_modal;

	std::unique_ptr<nana::picture> m_picMinIcon;
	std::unique_ptr<nana::picture> m_picCloseIcon;
	std::unique_ptr<nana::picture> m_picSettIcon;

	void imageFromResource(HMODULE* hModule, HDC* hdc, int resourceId, nana::paint::image* img_border);
};

