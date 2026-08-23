#pragma once
#include <unordered_set>
#include "../Gui.h"
#include "../Renderer.h"
#include "../BspRenderer.h"
#include "bsp/Bsp.h"
#include "../Settings.h"
#include "lang.h"
#include "filedialog/ImFileDialog.h"
#include "imgui_stdlib.h"
#include "quantizer.h"
#include "vis.h"
#include "winding.h"
#include "util.h"
#include "log.h"
#include "BspMerger.h"
#include "LeafNavMesh.h"
#include "as.h"
#include "lodepng.h"
#include "fmt/format.h"
#include "MutexManager.h"
#include <filesystem>
#include <algorithm>
#include <cmath>

extern float g_tooltip_delay;
