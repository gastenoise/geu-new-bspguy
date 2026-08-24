#include "GuiCommandPalette.h"
#include "ActionRegistry.h"
#include "../Gui.h"
#include "../Renderer.h"
#include "imgui.h"
#include "fmt/format.h"

GuiCommandPalette& GuiCommandPalette::getInstance()
{
	static GuiCommandPalette instance;
	return instance;
}

void GuiCommandPalette::open()
{
	isOpen = true;
	searchBuffer[0] = '\0';
	selectedIdx = 0;
	requestFocus = true;
}

void GuiCommandPalette::toggle()
{
	if (isOpen)
		close();
	else
		open();
}

void GuiCommandPalette::close()
{
	isOpen = false;
	searchBuffer[0] = '\0';
	selectedIdx = 0;
	requestFocus = false;
}

void GuiCommandPalette::draw(Gui* gui)
{
	if (!isOpen)
		return;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImVec2 center = viewport->GetCenter();
	float windowWidth = std::min(650.0f, viewport->Size.x - 40.0f);
	float windowHeight = std::min(450.0f, viewport->Size.y - 80.0f);

	ImGui::SetNextWindowPos(ImVec2(center.x, center.y - 100.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight), ImGuiCond_Always);

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
							 ImGuiWindowFlags_NoResize |
							 ImGuiWindowFlags_NoMove |
							 ImGuiWindowFlags_NoSavedSettings;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.13f, 0.96f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.30f, 0.50f, 0.90f, 0.80f));

	if (ImGui::Begin("##CommandPaletteWindow", &isOpen, flags))
	{
		// Search Input
		ImGui::PushItemWidth(-1);
		if (requestFocus)
		{
			ImGui::SetKeyboardFocusHere();
			requestFocus = false;
		}

		if (ImGui::InputTextWithHint("##CommandSearch", "Type a command or search action (Esc to close)...", searchBuffer, sizeof(searchBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			// Enter key in input box
			auto matches = ActionRegistry::getInstance().searchActions(searchBuffer);
			if (!matches.empty() && selectedIdx >= 0 && selectedIdx < (int)matches.size())
			{
				if (!matches[selectedIdx].isEnabled || matches[selectedIdx].isEnabled())
				{
					if (matches[selectedIdx].callback)
					{
						matches[selectedIdx].callback();
						close();
					}
				}
			}
		}
		ImGui::PopItemWidth();

		// Handle navigation keys
		if (ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			close();
		}

		auto matches = ActionRegistry::getInstance().searchActions(searchBuffer);

		if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
		{
			selectedIdx++;
			if (selectedIdx >= (int)matches.size())
				selectedIdx = 0;
		}
		if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
		{
			selectedIdx--;
			if (selectedIdx < 0)
				selectedIdx = matches.empty() ? 0 : (int)matches.size() - 1;
		}

		ImGui::Separator();

		// Action list child window
		if (ImGui::BeginChild("##CommandListChild", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar))
		{
			if (matches.empty())
			{
				ImGui::Spacing();
				ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No matching commands found.");
			}
			else
			{
				for (int i = 0; i < (int)matches.size(); i++)
				{
					const auto& action = matches[i];
					bool enabled = !action.isEnabled || action.isEnabled();
					bool isSelected = (i == selectedIdx);

					ImGui::PushID(i);

					if (isSelected)
					{
						ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.20f, 0.35f, 0.65f, 0.80f));
					}

					std::string badge = "[" + action.category + "]";
					std::string label = fmt::format("{:<12} {}", badge, action.title);

					if (!action.shortcut.empty())
					{
						label = fmt::format("{:<45} ({})", label, action.shortcut);
					}

					if (!enabled)
					{
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
					}

					if (ImGui::Selectable(label.c_str(), isSelected, 0, ImVec2(0, 24.0f)))
					{
						if (enabled && action.callback)
						{
							action.callback();
							close();
						}
					}

					if (!enabled)
					{
						ImGui::PopStyleColor();
					}

					if (isSelected)
					{
						ImGui::PopStyleColor();
						if (ImGui::IsWindowFocused() || ImGui::IsItemVisible())
						{
							ImGui::SetScrollHereY(0.5f);
						}
					}

					ImGui::PopID();
				}
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();

	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(3);
}
