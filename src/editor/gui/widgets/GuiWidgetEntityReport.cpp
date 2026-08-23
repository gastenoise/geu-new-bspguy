#include "../GuiWidgetsCommon.h"
#include <regex>

void Gui::drawEntityReport()
{
	ImGui::SetNextWindowSize(ImVec2(1200.f, 630.f), ImGuiCond_FirstUseEver);
	Bsp* map = app->getSelectedMap();

	std::string title = map ? "Entity Report - " + map->bsp_name : "Entity Report";

	if (ImGui::Begin(fmt::format("{}###ENTITY_WIDGET", title).c_str(), &showEntityReport))
	{
		if (!map)
		{
			ImGui::Text(get_localized_string(LANG_1167).c_str());
		}
		else
		{
			static float startFrom = 0.0f;
			static int MAX_FILTERS = 1;
			static std::vector<std::string> keyFilter = {""};
			static std::vector<std::string> valueFilter = {""};
			static std::vector<int> opFilter = {0}; // 0: =, 1: !=
			static std::vector<int> logicFilter = std::vector<int>(); // 0: AND, 1: OR
			static int lastSelect = -1;
			static std::vector<int> visibleEnts;
			static std::vector<bool> selectedItems;
			static bool selectAllItems = false;

			ImGui::BeginGroup();
			ImGui::BeginChild(get_localized_string(LANG_0848).c_str(), ImVec2(0.f, 500.f));

			bool criteriaChanged = false;
			if (MAX_FILTERS != lastMAX_FILTERS)
			{
				criteriaChanged = true;
			}
			else
			{
				for (int i = 0; i < MAX_FILTERS; i++)
				{
					if (keyFilter[i] != lastKeyFilters[i] ||
						valueFilter[i] != lastValueFilters[i] ||
						opFilter[i] != lastOpFilters[i] ||
						(i < MAX_FILTERS - 1 && logicFilter[i] != lastLogicFilters[i]))
					{
						criteriaChanged = true;
						break;
					}
				}
			}

			if (entityListChanged || criteriaChanged)
			{
				visibleEnts.clear();
				for (size_t i = 0; i < map->ents.size(); i++)
				{
					bool allMatch = false;

					for (int f = 0; f < MAX_FILTERS; f++)
					{
						bool currentFilterMatch = false;

						std::string key = keyFilter[f];
						std::string val = valueFilter[f];

						bool hasKey = map->ents[i]->hasKey(key);
						std::string entVal = hasKey ? map->ents[i]->keyvalues[key] : "";

						if (opFilter[f] == 0) // exact match
						{
							if (key.empty() && val.empty())
							{
								currentFilterMatch = true;
							}
							else if (key.empty())
							{
								for (const auto& kv : map->ents[i]->keyvalues)
								{
									if (kv.second == val)
									{
										currentFilterMatch = true;
										break;
									}
								}
							}
							else if (val.empty())
							{
								currentFilterMatch = hasKey;
							}
							else
							{
								currentFilterMatch = (entVal == val);
							}
						}
						else if (opFilter[f] == 1) // !=
						{
							if (key.empty() && val.empty())
							{
								currentFilterMatch = false;
							}
							else if (key.empty())
							{
								currentFilterMatch = true;
								for (const auto& kv : map->ents[i]->keyvalues)
								{
									if (kv.second == val)
									{
										currentFilterMatch = false;
										break;
									}
								}
							}
							else if (val.empty())
							{
								currentFilterMatch = !hasKey;
							}
							else
							{
								currentFilterMatch = (entVal != val);
							}
						}
						else if (opFilter[f] == 2) // substring / regex
						{
							if (key.empty() && val.empty())
							{
								currentFilterMatch = true;
							}
							else if (key.empty())
							{
								for (const auto& kv : map->ents[i]->keyvalues)
								{
									if (kv.second.find(val) != std::string::npos)
									{
										currentFilterMatch = true;
										break;
									}
								}
							}
							else if (val.empty())
							{
								currentFilterMatch = hasKey;
							}
							else
							{
								currentFilterMatch = (entVal.find(val) != std::string::npos);
							}
						}

						if (f == 0)
						{
							allMatch = currentFilterMatch;
						}
						else
						{
							if (logicFilter[f - 1] == 0) // AND
								allMatch = allMatch && currentFilterMatch;
							else // OR
								allMatch = allMatch || currentFilterMatch;
						}
					}

					if (allMatch)
					{
						visibleEnts.push_back((int)i);
					}
				}

				selectedItems.resize(visibleEnts.size(), false);
				lastMAX_FILTERS = MAX_FILTERS;
				lastKeyFilters = keyFilter;
				lastValueFilters = valueFilter;
				lastOpFilters = opFilter;
				lastLogicFilters = logicFilter;
				entityListChanged = false;
			}

			// Table rendering
			if (ImGui::BeginTable("##EntityReportTable", 4, ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders))
			{
				ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 40.0f);
				ImGui::TableSetupColumn("Classname", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Targetname", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Origin / Model", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableHeadersRow();

				ImGuiListClipper clipper;
				clipper.Begin((int)visibleEnts.size());

				while (clipper.Step())
				{
					for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
					{
						int entIdx = visibleEnts[row];
						Entity* ent = map->ents[entIdx];

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);

						bool isSelected = app->pickInfo.IsSelectedEnt(entIdx);
						std::string rowLabel = fmt::format("{}##row{}", entIdx, row);

						if (ImGui::Selectable(rowLabel.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
						{
							if (ImGui::GetIO().KeyCtrl)
							{
								if (isSelected)
									app->pickInfo.DelSelectedEnt(entIdx);
								else
									app->pickInfo.AddSelectedEnt(entIdx);
							}
							else
							{
								app->pickInfo.SetSelectedEnt(entIdx);
							}

							if (ImGui::IsMouseDoubleClicked(0))
							{
								app->goToEnt(map, entIdx);
							}
						}

						ImGui::TableSetColumnIndex(1);
						ImGui::TextUnformatted(ent->hasKey("classname") ? ent->keyvalues["classname"].c_str() : "unknown");

						ImGui::TableSetColumnIndex(2);
						ImGui::TextUnformatted(ent->hasKey("targetname") ? ent->keyvalues["targetname"].c_str() : "-");

						ImGui::TableSetColumnIndex(3);
						if (ent->isBspModel())
						{
							ImGui::Text("*%d", ent->getBspModelIdx());
						}
						else if (ent->hasKey("origin"))
						{
							ImGui::TextUnformatted(ent->keyvalues["origin"].c_str());
						}
						else
						{
							ImGui::TextUnformatted("-");
						}
					}
				}
				clipper.End();
				ImGui::EndTable();
			}

			ImGui::EndChild();

			// Bottom filter bar
			ImGui::Text("%zu matching entities shown.", visibleEnts.size());
			ImGui::SameLine();
			if (ImGui::Button("Add Filter"))
			{
				MAX_FILTERS++;
				keyFilter.push_back("");
				valueFilter.push_back("");
				opFilter.push_back(0);
				logicFilter.push_back(0);
			}
			if (MAX_FILTERS > 1)
			{
				ImGui::SameLine();
				if (ImGui::Button("Remove Filter"))
				{
					MAX_FILTERS--;
					keyFilter.pop_back();
					valueFilter.pop_back();
					opFilter.pop_back();
					logicFilter.pop_back();
				}
			}

			for (int f = 0; f < MAX_FILTERS; f++)
			{
				ImGui::PushID(f);
				if (f > 0)
				{
					ImGui::Combo("##logic", &logicFilter[f - 1], "AND\0OR\0");
					ImGui::SameLine();
				}
				ImGui::SetNextItemWidth(120.0f);
				ImGui::InputTextWithHint("##key", "Key (e.g. classname)", &keyFilter[f]);
				ImGui::SameLine();
				ImGui::SetNextItemWidth(100.0f);
				ImGui::Combo("##op", &opFilter[f], "Equals\0Not Equals\0Contains\0");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(180.0f);
				ImGui::InputTextWithHint("##val", "Value (e.g. info_player_start)", &valueFilter[f]);
				ImGui::PopID();
			}

			ImGui::EndGroup();
		}
	}
	ImGui::End();
}
