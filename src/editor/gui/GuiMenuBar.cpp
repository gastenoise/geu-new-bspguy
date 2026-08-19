#include "../Gui.h"
#include "../Renderer.h"
#include "../BspRenderer.h"
#include "bsp/Bsp.h"
#include "../Settings.h"
#include "lang.h"
#include "filedialog/ImFileDialog.h"
#include "imgui_stdlib.h"
#include "util.h"
#include "log.h"
#include "fmt/format.h"
#include <filesystem>
#include <algorithm>
#include <cmath>

extern float g_tooltip_delay;
extern std::string g_working_dir;
extern Settings g_settings;
extern Renderer* g_app;
extern int pickCount;
extern std::string g_game_dir;
extern bool g_console_visible;
extern std::vector<BspRenderer*> mapRenderers;

void Gui::drawMenuBar()
{
	ImGuiContext& g = *GImGui;
	static bool ditheringEnabled = false;
	Bsp* map = app->getSelectedMap();

	if (ImGui::BeginMainMenuBar())
	{
		if (ifd::FileDialog::Instance().IsDone("PngDirOpenDialog"))
		{
			if (ifd::FileDialog::Instance().HasResult())
			{
				std::filesystem::path res = ifd::FileDialog::Instance().GetResult();
				std::string png_import_dir = stripFileName(res.string());
				g_settings.lastdir = stripFileName(res.string());
				if (dirExists(png_import_dir))
				{
					createDir(g_working_dir + "temp/");
					std::string tempWad = g_working_dir + "temp/temp2.wad";
					removeFile(tempWad);
					if (map && map->import_textures_to_wad(tempWad, png_import_dir, ditheringEnabled))
					{
						map->ImportWad(tempWad);
					}
					removeFile(tempWad);
				}
			}
			ifd::FileDialog::Instance().Close();
		}

		if (ifd::FileDialog::Instance().IsDone("WadOpenDialog"))
		{
			if (ifd::FileDialog::Instance().HasResult())
			{
				std::filesystem::path res = ifd::FileDialog::Instance().GetResult();
				if (fileExists(res.string()))
				{
					if (!map)
					{
						app->addMap(new Bsp(""));
						app->selectMapId(0);
						map = app->getSelectedMap();
					}

					g_settings.AddRecentFile(res.string());
					for (size_t i = 0; i < map->ents.size(); i++)
					{
						if (map->ents[i]->keyvalues["classname"] == "worldspawn")
						{
							std::vector<std::string> wadNames = splitString(map->ents[i]->keyvalues["wad"], ";");
							std::string newWadNames;
							for (size_t k = 0; k < wadNames.size(); k++)
							{
								if (wadNames[k].find(res.filename().string()) == std::string::npos)
									newWadNames += wadNames[k] + ";";
							}
							map->ents[i]->setOrAddKeyvalue("wad", newWadNames);
							break;
						}
					}
					app->updateEnts();
					map->ImportWad(res.string());
					app->reloadBspModels();
					g_settings.lastdir = stripFileName(res.string());
				}
			}
			ifd::FileDialog::Instance().Close();
		}

		if (ifd::FileDialog::Instance().IsDone("MapOpenDialog"))
		{
			if (ifd::FileDialog::Instance().HasResult())
			{
				std::filesystem::path res = ifd::FileDialog::Instance().GetResult();
				if (fileExists(res.string()))
				{
					g_settings.AddRecentFile(res.string());
					OpenFile(res.string());
					g_settings.lastdir = stripFileName(res.string());
				}
			}
			ifd::FileDialog::Instance().Close();
		}

		drawMenu_File();
		drawMenu_Edit();
		drawMenu_View();
		drawMenu_Map();
		drawMenu_Tools();
		drawMenu_Create();
		drawMenu_Windows();
		drawMenu_Help();
		if (showDebugWidget)
		{
			drawMenu_Debug();
		}

		ImGui::EndMainMenuBar();
	}
}

void Gui::drawMenu_File()
{
	Bsp* map = app->getSelectedMap();

	if (ImGui::BeginMenu(get_localized_string(LANG_0478).c_str()))
	{
		if (ImGui::MenuItem(get_localized_string(LANG_0479).c_str(), NULL, false, map && !map->is_mdl_model && !app->isLoading))
		{
			map->update_ent_lump();
			map->update_lump_pointers();
			map->validate();
			map->write(map->bsp_path);
		}

		if (ImGui::BeginMenu(get_localized_string(LANG_0480).c_str(), map && !map->is_mdl_model && !app->isLoading))
		{
			bool old_is_bsp30ext = map->is_bsp30ext;
			bool old_is_bsp2 = map->is_bsp2;
			bool old_is_bsp2_old = map->is_bsp2_old;
			bool old_is_bsp29 = map->is_bsp29;
			bool old_is_32bit_clipnodes = map->is_32bit_clipnodes;
			bool old_is_broken_clipnodes = map->is_broken_clipnodes;
			bool old_is_blue_shift = map->is_blue_shift;
			bool old_is_colored_lightmap = map->is_colored_lightmap;
			int old_bsp_version = map->bsp_header.nVersion;

			bool is_default_format = !old_is_bsp30ext && !old_is_bsp2 &&
				!old_is_bsp2_old && !old_is_bsp29 && !old_is_32bit_clipnodes && !old_is_broken_clipnodes
				&& !old_is_blue_shift && old_is_colored_lightmap && old_bsp_version == 30;

			if (ImGui::MenuItem(get_localized_string(LANG_0481).c_str(), NULL, is_default_format && map->is_texture_has_pal))
			{
				if (map->isValid())
				{
					map->update_ent_lump();
					map->update_lump_pointers();
					map->is_bsp30ext = false;
					map->is_bsp2 = false;
					map->is_bsp2_old = false;
					map->is_bsp29 = false;
					map->is_32bit_clipnodes = false;
					map->is_broken_clipnodes = false;
					map->is_blue_shift = false;
					map->is_colored_lightmap = true;
					map->target_save_texture_has_pal = true;
					map->bsp_header.nVersion = 30;

					if (map->validate() && map->isValid())
					{
						map->write(map->bsp_path);
					}
				}
			}

			if (ImGui::MenuItem((get_localized_string(LANG_0481) + "[NO PALETTE]").c_str(), NULL, is_default_format && !map->is_texture_has_pal))
			{
				if (map->isValid())
				{
					map->update_ent_lump();
					map->update_lump_pointers();
					map->is_bsp30ext = false;
					map->is_bsp2 = false;
					map->is_bsp2_old = false;
					map->is_bsp29 = false;
					map->is_32bit_clipnodes = false;
					map->is_broken_clipnodes = false;
					map->is_blue_shift = false;
					map->is_colored_lightmap = true;
					map->target_save_texture_has_pal = true;
					map->bsp_header.nVersion = 30;

					if (map->validate() && map->isValid())
					{
						map->write(map->bsp_path);
					}
				}
			}

			if (ImGui::MenuItem(get_localized_string(LANG_0485).c_str(), NULL, old_is_blue_shift))
			{
				if (map->isValid())
				{
					map->update_ent_lump();
					map->update_lump_pointers();
					map->is_bsp30ext = false;
					map->is_bsp2 = false;
					map->is_bsp2_old = false;
					map->is_bsp29 = false;
					map->is_32bit_clipnodes = false;
					map->is_broken_clipnodes = false;
					map->is_blue_shift = true;
					map->is_colored_lightmap = true;
					map->target_save_texture_has_pal = true;
					map->bsp_header.nVersion = 30;

					if (map->validate() && map->isValid())
					{
						map->write(map->bsp_path);
					}
				}
			}

			if (ImGui::MenuItem(get_localized_string(LANG_0489).c_str(), NULL, old_is_bsp29 && !old_is_broken_clipnodes && old_is_colored_lightmap))
			{
				if (map->isValid())
				{
					map->update_ent_lump();
					map->update_lump_pointers();
					map->is_bsp30ext = false;
					map->is_bsp2 = false;
					map->is_bsp2_old = false;
					map->is_bsp29 = true;
					map->is_32bit_clipnodes = false;
					map->is_broken_clipnodes = false;
					map->is_blue_shift = false;
					map->is_colored_lightmap = true;
					map->target_save_texture_has_pal = false;
					map->bsp_header.nVersion = 29;

					if (map->validate() && map->isValid())
					{
						map->write(map->bsp_path);
					}
				}
			}

			if (ImGui::MenuItem(get_localized_string(LANG_0493).c_str(), NULL, old_is_bsp29 && !old_is_broken_clipnodes && !old_is_colored_lightmap))
			{
				if (map->isValid())
				{
					map->update_ent_lump();
					map->update_lump_pointers();
					map->is_bsp30ext = false;
					map->is_bsp2 = false;
					map->is_bsp2_old = false;
					map->is_bsp29 = true;
					map->is_32bit_clipnodes = false;
					map->is_broken_clipnodes = false;
					map->is_blue_shift = false;
					map->is_colored_lightmap = false;
					map->target_save_texture_has_pal = false;
					map->bsp_header.nVersion = 29;

					if (map->validate() && map->isValid())
					{
						map->write(map->bsp_path);
					}
				}
			}

			if (ImGui::MenuItem(get_localized_string(LANG_0505).c_str(), NULL, old_is_bsp2 && !old_is_bsp2_old && old_is_colored_lightmap))
			{
				if (map->isValid())
				{
					map->update_ent_lump();
					map->update_lump_pointers();
					map->is_bsp30ext = false;
					map->is_bsp2 = true;
					map->is_bsp2_old = false;
					map->is_bsp29 = false;
					map->is_32bit_clipnodes = true;
					map->is_broken_clipnodes = false;
					map->is_blue_shift = false;
					map->is_colored_lightmap = true;
					map->target_save_texture_has_pal = false;
					map->bsp_header.nVersion = 30;

					if (map->validate() && map->isValid())
					{
						map->write(map->bsp_path);
					}
				}
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(get_localized_string(LANG_0523).c_str()))
		{
			if (ImGui::MenuItem(get_localized_string(LANG_0524).c_str()))
			{
				ifd::FileDialog::Instance().Open("MapOpenDialog", get_localized_string(LANG_0524), "BSP map file (*.bsp;*.map;*.mdl;*.spr;*.csm){.bsp,.map,.mdl,.spr,.csm},.*", false, g_settings.lastdir);
			}
			if (ImGui::MenuItem(get_localized_string(LANG_0526).c_str()))
			{
				ifd::FileDialog::Instance().Open("MapOpenDialog", get_localized_string(LANG_0527), "Studio Model (*.mdl){.mdl},.*", false, g_settings.lastdir);
			}
			if (ImGui::MenuItem(get_localized_string("OPEN_SPR_VIEW").c_str()))
			{
				ifd::FileDialog::Instance().Open("MapOpenDialog", get_localized_string("OPEN_SPR_VIEW"), "Sprite (*.spr){.spr},.*", false, g_settings.lastdir);
			}
			if (ImGui::MenuItem(get_localized_string("OPEN_XASHNT_CSM_VIEW").c_str()))
			{
				ifd::FileDialog::Instance().Open("MapOpenDialog", get_localized_string("OPEN_XASHNT_CSM_VIEW"), "XashNT Model (*.csm){.csm},.*", false, g_settings.lastdir);
			}
			if (ImGui::MenuItem(get_localized_string(LANG_0528).c_str()))
			{
				ifd::FileDialog::Instance().Open("WadOpenDialog", get_localized_string(LANG_0529), "Wad file (*.wad){.wad},.*", false, g_settings.lastdir);
			}
			ImGui::EndMenu();
		}

		if (ImGui::MenuItem(get_localized_string(LANG_0530).c_str(), NULL, false, !app->isLoading && map))
		{
			app->deselectMap();
		}
		if (ImGui::MenuItem(get_localized_string(LANG_0531).c_str(), NULL, false, !app->isLoading))
		{
			app->clearMaps();
		}

		if (ImGui::BeginMenu(get_localized_string(LANG_0532).c_str(), !app->isLoading && map))
		{
			if (ImGui::MenuItem(get_localized_string(LANG_0533).c_str(), NULL, false, map && !map->is_mdl_model))
			{
				std::string path = map->bsp_path.substr(0, map->bsp_path.find_last_of(".")) + ".ent";
				map->export_entities(path);
			}
			if (ImGui::MenuItem(get_localized_string(LANG_0534).c_str(), NULL, false, map && !map->is_mdl_model))
			{
				std::string path = map->bsp_path.substr(0, map->bsp_path.find_last_of(".")) + ".wad";
				map->export_wad_to_pngs(path, stripFileName(path));
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(get_localized_string(LANG_0543).c_str(), !app->isLoading))
		{
			if (ImGui::MenuItem(get_localized_string(LANG_0544).c_str(), NULL, false, map && !map->is_mdl_model))
			{
				showImportMapWidget_Type = SHOW_IMPORT_MODEL_BSP;
				showImportMapWidget = true;
			}
			if (ImGui::MenuItem(get_localized_string(LANG_0545).c_str(), NULL, false, map && !map->is_mdl_model))
			{
				showImportMapWidget_Type = SHOW_IMPORT_MODEL_ENTITY;
				showImportMapWidget = true;
			}
			if (ImGui::MenuItem(get_localized_string(LANG_0546).c_str(), NULL, false, map && !map->is_mdl_model))
			{
				std::string path = map->bsp_path.substr(0, map->bsp_path.find_last_of(".")) + ".lit";
				// import lightmaps placeholder
			}
			ImGui::EndMenu();
		}

		if (ImGui::MenuItem(get_localized_string(LANG_0550).c_str()))
		{
			std::string entPath = g_game_dir + "svencoop_addon/scripts/maps/bspguy/maps/" + map->bsp_name + ".ent";
			std::string bspPath = g_game_dir + "svencoop_addon/maps/" + map->bsp_name + ".bsp";
			map->export_entities(entPath);
			map->write(bspPath);
		}

		if (ImGui::MenuItem("Merge", NULL, false, !app->isLoading))
		{
			showMergeMapWidget = true;
		}

		if (ImGui::BeginMenu("Recent Files", g_settings.lastOpened.size()))
		{
			for (int i = (int)g_settings.lastOpened.size() - 1; i >= 0; i--)
			{
				std::string file = g_settings.lastOpened[i];
				std::string smallPath = file;
				if (smallPath.size() > 40)
					smallPath = "..." + smallPath.substr(smallPath.size() - 40);

				if (ImGui::MenuItem(smallPath.c_str(), NULL, false, fileExists(file)))
				{
					g_settings.AddRecentFile(file);
					OpenFile(file);
				}
			}
			ImGui::EndMenu();
		}

		if (ImGui::MenuItem(get_localized_string(LANG_0552).c_str(), 0, false, map && !map->is_mdl_model && !app->isLoading))
		{
			app->reloadMaps();
		}
		if (ImGui::MenuItem(get_localized_string(LANG_0553).c_str(), 0, false, map && !map->is_mdl_model && !app->isLoading))
		{
			map->validate();
		}
		if (ImGui::MenuItem(get_localized_string(LANG_0554).c_str(), 0, false, !app->isLoading))
		{
			showSettingsWidget = true;
		}
		if (ImGui::MenuItem(get_localized_string(LANG_0555).c_str(), NULL))
		{
			exit(0);
		}

		ImGui::EndMenu();
	}
}

void Gui::drawMenu_Edit()
{
	Bsp* map = app->getSelectedMap();

	if (ImGui::BeginMenu(get_localized_string(LANG_0556).c_str(), (map && !map->is_mdl_model)))
	{
		BspRenderer* rend = map ? map->getBspRender() : NULL;
		bool canUndo = rend && rend->undoHistory.size() > 0;
		bool canRedo = rend && rend->redoHistory.size() > 0;
		std::string undoTitle = canUndo ? "Undo " + rend->undoHistory.back()->desc : "Undo";
		std::string redoTitle = canRedo ? "Redo " + rend->redoHistory.back()->desc : "Redo";

		if (ImGui::MenuItem(undoTitle.c_str(), get_localized_string(LANG_0557).c_str(), false, canUndo))
		{
			if (rend) rend->undo();
		}
		else if (ImGui::MenuItem(redoTitle.c_str(), get_localized_string(LANG_0558).c_str(), false, canRedo))
		{
			if (rend) rend->redo();
		}

		bool nonWorldspawnEntSelected = false;
		if (app->pickInfo.selectedEnts.size())
		{
			if (app->pickInfo.selectedEnts[0] != 0)
				nonWorldspawnEntSelected = true;
		}

		if (ImGui::MenuItem(get_localized_string(LANG_1081).c_str(), get_localized_string(LANG_1082).c_str(), false, nonWorldspawnEntSelected && app->pickInfo.selectedEnts.size()))
		{
			app->cutEnt();
		}
		if (ImGui::MenuItem(get_localized_string(LANG_1083).c_str(), get_localized_string(LANG_1084).c_str(), false, app->pickInfo.selectedFaces.size() || (nonWorldspawnEntSelected && app->pickInfo.selectedEnts.size())))
		{
			app->copyEnt();
		}

		if (ImGui::BeginMenu((get_localized_string(LANG_0449) + "###BeginPaste2").c_str()))
		{
			if (ImGui::MenuItem((get_localized_string(LANG_0449) + "###BEG2_PASTE1").c_str(), get_localized_string(LANG_0441).c_str(), false))
			{
				app->pasteEnt(false);
			}
			if (ImGui::MenuItem((get_localized_string(LANG_0450) + "###BEG2_OPASTE1").c_str(), 0, false))
			{
				app->pasteEnt(true);
			}
			if (ImGui::MenuItem("Paste with bspmodel###BEG2_PASTE2", get_localized_string(LANG_0441).c_str(), false))
			{
				app->pasteEnt(false, true);
			}
			ImGui::EndMenu();
		}

		if (ImGui::MenuItem(get_localized_string(LANG_1085).c_str(), get_localized_string(LANG_1086).c_str(), false, nonWorldspawnEntSelected))
		{
			app->deleteEnts();
		}

		bool entSelected = app->pickInfo.selectedEnts.size() > 0;
		if (ImGui::MenuItem(app->movingEnt ? "Ungrab" : "Grab", get_localized_string(LANG_1088).c_str(), false, nonWorldspawnEntSelected))
		{
			if (app->movingEnt) app->ungrabEnt(); else app->grabEnt();
		}
		if (ImGui::MenuItem(get_localized_string(LANG_1089).c_str(), get_localized_string(LANG_1090).c_str(), false, entSelected))
		{
			showTransformWidget = true;
		}
		if (ImGui::MenuItem(get_localized_string(LANG_1091).c_str(), get_localized_string(LANG_1092).c_str(), false, entSelected))
		{
			showKeyvalueWidget = true;
		}

		ImGui::EndMenu();
	}
}

void Gui::drawMenu_View()
{
	Bsp* map = app->getSelectedMap();

	if (ImGui::BeginMenu(get_localized_string(LANG_MENU_VIEW).c_str()))
	{
		if (ImGui::BeginMenu(get_localized_string(LANG_0566).c_str(), map))
		{
			if (ImGui::MenuItem(get_localized_string(LANG_0567).c_str(), NULL, app->clipnodeRenderHull == -1))
				app->clipnodeRenderHull = -1;
			if (ImGui::MenuItem(get_localized_string(LANG_0568).c_str(), NULL, app->clipnodeRenderHull == 0))
				app->clipnodeRenderHull = 0;
			if (ImGui::MenuItem(get_localized_string(LANG_0569).c_str(), NULL, app->clipnodeRenderHull == 1))
				app->clipnodeRenderHull = 1;
			if (ImGui::MenuItem(get_localized_string(LANG_0570).c_str(), NULL, app->clipnodeRenderHull == 2))
				app->clipnodeRenderHull = 2;
			if (ImGui::MenuItem(get_localized_string(LANG_0571).c_str(), NULL, app->clipnodeRenderHull == 3))
				app->clipnodeRenderHull = 3;
			ImGui::EndMenu();
		}

		ImGui::Separator();

		if (ImGui::BeginMenu("Toggle Panels / Widgets"))
		{
			ImGui::MenuItem(get_localized_string(LANG_0596).c_str(), NULL, &showKeyvalueWidget);
			ImGui::MenuItem(get_localized_string(LANG_1160).c_str(), NULL, &showTransformWidget);
			ImGui::MenuItem(get_localized_string(LANG_0597).c_str(), "", &showFaceEditWidget);
			ImGui::MenuItem(get_localized_string(LANG_0598).c_str(), "", &showTextureBrowser);
			ImGui::MenuItem(get_localized_string(LANG_0599).c_str(), "", &showLightmapEditorWidget);
			ImGui::MenuItem(get_localized_string(LANG_0594).c_str(), "", &showLogWidget);
			ImGui::MenuItem(get_localized_string(LANG_0595).c_str(), NULL, &showDebugWidget);
			ImGui::MenuItem("Map Overview", "", &showOverviewWidget);
			ImGui::EndMenu();
		}

		if (ImGui::MenuItem(get_localized_string(LANG_1095).c_str(), NULL, &showGOTOWidget)) {}
		if (ImGui::MenuItem(get_localized_string(LANG_0563).c_str(), NULL, &showLimitsWidget)) {}

		ImGui::EndMenu();
	}
}

void Gui::drawMenu_Map()
{
	Bsp* map = app->getSelectedMap();

	if (ImGui::BeginMenu(get_localized_string(LANG_0561).c_str(), (map && !map->is_mdl_model)))
	{
		if (ImGui::MenuItem(get_localized_string(LANG_0562).c_str(), NULL))
		{
			showEntityReport = true;
		}
		if (ImGui::MenuItem(get_localized_string(LANG_0563).c_str(), NULL))
		{
			showLimitsWidget = true;
		}

		if (ImGui::MenuItem("Generate nav mesh", NULL, false, true))
		{
			BspRenderer* rend = map ? map->getBspRender() : NULL;
			if (rend)
			{
				rend->generateNavMeshBuffer();
				rend->generateLeafNavMeshBuffer();
			}
		}

		ImGui::EndMenu();
	}
}

void Gui::drawMenu_Tools()
{
	Bsp* map = app->getSelectedMap();
	BspRenderer* rend = map ? map->getBspRender() : NULL;

	if (ImGui::BeginMenu(get_localized_string(LANG_MENU_TOOLS).c_str()))
	{
		bool hasAnyCollision = false;
		if (map)
		{
			for (int i = 0; i < MAX_MAP_HULLS; i++)
			{
				if (anyHullValid[i]) { hasAnyCollision = true; break; }
			}
		}

		if (ImGui::BeginMenu(get_localized_string(LANG_1093).c_str(), hasAnyCollision && !app->isLoading && map))
		{
			for (int i = 0; i < MAX_MAP_HULLS; i++)
			{
				if (ImGui::MenuItem(("Hull " + std::to_string(i)).c_str(), NULL, false, anyHullValid[i]))
				{
					if (rend) rend->pushUndoState("Delete Hull", EDIT_MODEL_LUMPS);
					map->delete_hull(i, 0);
					if (rend) rend->reload();
					pickCount++;
				}
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Experimental / WIP Tools"))
		{
			if (ImGui::MenuItem("PROTECT MAP!(WIP)", NULL, false, map && !map->is_protected && rend)) {}
			ImGui::EndMenu();
		}

		ImGui::EndMenu();
	}
}

void Gui::drawMenu_Create()
{
	Bsp* map = app->getSelectedMap();

	if (ImGui::BeginMenu(get_localized_string(LANG_0587).c_str(), (map && !map->is_mdl_model)))
	{
		ImGui::EndMenu();
	}
}

void Gui::drawMenu_Windows()
{
	if (ImGui::BeginMenu(get_localized_string(LANG_0601).c_str()))
	{
		ImGui::MenuItem("Console", NULL, &g_console_visible);
		ImGui::Separator();
		for (auto& bspRend : mapRenderers)
		{
			if (bspRend && bspRend->map)
			{
				bool selected = (app->getSelectedMap() == bspRend->map);
				if (ImGui::MenuItem(bspRend->map->bsp_name.c_str(), NULL, selected))
				{
					app->selectMap(bspRend->map);
				}
			}
		}
		ImGui::EndMenu();
	}
}

void Gui::drawMenu_Help()
{
	if (ImGui::BeginMenu(get_localized_string(LANG_0602).c_str()))
	{
		if (ImGui::MenuItem(get_localized_string(LANG_0603).c_str()))
		{
			showHelpWidget = true;
		}
		if (ImGui::MenuItem(get_localized_string(LANG_0604).c_str()))
		{
			showAboutWidget = true;
		}
		ImGui::Separator();
		ImGui::MenuItem("Debug Mode", NULL, &showDebugWidget);
		ImGui::EndMenu();
	}
}

void Gui::drawMenu_Debug()
{
	if (ImGui::BeginMenu(get_localized_string(LANG_0605).c_str()))
	{
		if (ImGui::MenuItem("Print textures"))
		{
			Bsp* map = app->getSelectedMap();
			if (map) map->print_info(false, 0, 0);
		}
		ImGui::EndMenu();
	}
}
