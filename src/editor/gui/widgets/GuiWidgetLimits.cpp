#include "../GuiWidgetsCommon.h"

void Gui::drawLimits()
{
	ImGui::SetNextWindowSize(ImVec2(550.f, 630.f), ImGuiCond_FirstUseEver);

	if (limitsInvalidated && showLimitsWidget)
	{
		reloadLimits();
		limitsInvalidated = false;
	}

	Bsp* map = app->getSelectedMap();
	std::string title = map ? "Limits - " + map->bsp_name : "Limits";

	static Bsp* oldMap = NULL;

	if (map != oldMap)
	{
		reloadLimits();
		oldMap = map;
	}

	if (!map)
		return;

	BspRenderer* rend = map->getBspRender();
	if (!rend)
		return;

	if (ImGui::Begin(fmt::format("{}###LIMITS_WIDGET", title).c_str(), &showLimitsWidget))
	{
		if (!map)
		{
			ImGui::Text(get_localized_string(LANG_1123).c_str());
		}
		else
		{
			if (ImGui::BeginTabBar(get_localized_string(LANG_1166).c_str()))
			{
				if (ImGui::BeginTabItem(get_localized_string(LANG_0836).c_str()))
				{
					if (!loadedStats)
					{
						stats.clear();

						{
							std::lock_guard<std::mutex> lock(Sync::TexturesList);
							stats.emplace_back(calcStat("GL_TEXTURES", (unsigned int)g_all_Textures.size(), 0, false));
						}
						stats.emplace_back(calcStat("models", map->modelCount, g_limits.maxMapModels, false));
						stats.emplace_back(calcStat("planes", map->planeCount, map->is_bsp2 ? INT_MAX : MAX_MAP_PLANES, false));
						stats.emplace_back(calcStat("vertexes", map->vertCount, MAX_MAP_VERTS, false));
						stats.emplace_back(calcStat("nodes", map->nodeCount, map->is_bsp2 ? INT_MAX : (int)g_limits.maxMapNodes, false));
						stats.emplace_back(calcStat("texinfos", map->texinfoCount, map->is_bsp2 ? INT_MAX : MAX_MAP_TEXINFOS, false));
						stats.emplace_back(calcStat("faces", map->faceCount, map->is_bsp2 ? INT_MAX : MAX_MAP_FACES, false));
						stats.emplace_back(calcStat("clipnodes", map->clipnodeCount, map->is_32bit_clipnodes ? INT_MAX : g_limits.maxMapClipnodes, false));
						stats.emplace_back(calcStat("leaves", map->leafCount, map->is_bsp2 ? INT_MAX : g_limits.maxMapLeaves, false));
						stats.emplace_back(calcStat("marksurfaces", map->marksurfCount, map->is_bsp2 ? INT_MAX : MAX_MAP_MARKSURFS, false));
						stats.emplace_back(calcStat("surfedges", map->surfedgeCount, map->is_bsp2 ? INT_MAX : g_limits.maxMapSurfedges, false));
						stats.emplace_back(calcStat("edges", map->edgeCount, map->is_bsp2 ? INT_MAX : g_limits.maxMapEdges, false));
						stats.emplace_back(calcStat("textures", map->textureCount, g_limits.maxMapTextures, false));
						stats.emplace_back(calcStat("texturedata", map->textureDataLength, INT_MAX, true));
						stats.emplace_back(calcStat("lightdata", map->lightDataLength, g_limits.maxMapLightdata, true));
						stats.emplace_back(calcStat("visdata", map->visDataLength, g_limits.maxMapVisdata, true));
						stats.emplace_back(calcStat("entities", (unsigned int)map->ents.size(), g_limits.maxMapEnts, false));
						loadedStats = true;
					}

					ImGui::BeginChild(get_localized_string(LANG_0837).c_str());
					ImGui::Dummy(ImVec2(0, 10));
					ImGui::PushFont(consoleFontLarge);

					float midWidth = consoleFontLarge->CalcTextSizeA(fontSize * 1.1f, FLT_MAX, FLT_MAX, "    Current / Max    ").x;
					float otherWidth = (ImGui::GetWindowWidth() - midWidth) / 2;
					ImGui::Columns(3);
					ImGui::SetColumnWidth(0, otherWidth);
					ImGui::SetColumnWidth(1, midWidth);
					ImGui::SetColumnWidth(2, otherWidth);

					ImGui::Text(get_localized_string(LANG_0838).c_str());
					ImGui::NextColumn();
					ImGui::Text(get_localized_string(LANG_0839).c_str());
					ImGui::NextColumn();
					ImGui::Text(get_localized_string(LANG_0840).c_str());
					ImGui::NextColumn();

					ImGui::Columns(1);
					ImGui::Separator();
					ImGui::BeginChild(get_localized_string(LANG_0841).c_str());
					ImGui::Columns(3);
					ImGui::SetColumnWidth(0, otherWidth);
					ImGui::SetColumnWidth(1, midWidth);
					ImGui::SetColumnWidth(2, otherWidth);

					for (size_t i = 0; i < stats.size(); i++)
					{
						ImGui::TextColored(stats[i].color, stats[i].name.c_str());
						ImGui::NextColumn();

						std::string val = stats[i].val + " / " + stats[i].max;
						ImGui::TextColored(stats[i].color, val.c_str());
						ImGui::NextColumn();

						ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.5f, 0.4f, 0, 1));
						ImGui::ProgressBar(stats[i].progress, ImVec2(-1, 0), stats[i].fullness.c_str());
						ImGui::PopStyleColor(1);
						ImGui::NextColumn();
					}

					ImGui::Columns(1);
					ImGui::EndChild();
					ImGui::PopFont();
					drawUndoMemUsage(rend);

					ImGui::EndChild();

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem(get_localized_string(LANG_1177).c_str()))
				{
					drawLimitTab(map, SORT_CLIPNODES);
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem(get_localized_string(LANG_0842).c_str()))
				{
					drawLimitTab(map, SORT_NODES);
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem(get_localized_string(LANG_0843).c_str()))
				{
					drawLimitTab(map, SORT_FACES);
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem(get_localized_string(LANG_0844).c_str()))
				{
					drawLimitTab(map, SORT_VERTS);
					ImGui::EndTabItem();
				}
			}

			ImGui::EndTabBar();
		}
	}

	ImGui::End();
}

void Gui::drawUndoMemUsage(BspRenderer* rend)
{
	ImGui::SeparatorText((get_localized_string(LANG_0721) + " " + std::to_string(rend->undoHistory.size())).c_str());
	float mb = rend->undoMemoryUsage / (1024.0f * 1024.0f);
	float mb_zip = rend->undoMemoryUsageZip / (1024.0f * 1024.0f);
	ImGui::Text(get_localized_string("UNDO_MEM_USAGE").c_str(), mb, mb_zip);
}

void Gui::drawLimitTab(Bsp* map, int sortMode)
{
	int maxCount = 0;
	const char* countName = "None";
	switch (sortMode)
	{
		case SORT_VERTS:
			maxCount = map->vertCount;
			countName = "Vertexes";
			break;
		case SORT_NODES:
			maxCount = map->nodeCount;
			countName = "Nodes";
			break;
		case SORT_CLIPNODES:
			maxCount = map->clipnodeCount;
			countName = "Clipnodes";
			break;
		case SORT_FACES:
			maxCount = map->faceCount;
			countName = "Faces";
			break;
	}

	if (!loadedLimit[sortMode])
	{
		std::vector<STRUCTUSAGE*> modelInfos = map->get_sorted_model_infos(sortMode);

		limitModels[sortMode].clear();
		for (size_t i = 0; i < modelInfos.size(); i++)
		{
			int val = 0;

			switch (sortMode)
			{
				case SORT_VERTS:
					val = modelInfos[i]->sum.verts;
					break;
				case SORT_NODES:
					val = modelInfos[i]->sum.nodes;
					break;
				case SORT_CLIPNODES:
					val = modelInfos[i]->sum.clipnodes;
					break;
				case SORT_FACES:
					val = modelInfos[i]->sum.faces;
					break;
			}

			ModelInfo stat = calcModelStat(map, modelInfos[i], val, maxCount, false);
			limitModels[sortMode].push_back(stat);
			delete modelInfos[i];
		}
		loadedLimit[sortMode] = true;
	}
	std::vector<ModelInfo>& modelInfos = limitModels[sortMode];

	ImGui::BeginChild(get_localized_string(LANG_1124).c_str());
	ImGui::Dummy(ImVec2(0, 10));
	ImGui::PushFont(consoleFontLarge);

	float valWidth = consoleFontLarge->CalcTextSizeA(fontSize * 1.2f, FLT_MAX, FLT_MAX, " Clipnodes ").x;
	float usageWidth = consoleFontLarge->CalcTextSizeA(fontSize * 1.2f, FLT_MAX, FLT_MAX, "  Usage   ").x;
	float modelWidth = consoleFontLarge->CalcTextSizeA(fontSize * 1.2f, FLT_MAX, FLT_MAX, " Model ").x;
	float bigWidth = ImGui::GetWindowWidth() - (valWidth + usageWidth + modelWidth);
	ImGui::Columns(4);
	ImGui::SetColumnWidth(0, bigWidth);
	ImGui::SetColumnWidth(1, modelWidth);
	ImGui::SetColumnWidth(2, valWidth);
	ImGui::SetColumnWidth(3, usageWidth);

	ImGui::Text(get_localized_string(LANG_0845).c_str());
	ImGui::NextColumn();
	ImGui::Text(get_localized_string(LANG_0846).c_str());
	ImGui::NextColumn();
	ImGui::Text(countName);
	ImGui::NextColumn();
	ImGui::Text(get_localized_string(LANG_0847).c_str());
	ImGui::NextColumn();

	ImGui::Columns(1);
	ImGui::Separator();
	ImGui::BeginChild(get_localized_string(LANG_1125).c_str());
	ImGui::Columns(4);
	ImGui::SetColumnWidth(0, bigWidth);
	ImGui::SetColumnWidth(1, modelWidth);
	ImGui::SetColumnWidth(2, valWidth);
	ImGui::SetColumnWidth(3, usageWidth);

	for (size_t i = 0; i < limitModels[sortMode].size(); i++)
	{
		if (modelInfos[i].val == "0")
		{
			break;
		}

		std::string cname = modelInfos[i].classname + "##" + "select" + std::to_string(i);
		int flags = ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SpanAllColumns;
		if (ImGui::Selectable(cname.c_str(), app->pickInfo.IsSelectedEnt(modelInfos[i].entIdx), flags))
		{
			int entIdx = modelInfos[i].entIdx;
			if ((size_t)entIdx < map->ents.size())
			{
				app->pickInfo.SetSelectedEnt(entIdx);
				// map should already be valid if limits are showing

				if (ImGui::IsMouseDoubleClicked(0))
				{
					app->goToEnt(map, (int)entIdx);
				}
			}
		}
		ImGui::NextColumn();

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(modelInfos[i].model.c_str()).x - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
		ImGui::Text(modelInfos[i].model.c_str());
		ImGui::NextColumn();

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(modelInfos[i].val.c_str()).x - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
		ImGui::Text(modelInfos[i].val.c_str());
		ImGui::NextColumn();

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(modelInfos[i].usage.c_str()).x - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
		ImGui::Text(modelInfos[i].usage.c_str());
		ImGui::NextColumn();
	}

	ImGui::Columns(1);
	ImGui::EndChild();

	ImGui::PopFont();
	ImGui::EndChild();
}
