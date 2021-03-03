#pragma once

#include <nana/gui/widgets/widget.hpp>
#include <nana/gui/element.hpp>
#include <nana/push_ignore_diagnostic>


namespace nana {
	namespace drawerbase
	{
		namespace sdrbutton
		{
			class trigger : public drawer_trigger
			{
				class measurer;
			public:
				trigger();
				~trigger();

				void emit_click();
			private:
				void attached(widget_reference, graph_reference) override;
				void refresh(graph_reference)	override;
				void mouse_enter(graph_reference, const arg_mouse&) override;
				void mouse_leave(graph_reference, const arg_mouse&) override;
				void mouse_down(graph_reference, const arg_mouse&)	override;
				void mouse_up(graph_reference, const arg_mouse&)	override;
				void key_press(graph_reference, const arg_keyboard&) override;
			private:
				void _m_draw_title(graph_reference, bool enabled);
				void _m_draw_border(graph_reference, bool enabled);
				void _m_press(graph_reference, bool);
			private:
				widget* wdg_{ nullptr };
				paint::graphics* graph_{ nullptr };
				std::unique_ptr<measurer> measurer_;

				struct attr_tag
				{
					element_state e_state;
					bool keep_pressed;
				} attr_;
			};
		}
	}
			
	class sdrbutton : public widget_object<category::widget_tag, drawerbase::sdrbutton::trigger>
	{
		typedef widget_object<category::widget_tag, drawerbase::sdrbutton::trigger> base_type;
	public:
		sdrbutton();
		sdrbutton(window parent, nana::point point, size_t width);
	private:
		void _m_complete_creation() override;
		void _m_caption(native_string_type&&) override;
	};
}


