#pragma once

#include "Bsp.h"
#include "Entity.h"
#include "Renderer.h"
#include "Settings.h"
#include "util.h"

// Undoable actions

class EditBspCommand
{
  public:
	std::string desc;
	EditBspCommand(const std::string &desc, LumpState oldLumps, LumpState newLumps, unsigned int targetLumps);
	~EditBspCommand() = default;

	void execute();
	void undo();
	size_t memoryused;
	size_t memoryUsage();
	size_t memoryUsageZip();
	LumpState oldLumps;
	LumpState newLumps;

  private:
	void refresh(BspRenderer *renderer);
	unsigned int targetLumps;
};
