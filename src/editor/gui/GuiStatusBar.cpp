#include "../Gui.h"
#include "../Renderer.h"
#include "../BspRenderer.h"
#include "bsp/Bsp.h"
#include "../Settings.h"
#include "lang.h"
#include "fmt/format.h"

extern Settings g_settings;
extern Renderer* g_app;

void Gui::drawStatusBar()
{
	if (!app)
		return;

	Bsp* map = app->getSelectedMap();
	BspRenderer* rend = map ? map->getBspRender() : nullptr;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	float statusBarHeight = ImGui::GetFrameHeight() + 6.0f;

	ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - statusBarHeight));
	ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, statusBarHeight));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoSavedSettings;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 3.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.10f, 0.92f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.20f, 0.20f, 0.25f, 0.70f));

	if (ImGui::Begin("##GuiStatusBar", nullptr, flags))
	{
		if (ImGui::BeginTable("##StatusBarTable", 3, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoBordersInBody))
		{
			ImGui::TableSetupColumn("Selection", ImGuiTableColumnFlags_WidthStretch, 0.45f);
			ImGui::TableSetupColumn("Navigation", ImGuiTableColumnFlags_WidthStretch, 0.35f);
			ImGui::TableSetupColumn("System", ImGuiTableColumnFlags_WidthStretch, 0.20f);

			ImGui::TableNextRow();

			// Column 1: Selection Info
			ImGui::TableSetColumnIndex(0);
			if (!map)
			{
				ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No map loaded");
			}
			else if (app->pickMode == PICK_OBJECT)
			{
				if (app->pickInfo.selectedEnts.empty())
				{
					ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Mode: Objects | Ready");
				}
				else if (app->pickInfo.selectedEnts.size() == 1)
				{
					int entIdx = app->pickInfo.selectedEnts[0];
					if (entIdx >= 0 && entIdx < (int)map->ents.size())
					{
						Entity* ent = map->ents[entIdx];
						std::string cname = ent->hasKey("classname") ? ent->keyvalues["classname"] : "unknown";
						std::string tname = ent->hasKey("targetname") ? (" (\"" + ent->keyvalues["targetname"] + "\")") : "";
						int mdlIdx = ent->getBspModelIdx();

						if (mdlIdx > 0 && mdlIdx < map->modelCount)
						{
							vec3 mins, maxs;
							map->get_bounding_box(mdlIdx, mins, maxs);
							vec3 size = maxs - mins;
							ImGui::Text("Ent #%d: %s%s | Model *%d | Size: %.0fx%.0fx%.0f",
								entIdx, cname.c_str(), tname.c_str(), mdlIdx, size.x, size.y, size.z);
						}
						else
						{
							vec3 origin = ent->hasKey("origin") ? parseVector(ent->keyvalues["origin"]) : vec3();
							ImGui::Text("Ent #%d: %s%s | Pos: (%.0f, %.0f, %.0f)",
								entIdx, cname.c_str(), tname.c_str(), origin.x, origin.y, origin.z);
						}
					}
				}
				else
				{
					ImGui::Text("%zu Entities Selected", app->pickInfo.selectedEnts.size());
				}
			}
			else // PICK_FACE or PICK_FACE_LEAF
			{
				if (app->pickInfo.selectedFaces.empty())
				{
					ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s | Ready",
						app->pickMode == PICK_FACE ? "Mode: Faces" : "Mode: Leaf Faces");
				}
				else if (app->pickInfo.selectedFaces.size() == 1)
				{
					int faceIdx = app->pickInfo.selectedFaces[0];
					if (faceIdx >= 0 && faceIdx < map->faceCount)
					{
						BSPFACE32& face = map->faces[faceIdx];
						std::string texName = "unknown";
						if (face.iTextureInfo < map->texinfoCount)
						{
							BSPTEXTUREINFO& info = map->texinfos[face.iTextureInfo];
							if (info.iMiptex >= 0 && info.iMiptex < map->textureCount)
							{
								int texOffset = ((int*)map->textures)[info.iMiptex + 1];
								if (texOffset >= 0)
								{
									BSPMIPTEX& tex = *((BSPMIPTEX*)(map->textures + texOffset));
									texName = fmt::format("{} ({}x{})", tex.szName, tex.nWidth, tex.nHeight);
								}
							}
						}
						ImGui::Text("Face #%d | Tex: %s | Plane #%d", faceIdx, texName.c_str(), face.iPlane);
					}
				}
				else
				{
					ImGui::Text("%zu Faces Selected", app->pickInfo.selectedFaces.size());
				}
			}

			// Column 2: Navigation & Spatial
			ImGui::TableSetColumnIndex(1);
			if (rend)
			{
				vec3 hlAngles = cameraAngles;
				hlAngles = hlAngles.unflipUV();
				hlAngles = hlAngles.normalize_angles();
				hlAngles.y -= 90.0f;

				std::string leafStr = rend->curLeafIdx >= 0 ? fmt::format("Leaf: {}", rend->curLeafIdx) : "Leaf: -";

				ImGui::Text("Hit: [%.0f, %.0f, %.0f] | Ang: [%.0f, %.0f, %.0f] | %s",
					floatRound(rend->intersectVec.x), floatRound(rend->intersectVec.y), floatRound(rend->intersectVec.z),
					floatRound(hlAngles.x), floatRound(hlAngles.y), floatRound(hlAngles.z),
					leafStr.c_str());
			}

			// Column 3: Grid Snapping & System Status
			ImGui::TableSetColumnIndex(2);

			int snapGridVal = (app->gridSnapLevel >= 0 && app->gridSnapLevel <= 10) ? (1 << app->gridSnapLevel) : 0;
			std::string gridLabel = app->gridSnappingEnabled ? fmt::format("Snap: {}u", snapGridVal) : "Snap: OFF";

			if (ImGui::SmallButton(gridLabel.c_str()))
			{
				if (!app->gridSnappingEnabled)
				{
					app->gridSnappingEnabled = true;
					if (app->gridSnapLevel < 0)
						app->gridSnapLevel = 3; // default 8 units
				}
				else
				{
					app->gridSnapLevel++;
					if (app->gridSnapLevel > 9) // wrap up to 512 then off
					{
						app->gridSnappingEnabled = false;
						app->gridSnapLevel = 0;
					}
				}
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Click to cycle grid snap size: 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, OFF");
			}

			ImGui::SameLine();
			if (app->isLoading)
			{
				ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Loading...");
			}
			else
			{
				ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Ready");
			}

			ImGui::SameLine();
			if (imgui_io)
			{
				ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "| %.0f FPS", imgui_io->Framerate);
			}

			ImGui::EndTable();
		}
	}
	ImGui::End();

	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(2);
}
