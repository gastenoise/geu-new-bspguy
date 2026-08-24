#include "../GuiWidgetsCommon.h"

void Gui::drawLog()
{
	static bool AutoScroll = true;
	static bool scroll_to_bottom = false;

	ImGui::SetNextWindowSize(ImVec2(750.f, 300.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(200.f, 100.f), ImVec2(FLT_MAX, app->windowHeight - 40.f));
	if (!ImGui::Begin(fmt::format("{}###LOG_WIDGET", get_localized_string(LANG_1164)).c_str(), &showLogWidget))
	{
		ImGui::End();
		return;
	}

	static std::vector<std::string> log_buffer_copy;
	static std::vector<unsigned int> color_buffer_copy;

	{
		std::lock_guard<std::mutex> lock(Sync::LogConsole);
		bool logUpdated = log_buffer_copy.size() != g_log_buffer.size();
		if (logUpdated)
		{
			log_buffer_copy = g_log_buffer;
			color_buffer_copy = g_color_buffer;
			scroll_to_bottom = true;
		}
	}

	ImGui::BeginChild(get_localized_string(LANG_0706).c_str(), ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

	bool copy = false;
	if (ImGui::BeginPopupContextWindow())
	{
		if (ImGui::MenuItem(get_localized_string(LANG_1165).c_str()))
		{
			copy = true;
		}
		if (ImGui::MenuItem(get_localized_string(LANG_0707).c_str()))
		{
			std::lock_guard<std::mutex> lock(Sync::LogConsole);
			g_log_buffer.clear();
			g_color_buffer.clear();
		}
		if (ImGui::MenuItem(get_localized_string(LANG_0708).c_str(), NULL, &AutoScroll))
		{
		}
		ImGui::EndPopup();
	}

	ImGui::PushFont(consoleFont);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

	if (copy)
	{
		std::string logStr;
		for (const auto &str : log_buffer_copy)
		{
			logStr += str + "\n";
		}
		ImGui::SetClipboardText(logStr.c_str());
	}

	ImGuiListClipper clipper;
	clipper.Begin((int)log_buffer_copy.size());
	while (clipper.Step())
	{
		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, imguiColorFromConsole(color_buffer_copy[i]));
			ImGui::TextUnformatted(log_buffer_copy[i].c_str());
			ImGui::PopStyleColor();
		}
	}
	clipper.End();

	if (AutoScroll && scroll_to_bottom)
	{
		ImGui::SetScrollHereY(1.0f);
		scroll_to_bottom = false;
	}

	ImGui::PopFont();
	ImGui::PopStyleVar();

	ImGui::EndChild();
	ImGui::End();
}
