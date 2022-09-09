#include "CountryWindow.h"
#include "../countries.h";

#define WIDTH (400)
#define HEIGHT (300)

#define TEXTHEIGHT (24)
#define FULLWIDTH (WIDTH - 40)
#define LEFT (20)
#define ROW(x) (40 + ((x - 1) * 45))
#define TEXTROW(x) (52 + ((x - 1) * 45))

CountryWindow::CountryWindow(nana::form* parent, DataService& dataService, std::string current) :
	sdrwindow(parent, "COUNTRIES", WIDTH, HEIGHT, dataService),
	m_ituCode(current)
{
}

nana::listbox::oresolver& operator<<(nana::listbox::oresolver& ores, const staticdata::country_t& country)
{
	return ores << country.code << country.name;
}

bool findSearchTextInCountry(const std::string& searchText, const staticdata::country_t country)
{	
	if (searchText.empty())
		return true;

	auto it = std::search(
		country.name.begin(), country.name.end(),
		searchText.begin(), searchText.end(),
		[](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
	);
	return (it != country.name.end());
}

void CountryWindow::createWidgets()
{
	// Search
	m_lblSearchC = makeLabelTitle(LEFT, ROW(1), FULLWIDTH, "COUNTRY NAME");
	m_txtSearch = makeTextbox(LEFT, TEXTROW(1), FULLWIDTH, TEXTHEIGHT, false, "Start typing country name ...");
	m_txtSearch->focus();

	// Results
	m_lblResultsC = makeLabelTitle(LEFT, ROW(2), FULLWIDTH, "RESULTS");
	m_lstResults = makeListbox(LEFT, TEXTROW(2), FULLWIDTH, 150);
	m_lstResults->append_header("ITU", 60);
	m_lstResults->append_header("NAME", 260);

	// Buttons
	m_btnOk = makeButton(20, 260, "OK", "Accept selected country");
	if (m_ituCode.empty())
		m_btnOk->enabled(false);

	m_btnCancel = makeButton(84, 260, "CANCEL", "Cancel selection");

	// Events
	m_lstResults->events().selected([&](const nana::arg_listbox& arg) {
		if (arg.item.selected()) {
			m_ituCode = arg.item.text(0);
			m_btnOk->enabled(true);
		}
		else
		{
			m_ituCode = "";
			m_btnOk->enabled(false);
		}
	});

	m_lstResults->events().dbl_click([&] {
		if (!m_ituCode.empty())
			Close();
	});

	m_btnOk->events().click([&] {
		Close();
	});

	m_btnCancel->events().click([&] {
		m_ituCode = "";
		Close();
	});

	m_txtSearch->events().text_changed([&] {
		updateList(m_txtSearch->caption());
	});

	updateList("");
}

std::string CountryWindow::GetITUCode()
{
	return m_ituCode;
}

void CountryWindow::updateList(std::string searchText)
{
	m_lstResults->clear();
	auto list = m_lstResults->at(0);
	for (auto country : staticdata::countries) {
		if (findSearchTextInCountry(searchText, country)) {
			auto item = list.append(country);
			if (country.code == m_ituCode)
				item.select(true, true);
		}
	}
}