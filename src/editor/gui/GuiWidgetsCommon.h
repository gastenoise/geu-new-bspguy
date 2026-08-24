#pragma once
#include "../BspRenderer.h"
#include "../Gui.h"
#include "../Renderer.h"
#include "../Settings.h"
#include "BspMerger.h"
#include "LeafNavMesh.h"
#include "MutexManager.h"
#include "as.h"
#include "bsp/Bsp.h"
#include "filedialog/ImFileDialog.h"
#include "fmt/format.h"
#include "imgui_stdlib.h"
#include "lang.h"
#include "lodepng.h"
#include "log.h"
#include "quantizer.h"
#include "util.h"
#include "vis.h"
#include "winding.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <unordered_set>

extern float g_tooltip_delay;
