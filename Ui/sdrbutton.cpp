#include "sdrbutton.h"
#include <nana/gui/detail/widget_content_measurer_interface.hpp>
#include <nana/paint/text_renderer.hpp>

namespace nana {
	namespace drawerbase
	{
		namespace sdrbutton
		{
			class trigger::measurer	: public dev::widget_content_measurer_interface
			{
			public:
				measurer(trigger* t)
					: trigger_{ t }
				{}

				std::optional<size> measure(graph_reference graph, unsigned limit_pixels, bool /*limit_width*/) const override
				{
					//Button doesn't provide a support of vfit and hfit
					if (limit_pixels)
						return{};

					wchar_t shortkey;
					return graph.text_extent_size(API::transform_shortkey_text(trigger_->wdg_->caption(), shortkey, nullptr));
				}

				size extension() const override
				{
					return { 14, 10 };
				}
			private:
				trigger* trigger_;
			};

			trigger::trigger()
			{
				attr_.e_state = element_state::normal;
				attr_.keep_pressed = false;

				measurer_.reset(new measurer{ this });
			}

			trigger::~trigger()
			{
			}

			void trigger::attached(widget_reference widget, graph_reference graph)
			{
				graph_ = &graph;

				wdg_ = &widget;
				window wd = widget;

				API::dev::enable_space_click(widget, true);
				API::tabstop(wd);
				API::effects_edge_nimbus(wd, effects::edge_nimbus::none);
				API::dev::set_measurer(widget, measurer_.get());
			}

			void trigger::refresh(graph_reference graph)
			{
				bool eb = wdg_->enabled();;

				element_state e_state = attr_.e_state;
				if (eb)
				{

						if (element_state::normal == e_state)
							e_state = element_state::focus_normal;
						else if (element_state::hovered == e_state)
							e_state = element_state::focus_hovered;
					
				}
				else
					e_state = element_state::disabled;

				API::dev::copy_transparent_background(*wdg_, graph);
				_m_draw_border(graph, eb);
				_m_draw_title(graph, eb);
			}


			void trigger::mouse_enter(graph_reference graph, const arg_mouse&)
			{
				attr_.e_state = (attr_.keep_pressed ? element_state::pressed : element_state::hovered);
				refresh(graph);
				API::dev::lazy_refresh();
			}

			void trigger::mouse_leave(graph_reference graph, const arg_mouse&)
			{
				attr_.e_state = element_state::normal;
				refresh(graph);
				API::dev::lazy_refresh();
			}

			void trigger::mouse_down(graph_reference graph, const arg_mouse& arg)
			{
				if (::nana::mouse::left_button != arg.button)
					return;

				_m_press(graph, true);
			}

			void trigger::mouse_up(graph_reference graph, const arg_mouse& arg)
			{
				if (::nana::mouse::left_button != arg.button)
					return;

				_m_press(graph, false);
			}

			void trigger::key_press(graph_reference, const arg_keyboard& arg)
			{
				bool ch_tabstop_next;
				switch (arg.key)
				{
				case keyboard::os_arrow_left: case keyboard::os_arrow_up:
					ch_tabstop_next = false;
					break;
				case keyboard::os_arrow_right: case keyboard::os_arrow_down:
					ch_tabstop_next = true;
					break;
				default:
					return;
				}

				API::move_tabstop(*wdg_, ch_tabstop_next);
			}

			void trigger::_m_draw_title(graph_reference graph, bool enabled)
			{
				std::wstring str = to_wstring(wdg_->caption());

				nana::size ts = graph.text_extent_size(str);
				nana::size gsize = graph.size();

				nana::point pos{
					static_cast<int>(gsize.width - ts.width) >> 1, static_cast<int>(gsize.height - ts.height) >> 1
				};

				unsigned omitted_pixels = gsize.width;
				std::size_t txtlen = str.size();
				const auto txtptr = str.c_str();
				if (ts.width)
				{
					if (enabled)
					{
						auto text_color = element_state::pressed == attr_.e_state ? colors::black : colors::white;
						graph.palette(true, text_color);
					}
					else
					{
						graph.palette(true, color_rgb(0x7777777));
					}

#ifdef _nana_std_has_string_view
						graph.bidi_string(pos, { txtptr, txtlen });
#else
						graph.bidi_string(pos, txtptr, txtlen);
#endif
				}
			}

			void trigger::_m_draw_border(graph_reference graph, bool enabled)
			{
				nana::rectangle r(graph.size());

				auto pressed = element_state::pressed == attr_.e_state;
				auto color = enabled ? color_rgb(0xFFFFFF) : color_rgb(0xC1C3C2);

				graph.rectangle(r, pressed, color);
			}

			void trigger::_m_press(graph_reference graph, bool is_pressed)
			{
				if (is_pressed)
				{
					if (attr_.e_state == element_state::pressed)
						return;

					attr_.e_state = element_state::pressed;
					attr_.keep_pressed = true;
					wdg_->set_capture(true);
				}
				else
				{
					wdg_->release_capture();

					attr_.keep_pressed = false;

					if (element_state::pressed == attr_.e_state)
						attr_.e_state = element_state::hovered;
					else
						attr_.e_state = element_state::normal;

				}

				refresh(graph);
				API::dev::lazy_refresh();
			}

			void trigger::emit_click()
			{
				arg_click arg;
				arg.window_handle = wdg_->handle();
				arg.mouse_args = nullptr;
				API::emit_event(event_code::click, arg.window_handle, arg);
			}

		}
	}

	sdrbutton::sdrbutton() {}

	sdrbutton::sdrbutton(window wd, nana::point point, size_t width)
	{
		create(wd, rectangle(point.x, point.y, width, 16), true);
		API::effects_bground(*this, effects::bground_transparent(0), 0.0);
	}

	void sdrbutton::_m_complete_creation()
	{
		events().shortkey.connect_unignorable([this](const arg_keyboard&)
			{
				get_drawer_trigger().emit_click();
			});
	}

	void sdrbutton::_m_caption(native_string_type&& text)
	{
		API::unregister_shortkey(handle());

		wchar_t shortkey;
		API::transform_shortkey_text(to_utf8(text), shortkey, nullptr);
		if (shortkey)
			API::register_shortkey(handle(), shortkey);

		base_type::_m_caption(std::move(text));
	}
	
}
