#include "ActionRegistry.h"
#include <algorithm>
#include <cctype>

static std::string toLowerString(const std::string& str)
{
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(),
		[](unsigned char c) { return (char)std::tolower(c); });
	return result;
}

ActionRegistry& ActionRegistry::getInstance()
{
	static ActionRegistry instance;
	return instance;
}

void ActionRegistry::registerAction(const ActionItem& action)
{
	for (auto& a : actions)
	{
		if (a.id == action.id)
		{
			a = action;
			return;
		}
	}
	actions.push_back(action);
}

std::vector<ActionItem> ActionRegistry::searchActions(const std::string& query) const
{
	if (query.empty())
		return actions;

	std::string lowerQuery = toLowerString(query);
	std::vector<ActionItem> results;

	for (const auto& a : actions)
	{
		std::string lowerTitle = toLowerString(a.title);
		std::string lowerCat = toLowerString(a.category);
		std::string lowerDesc = toLowerString(a.description);
		std::string lowerShortcut = toLowerString(a.shortcut);

		if (lowerTitle.find(lowerQuery) != std::string::npos ||
			lowerCat.find(lowerQuery) != std::string::npos ||
			lowerDesc.find(lowerQuery) != std::string::npos ||
			lowerShortcut.find(lowerQuery) != std::string::npos)
		{
			results.push_back(a);
		}
	}

	return results;
}

bool ActionRegistry::executeAction(const std::string& id)
{
	for (const auto& a : actions)
	{
		if (a.id == id)
		{
			if (!a.isEnabled || a.isEnabled())
			{
				if (a.callback)
				{
					a.callback();
					return true;
				}
			}
			return false;
		}
	}
	return false;
}
