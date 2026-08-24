#pragma once
#include <string>
#include <vector>
#include <functional>
#include <memory>

struct ActionItem
{
	std::string id;
	std::string title;
	std::string category;
	std::string shortcut;
	std::string description;
	std::function<void()> callback;
	std::function<bool()> isEnabled;
};

class ActionRegistry
{
  public:
	static ActionRegistry& getInstance();

	void registerAction(const ActionItem& action);
	const std::vector<ActionItem>& getAllActions() const { return actions; }
	std::vector<ActionItem> searchActions(const std::string& query) const;
	bool executeAction(const std::string& id);

	void clear() { actions.clear(); }

  private:
	ActionRegistry() = default;
	std::vector<ActionItem> actions;
};

void RegisterAllAppActions(class Gui* gui, class Renderer* app);
