#pragma once
#include <string>

class Gui;

class GuiCommandPalette
{
public:
	static GuiCommandPalette& getInstance();

	void open();
	void toggle();
	void close();
	bool isOpened() const { return isOpen; }

	void draw(Gui* gui);

private:
	GuiCommandPalette() = default;

	bool isOpen = false;
	char searchBuffer[256] = { 0 };
	int selectedIdx = 0;
	bool requestFocus = false;
};
