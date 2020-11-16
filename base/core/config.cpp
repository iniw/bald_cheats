// used: shgetknownfolderpath
#include <shlobj.h>

#include "config.h"
// used: cheat variables
#include "variables.h"
// used: log config result state
#include "../utilities/logging.h"
// used: json parser implementation
#include "../../dependencies/json/json.hpp"
// used: arrWeaponNames
#include "../sdk/gamedata.h"

bool C::Setup(std::string_view szDefaultFileName)
{
	// create directory "settings" in "%userprofile%\documents\.qo0" if it incorrect or doesnt exists
	if (!std::filesystem::is_directory(fsPath))
	{
		std::filesystem::remove(fsPath);
		if (!std::filesystem::create_directories(fsPath))
			return false;
	}

	// create default config
	if (!Save(szDefaultFileName))
		return false;

	// load default config
	if (!Load(szDefaultFileName))
		return false;

	// refresh configs list
	Refresh();

	return true;
}

bool C::Save(std::string_view szFileName)
{
	// check for extension if it is not our replace it
	std::filesystem::path fsFilePath(szFileName);
	if (fsFilePath.extension() != XorStr(".bald"))
		fsFilePath.replace_extension(XorStr(".bald"));

	// get utf-8 full path to config
	const std::string szFile = std::filesystem::path(fsPath / fsFilePath).u8string();
	nlohmann::json config;

	try
	{
		for (auto& variable : vecVariables)
		{
			nlohmann::json entry;

			// save hashes to compare it later
			entry[XorStr("name-id")] = variable.uNameHash;
			entry[XorStr("type-id")] = variable.uTypeHash;

			// get current variable
			switch (variable.uTypeHash)
			{
			case FNV1A::HashConst("int"):
			{
				entry[XorStr("value")] = variable.Get<int>();
				break;
			}
			case FNV1A::HashConst("float"):
			{
				entry[XorStr("value")] = variable.Get<float>();
				break;
			}
			case FNV1A::HashConst("bool"):
			{
				entry[XorStr("value")] = variable.Get<bool>();
				break;
			}
			case FNV1A::HashConst("std::string"):
			{
				entry[XorStr("value")] = variable.Get<std::string>();
				break;
			}
			case FNV1A::HashConst("Color"):
			{
				auto colVariable = variable.Get<Color>();

				// store RGBA as sub-node
				nlohmann::json sub;

				// fill node with all color values
				sub.push_back(colVariable.r());
				sub.push_back(colVariable.g());
				sub.push_back(colVariable.b());
				sub.push_back(colVariable.a());

				entry[XorStr("value")] = sub.dump();
				break;
			}
			case FNV1A::HashConst("std::vector<bool>"):
			{
				auto vecBools = variable.Get<std::vector<bool>>();

				// store vector values as sub-node
				nlohmann::json sub;

				// fill node with all vector values
				for (const auto& bValue : vecBools)
					sub.push_back(static_cast<bool>(bValue));

				entry[XorStr("value")] = sub.dump();
				break;
			}
			case FNV1A::HashConst("std::vector<int>"):
			{
				auto vecInts = variable.Get<std::vector<int>>();

				// store vector values as sub-node
				nlohmann::json sub;

				// fill node with all vector values
				for (auto& iValue : vecInts)
					sub.push_back(iValue);

				entry[XorStr("value")] = sub.dump();
				break;
			}
			case FNV1A::HashConst("std::vector<float>"):
			{
				auto vecFloats = variable.Get<std::vector<float>>();

				// store vector values as sub-node
				nlohmann::json sub;

				// fill node with all vector values
				for (auto& flValue : vecFloats)
					sub.push_back(flValue);

				entry[XorStr("value")] = sub.dump();
				break;
			}
			case FNV1A::HashConst("std::vector<LegitbotVariables_t>"):
			{
				auto vecVariables = variable.Get<std::vector<LegitbotVariables_t>>();

				// store vector values as sub-node
				nlohmann::json sub;

				// fill node with all vector values
				for (auto& weapon : vecVariables)
				{
					sub.push_back(weapon.iAimKey);
					sub.push_back(weapon.flAimFov);
					sub.push_back(weapon.iAimHitbox);
					sub.push_back(weapon.iAimPriorityHitbox);
					sub.push_back(weapon.bAimAtBacktrack);
					sub.push_back(weapon.bAimSilent);
					sub.push_back(weapon.flAimSmooth);
					sub.push_back(weapon.bAimRCS);
					sub.push_back(weapon.bAimAutoWall);
					sub.push_back(weapon.iAimAutoWallMinDamage);
				}

				entry[XorStr("value")] = sub.dump();
				break;
			}
			case FNV1A::HashConst("std::map<""int""," "SkinchangerVariables_t"">"):
			{
				auto mapVariables = variable.Get<std::map<int, SkinchangerVariables_t>>();
				
				// store vector values as sub-node
				nlohmann::json sub;

				for (std::size_t i = 0U; i < mapVariables.size(); i++)
				{
					int iIndex = arrWeaponNames[i].first;

					sub.push_back(iIndex);
					sub.push_back(mapVariables[iIndex].bEnabled);
					sub.push_back(mapVariables[iIndex].iPaintKit);
					sub.push_back(mapVariables[iIndex].iPaintKitIndex);
					sub.push_back(mapVariables[iIndex].iSeed);
					sub.push_back(mapVariables[iIndex].bStatTrak);
					sub.push_back(mapVariables[iIndex].iStatTrak);
					sub.push_back(mapVariables[iIndex].iQuality);
					sub.push_back(mapVariables[iIndex].bNameTag);
					sub.push_back(mapVariables[iIndex].szNameTag);
					sub.push_back(mapVariables[iIndex].flWear);
					sub.push_back(mapVariables[iIndex].iDefinitionIndex);
					sub.push_back(mapVariables[iIndex].iDefinitionIndexOverride);
				}

				entry[XorStr("value")] = sub.dump();
				break;
			}
			default:
				break;
			}

			// add current variable to config
			config.push_back(entry);
		}
	}
	catch (const nlohmann::detail::exception& ex)
	{
		L::PushConsoleColor(FOREGROUND_RED);
		L::Print(fmt::format(XorStr("[error] json save failed: {}"), ex.what()));
		L::PopConsoleColor();
		return false;
	}

	// open output config file
	std::ofstream ofsOutFile(szFile, std::ios::out | std::ios::trunc);

	if (!ofsOutFile.good())
		return false;

	try
	{
		// write stored variables
		ofsOutFile << config.dump(4);
		ofsOutFile.close();
	}
	catch (std::ofstream::failure& ex)
	{
		L::PushConsoleColor(FOREGROUND_RED);
		L::Print(fmt::format(XorStr("[error] failed to save configuration: {}"), ex.what()));
		L::PopConsoleColor();
		return false;
	}

	L::Print(fmt::format(XorStr("saved configuration at: {}"), szFile));
	return true;
}

bool C::Load(std::string_view szFileName)
{
	// get utf-8 full path to config
	const std::string szFile = std::filesystem::path(fsPath / szFileName).u8string();
	nlohmann::json config;

	// open input config file
	std::ifstream ifsInputFile(szFile, std::ios::in);

	if (!ifsInputFile.good())
		return false;

	try
	{
		// parse saved variables
		ifsInputFile >> config;
		ifsInputFile.close();
	}
	catch (std::ifstream::failure& ex)
	{
		L::PushConsoleColor(FOREGROUND_RED);
		L::Print(fmt::format(XorStr("[error] failed to load configuration: {}"), ex.what()));
		L::PopConsoleColor();
		return false;
	}

	try
	{
		for (auto& variable : config)
		{
			int nIndex = GetVariableIndex(variable[XorStr("name-id")].get<FNV1A_t>());

			// check is variable exist
			if (nIndex == C_INVALID_VARIABLE)
				continue;

			// get variable
			auto& entry = vecVariables.at(nIndex);

			switch (variable[XorStr("type-id")].get<FNV1A_t>())
			{
			case FNV1A::HashConst("bool"):
			{
				entry.Set<bool>(variable[XorStr("value")].get<bool>());
				break;
			}
			case FNV1A::HashConst("float"):
			{
				entry.Set<float>(variable[XorStr("value")].get<float>());
				break;
			}
			case FNV1A::HashConst("int"):
			{
				entry.Set<int>(variable[XorStr("value")].get<int>());
				break;
			}
			case FNV1A::HashConst("std::string"):
			{
				entry.Set<std::string>(variable[XorStr("value")].get<std::string>());
				break;
			}
			case FNV1A::HashConst("Color"):
			{
				auto color = nlohmann::json::parse(variable[XorStr("value")].get<std::string>());

				entry.Set<Color>(Color(
					color.at(0).get<std::uint8_t>(),
					color.at(1).get<std::uint8_t>(),
					color.at(2).get<std::uint8_t>(),
					color.at(3).get<std::uint8_t>()
				));

				break;
			}
			case FNV1A::HashConst("std::vector<bool>"):
			{
				auto vector = nlohmann::json::parse(variable[XorStr("value")].get<std::string>());
				auto& vecBools = entry.Get<std::vector<bool>>();

				for (std::size_t i = 0U; i < vector.size(); i++)
				{
					// check is item out of bounds
					if (i < vecBools.size())
						vecBools.at(i) = vector.at(i).get<bool>();
				}

				break;
			}
			case FNV1A::HashConst("std::vector<int>"):
			{
				auto vector = nlohmann::json::parse(variable[XorStr("value")].get<std::string>());
				auto& vecInts = entry.Get<std::vector<int>>();

				for (std::size_t i = 0U; i < vector.size(); i++)
				{
					// check is item out of bounds
					if (i < vecInts.size())
						vecInts.at(i) = vector.at(i).get<int>();
				}

				break;
			}
			case FNV1A::HashConst("std::vector<float>"):
			{
				auto vector = nlohmann::json::parse(variable[XorStr("value")].get<std::string>());
				auto& vecFloats = entry.Get<std::vector<float>>();

				for (std::size_t i = 0U; i < vector.size(); i++)
				{
					// check is item out of bounds
					if (i < vecFloats.size())
						vecFloats.at(i) = vector.at(i).get<float>();
				}

				break;
			}
			case FNV1A::HashConst("std::vector<LegitbotVariables_t>"):
			{
				auto vector = nlohmann::json::parse(variable[XorStr("value")].get<std::string>());
				auto& vecVars = entry.Get<std::vector<LegitbotVariables_t>>();

				// fill node with all vector values
				for (std::size_t i = 0U; i < vector.size(); i++)
				{
					if (i < vecVars.size())
					{
						vecVars.at(i).iAimKey				= vector.at(i * 10 + 0).get<int>();
						vecVars.at(i).flAimFov				= vector.at(i * 10 + 1).get<float>();
						vecVars.at(i).iAimHitbox			= vector.at(i * 10 + 2).get<int>();
						vecVars.at(i).iAimPriorityHitbox	= vector.at(i * 10 + 3).get<int>();
						vecVars.at(i).bAimAtBacktrack		= vector.at(i * 10 + 4).get<bool>();
						vecVars.at(i).bAimSilent			= vector.at(i * 10 + 5).get<bool>();
						vecVars.at(i).flAimSmooth			= vector.at(i * 10 + 6).get<float>();
						vecVars.at(i).bAimRCS				= vector.at(i * 10 + 7).get<bool>();
						vecVars.at(i).bAimAutoWall			= vector.at(i * 10 + 8).get<bool>();
						vecVars.at(i).iAimAutoWallMinDamage = vector.at(i * 10 + 9).get<int>();
					}
				}

				break;
			}
			case FNV1A::HashConst("std::map<""int""," "SkinchangerVariables_t"">"):
			{
				auto map = nlohmann::json::parse(variable[XorStr("value")].get<std::string>());
				auto& mapVars = entry.Get<std::map<int, SkinchangerVariables_t>>();

				// fill node with all vector values
				for (std::size_t i = 0U; i < map.size(); i++)
				{
					if (i < mapVars.size())
					{
						int iIndex = arrWeaponNames[i].first;
																 // i * size of struct + 1 (skip definition index)
						mapVars.at(iIndex).bEnabled					= map.at(i * 13 + 1).get<bool>();
						mapVars.at(iIndex).iPaintKit				= map.at(i * 13 + 2).get<int>();
						mapVars.at(iIndex).iPaintKitIndex			= map.at(i * 13 + 3).get<int>();
						mapVars.at(iIndex).iSeed					= map.at(i * 13 + 4).get<int>();
						mapVars.at(iIndex).bStatTrak				= map.at(i * 13 + 5).get<bool>();
						mapVars.at(iIndex).iStatTrak				= map.at(i * 13 + 6).get<int>();
						mapVars.at(iIndex).iQuality					= map.at(i * 13 + 7).get<int>();
						mapVars.at(iIndex).bNameTag					= map.at(i * 13 + 8).get<bool>();
						mapVars.at(iIndex).szNameTag				= map.at(i * 13 + 9).get<std::string>();
						mapVars.at(iIndex).flWear					= map.at(i * 13 + 10).get<float>();
						mapVars.at(iIndex).iDefinitionIndex			= map.at(i * 13 + 11).get<int>();
						mapVars.at(iIndex).iDefinitionIndexOverride = map.at(i * 13 + 12).get<int>();
					}
				}

				break;
			}
			default:
				break;
			}
		}
	}
	catch (const nlohmann::detail::exception& ex)
	{
		L::PushConsoleColor(FOREGROUND_RED);
		L::Print(fmt::format(XorStr("[error] json load failed: {}"), ex.what()));
		L::PopConsoleColor();
		return false;
	}

	L::Print(fmt::format(XorStr("loaded configuration at: {}"), szFile));
	return true;
}

void C::Remove(std::string_view szFileName)
{
	// unable delete default config
	if (!szFileName.compare(XorStr("default.bald")))
		return;

	// get utf-8 full path to config
	const std::string szFile = std::filesystem::path(fsPath / szFileName).u8string();

	if (std::filesystem::remove(szFile))
		L::Print(fmt::format(XorStr("removed configuration at: {}"), szFile));
}

void C::Refresh()
{
	vecFileNames.clear();

	for (const auto& it : std::filesystem::directory_iterator(fsPath))
    {
		if (it.path().filename().extension() == XorStr(".bald"))
		{
			L::Print(fmt::format(XorStr("found configuration file: {}"), it.path().filename().u8string()));
			vecFileNames.push_back(it.path().filename().u8string());
		}
    }
}

std::size_t C::GetVariableIndex(const FNV1A_t uNameHash)
{
	for (std::size_t i = 0U; i < vecVariables.size(); i++)
	{
		if (vecVariables.at(i).uNameHash == uNameHash)
			return i;
	}

	return C_INVALID_VARIABLE;
}

std::filesystem::path C::GetWorkingPath()
{
	std::filesystem::path fsWorkingPath;

	// get path to user documents
	if (PWSTR pszPathToDocuments; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0UL, nullptr, &pszPathToDocuments)))
	{
		fsWorkingPath.assign(pszPathToDocuments);
		fsWorkingPath.append(XorStr(".bald"));
		CoTaskMemFree(pszPathToDocuments);
	}
	
	return fsWorkingPath;
}
