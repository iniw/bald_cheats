#include "menu.h"

// used: global variables
#include "../global.h"
// used: config variables
#include "../core/variables.h"
// used: actions with config
#include "../core/config.h"
// used: configurations error logging
#include "../utilities/logging.h"
// used: render setup, etc
#include "../utilities/draw.h"
// used: engine, inputsystem, convar interfaces
#include "../core/interfaces.h"
// used: render visuals
#include "../features/visuals.h"
// used: skinchanger tab items map
#include "../features/skinchanger.h"
// used: inputtext() wrappers for c++ standard library (stl) type: std::string
#include "../../dependencies/imgui/cpp/imgui_stdlib.h"
// used: ForceFullUpdate()
#include "../utilities.h"
// used: vecSkinKits and vecGloveKits
#include "../sdk/gamedata.h"

#pragma region menu_arrays
static const char* arrVisualsFlags[] =
{
	"armor",
	"kit",
	"scoped"
};

static const char* arrVisualsRemovals[] =
{
	"post-processing",
	"flashbang",
	"punch",
	"smoke",
	"scope",
	"3d skybox",
	"shadows",
	"decals"
};
#pragma endregion

// spectator list, radar, other stuff here
#pragma region menu_windows
void W::MainWindow(IDirect3DDevice9* pDevice)
{
	ImGuiIO& io = ImGui::GetIO();
	const ImVec2 vecScreenSize = io.DisplaySize;

	ImGuiStyle& style = ImGui::GetStyle();
	ImDrawList* pForegroundDrawList = ImGui::GetForegroundDrawList();

	#pragma region main_visuals
	ImDrawList* pBackgroundDrawList = ImGui::GetBackgroundDrawList();
	D::RenderDrawData(pBackgroundDrawList);
	#pragma endregion

	#pragma region main_window
	ImGui::PushFont(F::Whitney);
	io.MouseDrawCursor = bMainOpened;

	if (bMainOpened)
	{
		int x, y;
		I::InputSystem->GetCursorPosition(&x, &y);

		// set imgui mouse position
		io.MousePos = ImVec2(static_cast<float>(x), static_cast<float>(y));

		ImGui::SetNextWindowPos(ImVec2(vecScreenSize.x * 0.5f, vecScreenSize.y * 0.5f), ImGuiCond_Once, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(500, 327), ImGuiCond_Always);
		ImGui::Begin(XorStr("bald_cheats"), &bMainOpened, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);
		{
			ImVec2 vecPos = ImGui::GetCursorScreenPos();
			float flWindowWidth = ImGui::GetWindowWidth();

			// header separate line
			//pForegroundDrawList->AddRectFilledMultiColor(ImVec2(vecPos.x - 8.f, vecPos.y - 6.f), ImVec2(vecPos.x + flWindowWidth - flWindowWidth / 3.f - 8.f, vecPos.y - 8.f), IM_COL32(75, 50, 105, 255), IM_COL32(110, 100, 130, 255), IM_COL32(110, 100, 130, 255), IM_COL32(75, 50, 105, 255));
			//pForegroundDrawList->AddRectFilledMultiColor(ImVec2(vecPos.x + flWindowWidth - flWindowWidth / 3.f - 8.f, vecPos.y - 6.f), ImVec2(vecPos.x + flWindowWidth - 8.f, vecPos.y - 8.f), IM_COL32(110, 100, 130, 255), IM_COL32(75, 50, 105, 255), IM_COL32(75, 50, 105, 255), IM_COL32(110, 100, 130, 255));

			pForegroundDrawList->AddRectFilled(ImVec2(vecPos.x - 8.f, vecPos.y - 6.f), ImVec2(vecPos.x + flWindowWidth - 8.f, vecPos.y - 8.f), IM_COL32(232, 80, 90, 255));

			// add tabs
			static std::array<CTab, 4U> const arrTabs =
			{
				//CTab{ "rage", &T::RageBot },
				CTab{ "legit", &T::LegitBot },
				CTab{ "visuals", &T::Visuals },
				CTab{ "miscellaneous", &T::Miscellaneous },
				CTab{ "skinchanger", &T::SkinChanger }
			};

			T::Render<arrTabs.size()>(XorStr("main_tabs"), arrTabs, &iMainTab, style.Colors[ImGuiCol_TabActive]);

			ImGui::End();
		}
	}

	ImGui::PopFont();
	#pragma endregion
}
#pragma endregion

#pragma region menu_tabs
template <std::size_t S>
void T::Render(const char* szTabBar, const std::array<CTab, S> arrTabs, int* nCurrentTab, const ImVec4& colActive, ImGuiTabBarFlags flags)
{
	// is empty check
	if (arrTabs.empty())
		return;

	// set active tab color
	ImGui::PushStyleColor(ImGuiCol_TabActive, colActive);
	if (ImGui::BeginTabBar(szTabBar, flags))
	{
		for (std::size_t i = 0U; i < arrTabs.size(); i++)
		{
			// add tab
			if (ImGui::BeginTabItem(arrTabs.at(i).szName))
			{
				// set current tab index
				*nCurrentTab = i;
				ImGui::EndTabItem();
			}
		}

		// render inner tab
		if (arrTabs.at(*nCurrentTab).pRenderFunction != nullptr)
			arrTabs.at(*nCurrentTab).pRenderFunction();

		ImGui::EndTabBar();
	}
	ImGui::PopStyleColor();
}
#pragma endregion

#pragma region menu_tabs_main
void T::LegitBot()
{
	ImGuiStyle& style = ImGui::GetStyle();

	ImGui::Columns(2, nullptr, false);
	{
		ImGui::BeginChild(XorStr("legitbot.aimbot"), ImVec2(0, 215), true, ImGuiWindowFlags_MenuBar);
		{
			if (ImGui::BeginMenuBar())
			{
				ImGui::TextUnformatted(XorStr("aim  assistance"));
				ImGui::EndMenuBar();
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, -1));

			LegitbotVariables_t& WeaponVars = C::Get<std::vector<LegitbotVariables_t>>(Vars.vecLegitVars)[C::Get<int>(Vars.iLegitWeapon)];

			ImGui::Checkbox(XorStr("master switch"), &C::Get<bool>(Vars.bLegit));
			ImGui::Combo(XorStr("weapon config"), &C::Get<int>(Vars.iLegitWeapon), XorStr("rifles\0snipers\0pistols\0heavy pistols\0smgs\0other\0\0"));
			ImGui::Combo(XorStr("hitbox selection"), &WeaponVars.iAimHitbox, XorStr("closest\0head\0chest\0stomach\0\0"));
			ImGui::SliderFloat(XorStr("maximum fov##legitbot"), &WeaponVars.flAimFov, 0, 20, u8"%.1f\u00B0");

			if (!WeaponVars.bAimSilent)
				ImGui::SliderFloat(XorStr("smoothing amount##legitbot"), &WeaponVars.flAimSmooth, 1, 10, "%.1f");

			ImGui::HotKey(XorStr("aim key##legitbot"), &WeaponVars.iAimKey);
			ImGui::Checkbox(XorStr("aim at backtrack"), &WeaponVars.bAimAtBacktrack);
			ImGui::Checkbox(XorStr("silent aim##legitbot"), &WeaponVars.bAimSilent);

			if (C::Get<int>(Vars.iLegitWeapon) != (int)ELegitWeaponTypes::SNIPERS) // no rcs for snipers
				ImGui::Checkbox(XorStr("recoil control##legitbot"), &WeaponVars.bAimRCS);

			ImGui::Checkbox(XorStr("penetrate walls##legitbot"), &WeaponVars.bAimAutoWall);

			if (WeaponVars.bAimAutoWall)
				ImGui::SliderInt(XorStr("minimum damage##legitbot"), &WeaponVars.iAimAutoWallMinDamage, 0, 100, "%dhp");

			ImGui::PopStyleVar();
			ImGui::EndChild();
		}
	}
	ImGui::BeginChild(XorStr("legitbot.backtrack"), ImVec2(), true, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	{
		if (ImGui::BeginMenuBar())
		{
			ImGui::TextUnformatted(XorStr("backtracking"));
			ImGui::EndMenuBar();
		}
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, -1));

		ImGui::Checkbox(XorStr("backtracking##backtracking"), &C::Get<bool>(Vars.bBacktracking));
		if (C::Get<bool>(Vars.bBacktracking))
			ImGui::SliderInt(XorStr("time##backtracking"), &C::Get<int>(Vars.iBacktrackingTime), 0, 200, "%dms");

		ImGui::PopStyleVar();
		ImGui::EndChild();
	}

	ImGui::NextColumn();
	{
		ImGui::BeginChild(XorStr("legitbot.triggerbot"), ImVec2(), true, ImGuiWindowFlags_MenuBar);
		{
			if (ImGui::BeginMenuBar())
			{
				ImGui::TextUnformatted(XorStr("triggerbot"));

				if (ImGui::BeginMenu(XorStr("hitboxes")))
				{
					ImGui::MenuItem(XorStr("head"), nullptr, &C::Get<bool>(Vars.bTriggerHead));
					ImGui::MenuItem(XorStr("chest"), nullptr, &C::Get<bool>(Vars.bTriggerChest));
					ImGui::MenuItem(XorStr("stomach"), nullptr, &C::Get<bool>(Vars.bTriggerStomach));
					ImGui::MenuItem(XorStr("arms"), nullptr, &C::Get<bool>(Vars.bTriggerArms));
					ImGui::MenuItem(XorStr("legs"), nullptr, &C::Get<bool>(Vars.bTriggerLegs));
					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, -1));
			ImGui::Checkbox(XorStr("enable"), &C::Get<bool>(Vars.bTrigger));
			ImGui::HotKey(XorStr("trigger key"), &C::Get<int>(Vars.iTriggerKey));
			ImGui::SliderInt(XorStr("reaction delay##trigger"), &C::Get<int>(Vars.iTriggerDelay), 0, 500, "%dms");
			ImGui::SliderInt(XorStr("minimum damage##trigger"), &C::Get<int>(Vars.iTriggerMinimumDamage), 1, 100, "%dhp");
			ImGui::Checkbox(XorStr("penetrate walls##trigger"), &C::Get<bool>(Vars.bTriggerAutoWall));
			ImGui::PopStyleVar();

			ImGui::EndChild();
		}
	}
	ImGui::Columns(1);
}

void T::Visuals()
{
	ImGuiStyle& style = ImGui::GetStyle();

	ImGui::Columns(2, nullptr, false);
	{
		ImGui::BeginChild(XorStr("visuals.esp"), ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar);
		{
			if (ImGui::BeginMenuBar())
			{
				ImGui::TextUnformatted(XorStr("esp"));

				if (ImGui::BeginMenu(XorStr("filters")))
				{
					switch (iEspTab)
					{
					case 0:
					{
						ImGui::MenuItem(XorStr("enemies"), nullptr, &C::Get<bool>(Vars.bEspMainEnemies));
						ImGui::MenuItem(XorStr("weapons"), nullptr, &C::Get<bool>(Vars.bEspMainWeapons));
						ImGui::MenuItem(XorStr("grenades"), nullptr, &C::Get<bool>(Vars.bEspMainGrenades));
						ImGui::MenuItem(XorStr("bomb"), nullptr, &C::Get<bool>(Vars.bEspMainBomb));
						break;
					}
					case 1:
					{
						ImGui::MenuItem(XorStr("enemies"), nullptr, &C::Get<bool>(Vars.bEspGlowEnemies));
						ImGui::MenuItem(XorStr("weapons"), nullptr, &C::Get<bool>(Vars.bEspGlowWeapons));
						ImGui::MenuItem(XorStr("grenades"), nullptr, &C::Get<bool>(Vars.bEspGlowGrenades));
						ImGui::MenuItem(XorStr("bomb"), nullptr, &C::Get<bool>(Vars.bEspGlowBomb));
						ImGui::MenuItem(XorStr("planted bomb"), nullptr, &C::Get<bool>(Vars.bEspGlowPlantedBomb));
						break;
					}
					case 2:
					{
						ImGui::MenuItem(XorStr("enemies"), nullptr, &C::Get<bool>(Vars.bEspChamsEnemies));
						ImGui::MenuItem(XorStr("viewmodel"), nullptr, &C::Get<bool>(Vars.bEspChamsViewModel));
						ImGui::MenuItem(XorStr("backtrack"), nullptr, &C::Get<bool>(Vars.bEspChamsBacktrack));
						break;
					}
					default:
						break;
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}

			static std::array<CTab, 3U> const arrEspTabs =
			{
				CTab{ "main", [&style]()
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, -1));
				ImGui::Checkbox(XorStr("enable##main"), &C::Get<bool>(Vars.bEspMain));

				if (C::Get<bool>(Vars.bEspMainEnemies))
				{
					static float flEnemiesChildSize = 0.f;	
					ImGui::BeginChild("visuals.enemies", ImVec2(0, flEnemiesChildSize), true, ImGuiWindowFlags_NoTitleBar);
					{
						ImGui::Text(XorStr("enemies"));
						ImGui::Checkbox(XorStr("radar##player"), &C::Get<bool>(Vars.bEspMainPlayerRadar));
						ImGui::Checkbox(XorStr("health##player"), &C::Get<bool>(Vars.bEspMainPlayerHealth));
						ImGui::ColorCheckbox(XorStr("override health color##player"), &C::Get<bool>(Vars.bEspMainPlayerOverrideHealthColor), &C::Get<Color>(Vars.colEspMainPlayerHealth));
						ImGui::ColorCheckbox(XorStr("box##player"), &C::Get<bool>(Vars.bEspMainPlayerBox), &C::Get<Color>(Vars.colEspMainPlayerBox));
						ImGui::ColorCheckbox(XorStr("skeleton##player"), &C::Get<bool>(Vars.bEspMainPlayerSkeleton), &C::Get<Color>(Vars.colEspMainPlayerSkeleton));
						ImGui::ColorCheckbox(XorStr("visualize backtrack##player"), &C::Get<bool>(Vars.bEspMainPlayerBacktrack), &C::Get<Color>(Vars.colEspMainPlayerBacktrack));
						ImGui::ColorCheckbox(XorStr("name##player"), &C::Get<bool>(Vars.bEspMainPlayerName), &C::Get<Color>(Vars.colEspMainPlayerName));
						ImGui::ColorCheckbox(XorStr("flash timer##player"), &C::Get<bool>(Vars.bEspMainPlayerFlash), &C::Get<Color>(Vars.colEspMainPlayerFlash));
						ImGui::ColorCheckbox(XorStr("nadekill indicator##player"), &C::Get<bool>(Vars.bEspMainPlayerNadeKill), &C::Get<Color>(Vars.colEspMainPlayerNadeKill));
						ImGui::ColorCheckbox(XorStr("weapon text##player"), &C::Get<bool>(Vars.bEspMainPlayerWeaponText), &C::Get<Color>(Vars.colEspMainPlayerWeaponText));
						ImGui::ColorCheckbox(XorStr("weapon icon##player"), &C::Get<bool>(Vars.bEspMainPlayerWeaponIcon), &C::Get<Color>(Vars.colEspMainPlayerWeaponIcon));
						ImGui::ColorCheckbox(XorStr("ammo##player"), &C::Get<bool>(Vars.bEspMainPlayerAmmo), &C::Get<Color>(Vars.colEspMainPlayerAmmo));
						ImGui::MultiCombo(XorStr("flags##player"), arrVisualsFlags, C::Get<std::vector<bool>>(Vars.vecEspMainPlayerFlags), IM_ARRAYSIZE(arrVisualsFlags));

						flEnemiesChildSize = ImGui::GetCursorPosY() + style.ItemSpacing.y;
						ImGui::EndChild();
					}
				}

				if (C::Get<bool>(Vars.bEspMainWeapons))
				{
					static float flWeaponsChildSize = 0.f;
					ImGui::BeginChild("visuals.weapons", ImVec2(0, flWeaponsChildSize), true, ImGuiWindowFlags_NoTitleBar);
					{
						ImGui::Text(XorStr("weapons"));
						ImGui::ColorCheckbox(XorStr("box##weapon"), &C::Get<bool>(Vars.bEspMainWeaponBox), &C::Get<Color>(Vars.colEspMainWeaponBox));
						ImGui::ColorCheckbox(XorStr("text##weapon"), &C::Get<bool>(Vars.bEspMainWeaponText), &C::Get<Color>(Vars.colEspMainWeaponText));
						ImGui::ColorCheckbox(XorStr("icon##weapon"), &C::Get<bool>(Vars.bEspMainWeaponIcon), &C::Get<Color>(Vars.colEspMainWeaponIcon));
						ImGui::ColorCheckbox(XorStr("ammo##weapon"), &C::Get<bool>(Vars.bEspMainWeaponAmmo), &C::Get<Color>(Vars.colEspMainWeaponAmmo));
						ImGui::ColorCheckbox(XorStr("distance##weapon"), &C::Get<bool>(Vars.bEspMainWeaponDistance), &C::Get<Color>(Vars.colEspMainWeaponDistance));

						flWeaponsChildSize = ImGui::GetCursorPosY() + style.ItemSpacing.y;
						ImGui::EndChild();
					}
				}

				if (C::Get<bool>(Vars.bEspMainGrenades))
				{
					static float flGrenadesChildSize = 0.f;
					ImGui::BeginChild("visuals.grenades", ImVec2(0, flGrenadesChildSize), true, ImGuiWindowFlags_NoTitleBar);
					{
						ImGui::Text(XorStr("grenades"));
						ImGui::Checkbox(XorStr("frame"), &C::Get<bool>(Vars.bEspMainGrenadesFrame));
						ImGui::Checkbox(XorStr("duration"), &C::Get<bool>(Vars.bEspMainGrenadesDuration));
						ImGui::ColorCheckbox(XorStr("distance"), &C::Get<bool>(Vars.bEspMainGrenadesDistance), &C::Get<Color>(Vars.colEspMainGrenadesDistance));
						ImGui::ColorCheckbox(XorStr("name"), &C::Get<bool>(Vars.bEspMainGrenadesName), &C::Get<Color>(Vars.colEspMainGrenadesName));
						flGrenadesChildSize = ImGui::GetCursorPosY() + style.ItemSpacing.y;
						ImGui::EndChild();
					}
				}
				ImGui::PopStyleVar();
			}},
				CTab{ "glow", [&style]()
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, -1));

				ImGui::Checkbox(XorStr("enable"), &C::Get<bool>(Vars.bEspGlow));

				if (C::Get<bool>(Vars.bEspGlowEnemies))
				{
					static float flEnemiesChildSize = 0.f;
					ImGui::BeginChild("glow.enemies", ImVec2(0, flEnemiesChildSize), true, ImGuiWindowFlags_NoTitleBar);
					{
						ImGui::Text(XorStr("enemies"));
						ImGui::ColorCheckbox(XorStr("visible##enemies"), &C::Get<bool>(Vars.bEspGlowEnemiesVisible), &C::Get<Color>(Vars.colEspGlowEnemiesVisible));
						ImGui::ColorCheckbox(XorStr("non-visible##enemies"), &C::Get<bool>(Vars.bEspGlowEnemiesWall), &C::Get<Color>(Vars.colEspGlowEnemiesWall));
						flEnemiesChildSize = ImGui::GetCursorPosY() + style.ItemSpacing.y;
						ImGui::EndChild();
					}
				}
				if (C::Get<bool>(Vars.bEspGlowWeapons) || C::Get<bool>(Vars.bEspGlowGrenades) || C::Get<bool>(Vars.bEspGlowBomb) || C::Get<bool>(Vars.bEspGlowPlantedBomb))
				{
					static float flOthersChildSize = 0.f;
					ImGui::BeginChild("glow.others", ImVec2(0, flOthersChildSize), true, ImGuiWindowFlags_NoTitleBar);
					{
						ImGui::Text(XorStr("others"));
						ImGui::Separator();
						if (C::Get<bool>(Vars.bEspGlowWeapons))
						{
							ImGui::Text(XorStr("weapons"));
							ImGui::ColorCheckbox(XorStr("visible##weapons"), &C::Get<bool>(Vars.bEspGlowWeaponsVisible), &C::Get<Color>(Vars.colEspGlowWeaponsVisible));
							ImGui::ColorCheckbox(XorStr("non-visible##weapons"), &C::Get<bool>(Vars.bEspGlowWeaponsWall), &C::Get<Color>(Vars.colEspGlowWeaponsWall));
						}

						if (C::Get<bool>(Vars.bEspGlowGrenades))
						{
							ImGui::Text(XorStr("grenades"));
							ImGui::ColorCheckbox(XorStr("visible##grenades"), &C::Get<bool>(Vars.bEspGlowGrenadesVisible), &C::Get<Color>(Vars.colEspGlowGrenadesVisible));
							ImGui::ColorCheckbox(XorStr("non-visible##grenades"), &C::Get<bool>(Vars.bEspGlowGrenadesWall), &C::Get<Color>(Vars.colEspGlowGrenadesWall));
						}

						if (C::Get<bool>(Vars.bEspGlowBomb))
						{
							ImGui::Text(XorStr("bomb"));
							ImGui::ColorCheckbox(XorStr("visible##bomb"), &C::Get<bool>(Vars.bEspGlowBombVisible), &C::Get<Color>(Vars.colEspGlowBombVisible));
							ImGui::ColorCheckbox(XorStr("non-visible##bomb"), &C::Get<bool>(Vars.bEspGlowBombWall), &C::Get<Color>(Vars.colEspGlowBombWall));
						}

						if (C::Get<bool>(Vars.bEspGlowPlantedBomb))
						{
							ImGui::Text(XorStr("planted bomb"));
							ImGui::ColorCheckbox(XorStr("visible##planted bomb"), &C::Get<bool>(Vars.bEspGlowPlantedBombVisible), &C::Get<Color>(Vars.colEspGlowPlantedBombVisible));
							ImGui::ColorCheckbox(XorStr("non-visible##planted bomb"), &C::Get<bool>(Vars.bEspGlowPlantedBombWall), &C::Get<Color>(Vars.colEspGlowPlantedBombWall));
						}
						
						flOthersChildSize = ImGui::GetCursorPosY() + style.ItemSpacing.y;
						ImGui::EndChild();
					}
				}
				// @note: if u rebuild glow and wanna use styles do like that
				//ImGui::Combo(XorStr("styles example##glow"), XorStr("outer\0rim\0edge\0edge pulse\0\0"));

				ImGui::PopStyleVar();
			}},
				CTab{ "chams", [&style]()
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, -1));
				ImGui::Checkbox(XorStr("enable##chams"), &C::Get<bool>(Vars.bEspChams));

				if (C::Get<bool>(Vars.bEspChamsEnemies))
				{
					static float flEnemiesChildSize = 0.f;
					ImGui::BeginChild("chams.enemies", ImVec2(0, flEnemiesChildSize), true, ImGuiWindowFlags_NoTitleBar);
					{
						ImGui::Text(XorStr("enemies"));
						ImGui::ColorCheckbox(XorStr("visible##chamsenemies"), &C::Get<bool>(Vars.bEspChamsEnemiesVisible), &C::Get<Color>(Vars.colEspChamsEnemiesVisible));
						ImGui::Combo(XorStr("visible material##chamsenemies"), &C::Get<int>(Vars.iEspChamsEnemiesVisible), XorStr("covered\0flat\0wireframe\0reflective\0\0"));
						ImGui::ColorCheckbox(XorStr("non-visible##chamsenemies"), &C::Get<bool>(Vars.bEspChamsEnemiesWall), &C::Get<Color>(Vars.colEspChamsEnemiesWall));
						ImGui::Combo(XorStr("non-visible material##chamsenemies"), &C::Get<int>(Vars.iEspChamsEnemiesWall), XorStr("covered\0flat\0wireframe\0reflective\0\0"));
						flEnemiesChildSize = ImGui::GetCursorPosY() + style.ItemSpacing.y;
						ImGui::EndChild();
					}
				}

				if (C::Get<bool>(Vars.bEspChamsBacktrack))
				{
					static float flEnemiesBacktrackChildSize = 0.f;
					ImGui::BeginChild("chams.backtrack", ImVec2(0, flEnemiesBacktrackChildSize), true, ImGuiWindowFlags_NoTitleBar);
					{
						ImGui::Text(XorStr("backtrack"));
						ImGui::Combo(XorStr("type##chamsbacktrack"), &C::Get<int>(Vars.iEspChamsBacktrackType), XorStr("all records\0last record\0\0"));
						ImGui::ColorCheckbox(XorStr("enable##chamsbacktrack"), &C::Get<bool>(Vars.bEspChamsEnemiesBacktrack), &C::Get<Color>(Vars.colEspChamsEnemiesBacktrack));
						ImGui::Combo(XorStr("material##chamsbacktrack"), &C::Get<int>(Vars.iEspChamsEnemiesBacktrack), XorStr("covered\0flat\0wireframe\0reflective\0\0"));
						flEnemiesBacktrackChildSize = ImGui::GetCursorPosY() + style.ItemSpacing.y;
						ImGui::EndChild();
					}
				}

				if (C::Get<bool>(Vars.bEspChamsViewModel))
				{
					static float flViewmodelChildSize = 0.f;
					ImGui::BeginChild("chams.viewmodel", ImVec2(0, flViewmodelChildSize), true, ImGuiWindowFlags_NoTitleBar);
					{
						ImGui::Text(XorStr("viewmodel"));
						ImVec4 vecViewmodelColor = { C::Get<Color>(Vars.colEspChamsViewModel).rBase(), C::Get<Color>(Vars.colEspChamsViewModel).gBase(), C::Get<Color>(Vars.colEspChamsViewModel).bBase(), C::Get<Color>(Vars.colEspChamsViewModel).aBase() };
						if (ImGui::ColorEdit4(XorStr("color"), &vecViewmodelColor.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip))
						{
							C::Get<Color>(Vars.colEspChamsViewModel) = Color(vecViewmodelColor.x, vecViewmodelColor.y, vecViewmodelColor.z, vecViewmodelColor.w);
						}
						ImGui::Combo(XorStr("viewmodel material##chams"), &C::Get<int>(Vars.iEspChamsViewModel), XorStr("no draw\0covered\0flat\0wireframe\0glow\0scroll\0chrome\0\0"));
						flViewmodelChildSize = ImGui::GetCursorPosY() + style.ItemSpacing.y;
						ImGui::EndChild();
					}
				}

				ImGui::PopStyleVar();
			} }
			};
			T::Render<arrEspTabs.size()>(XorStr("visuals_esp"), arrEspTabs, &iEspTab, style.Colors[ImGuiCol_ScrollbarGrab]);

			ImGui::EndChild();
		}
	}
	ImGui::NextColumn();
	{
		ImGui::BeginChild(XorStr("visuals.world"), ImVec2(0, 140), true, ImGuiWindowFlags_MenuBar);
		{
			if (ImGui::BeginMenuBar())
			{
				ImGui::TextUnformatted(XorStr("world"));
				ImGui::EndMenuBar();
			}
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, -1));

			ImGui::MultiCombo(XorStr("removals"), arrVisualsRemovals, C::Get<std::vector<bool>>(Vars.vecWorldRemovals), IM_ARRAYSIZE(arrVisualsRemovals));
			ImGui::HotKey(XorStr("thirdperson"), &C::Get<int>(Vars.iWorldThirdPersonKey));
			ImGui::SliderInt(XorStr("distance"), &C::Get<int>(Vars.iWorldThirdPersonOffset), 50, 500);
			ImGui::Checkbox(XorStr("night mode"), &C::Get<bool>(Vars.bWorldNightMode));
			ImGui::Checkbox(XorStr("fish on da head"), &C::Get<bool>(Vars.bWorldFish));
			ImGui::ColorCheckbox(XorStr("custom fog"), &C::Get<bool>(Vars.bWorldFog), &C::Get<Color>(Vars.colWorldFogPrimary));
			if (C::Get<bool>(Vars.bWorldFog))
			{
				ImGui::SliderInt(XorStr("start"), &C::Get<int>(Vars.iWorldFogStart), 0, 1000);	
				ImGui::SliderInt(XorStr("end"), &C::Get<int>(Vars.iWorldFogEnd), -0, 2000);
			}

			ImGui::PopStyleVar();
			ImGui::EndChild();
		}

		ImGui::BeginChild(XorStr("visuals.screen"), ImVec2(0, 0), true, ImGuiWindowFlags_MenuBar);
		{
			if (ImGui::BeginMenuBar())
			{
				ImGui::TextUnformatted(XorStr("screen"));
				ImGui::EndMenuBar();
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, -1));
			ImGui::SliderFloat(XorStr("camera fov"), &C::Get<float>(Vars.flScreenCameraFOV), -89.f, 89.f, u8"%.1f\u00B0");
			ImGui::SliderFloat(XorStr("viewmodel fov"), &C::Get<float>(Vars.flScreenViewModelFOV), -90.f, 90.f, u8"%.1f\u00B0");
			ImGui::Checkbox(XorStr("spectator list"), &C::Get<bool>(Vars.bSpectatorList));
			ImGui::Checkbox(XorStr("sniper crosshair"), &C::Get<bool>(Vars.bScreenSniperCrosshair));
			ImGui::Checkbox(XorStr("hitmarker"), &C::Get<bool>(Vars.bScreenHitMarker));
			if (C::Get<bool>(Vars.bScreenHitMarker))
			{
				ImGui::Checkbox(XorStr("damage"), &C::Get<bool>(Vars.bScreenHitMarkerDamage));
				ImGui::Checkbox(XorStr("sound"), &C::Get<bool>(Vars.bScreenHitMarkerSound));
				ImGui::SliderFloat(XorStr("time"), &C::Get<float>(Vars.flScreenHitMarkerTime), 0.1f, 2.f, "%.2fsec");
				ImGui::SliderInt(XorStr("gap"), &C::Get<int>(Vars.iScreenHitMarkerGap), 1, 20, "%d pixels");
				ImGui::SliderInt(XorStr("lenght"), &C::Get<int>(Vars.iScreenHitMarkerLenght), 5, 20, "%d pixels");
			}
			ImGui::PopStyleVar();

			ImGui::EndChild();
		}
	}
	ImGui::Columns(1);
}

void T::Miscellaneous()
{
	ImGuiStyle& style = ImGui::GetStyle();

	ImGui::Columns(2, nullptr, false);
	{
		static float flMovementChildSize = 0.f;
		ImGui::BeginChild(XorStr("misc.general"), ImVec2(0, flMovementChildSize), true, ImGuiWindowFlags_MenuBar);
		{
			if (ImGui::BeginMenuBar())
			{
				ImGui::TextUnformatted(XorStr("general"));
				ImGui::EndMenuBar();
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, -1));
			ImGui::Checkbox(XorStr("bhop"), &C::Get<bool>(Vars.bMiscBunnyHop));
			ImGui::HotKey(XorStr("jumpbug"), &C::Get<int>(Vars.iMiscJumpBugKey));
			ImGui::HotKey(XorStr("edgebug"), &C::Get<int>(Vars.iMiscEdgeBugKey));
			ImGui::HotKey(XorStr("edgejump"), &C::Get<int>(Vars.iMiscEdgeJumpKey));
			if (C::Get<int>(Vars.iMiscEdgeJumpKey) > 0)
				ImGui::Checkbox(XorStr("lj on edgejump"), &C::Get<bool>(Vars.bMiscEdgeJumpLong));
			ImGui::Checkbox(XorStr("no crouch cooldown"), &C::Get<bool>(Vars.bMiscNoCrouchCooldown));
			ImGui::Checkbox(XorStr("velocity indicator"), &C::Get<bool>(Vars.bMiscVeloIndicator));
			ImGui::PopStyleVar();

			flMovementChildSize = ImGui::GetCursorPosY() + style.ItemSpacing.y;
			ImGui::EndChild();
		}

		ImGui::BeginChild(XorStr("misc.other"), ImVec2(0, 0), true, ImGuiWindowFlags_MenuBar);
		{
			if (ImGui::BeginMenuBar())
			{
				ImGui::TextUnformatted(XorStr("other"));
				ImGui::EndMenuBar();
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, -1));
			ImGui::Checkbox(XorStr("fake latency"), &C::Get<bool>(Vars.bMiscFakeLatency));
			ImGui::SliderInt(XorStr("amount"), &C::Get<int>(Vars.iMiscFakeLatencyAmount), 1, 200, "%dms");
			ImGui::Checkbox(XorStr("reveal ranks"), &C::Get<bool>(Vars.bMiscRevealRanks));
			ImGui::Checkbox(XorStr("unlock inventory"), &C::Get<bool>(Vars.bMiscUnlockInventory));
			ImGui::Checkbox(XorStr("auto accept"), &C::Get<bool>(Vars.bMiscAutoAccept));
			ImGui::Checkbox(XorStr("auto pistol"), &C::Get<bool>(Vars.bMiscAutoPistol));
			ImGui::HotKey(XorStr("blockbot"), &C::Get<int>(Vars.iMiscBlockBotKey));
			//ImGui::Checkbox(XorStr("anti-untrusted"), &C::Get<bool>(Vars.bMiscAntiUntrusted));
			ImGui::PopStyleVar();

			ImGui::EndChild();
		}
	}
	ImGui::NextColumn();
	{
		// current selected configuration name
		static std::string szCurrentConfig = { };

		static float flConfigChildSize = 0.f;
		ImGui::BeginChild(XorStr("misc.config"), ImVec2(0, 0), true, ImGuiWindowFlags_MenuBar);
		{
			if (ImGui::BeginMenuBar())
			{
				ImGui::TextUnformatted(XorStr("configuration"));
				ImGui::EndMenuBar();
			}

			ImGui::PushItemWidth(-1);

			ImGui::ListBox(XorStr("##config.list"), &iSelectedConfig, [](int nIndex)
				{
					// return current displaying configuration name
					return C::vecFileNames.at(nIndex).c_str();
				}, C::vecFileNames.size(), 9);

			szCurrentConfig = !C::vecFileNames.empty() ? C::vecFileNames.at(iSelectedConfig) : "";
			ImGui::PopItemWidth();

			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, 0));
				ImGui::PushItemWidth(-1);
				if (ImGui::InputTextWithHint(XorStr("##config.file"), XorStr("create new..."), &szConfigFile, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					if (!C::Save(szConfigFile))
					{
						L::PushConsoleColor(FOREGROUND_RED);
						L::Print(fmt::format(XorStr("[error] failed to create \"{}\" config"), szConfigFile));
						L::PopConsoleColor();
					}

					szConfigFile.clear();
					C::Refresh();
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip(XorStr("press enter to create a new config"));

				if (ImGui::Button(XorStr("save"), ImVec2(-1, 15)))
				{
					if (!C::Save(szCurrentConfig))
					{
						L::PushConsoleColor(FOREGROUND_RED);
						L::Print(fmt::format(XorStr("[error] failed to save \"{}\" config"), szCurrentConfig));
						L::PopConsoleColor();
					}
				}

				if (ImGui::Button(XorStr("load"), ImVec2(-1, 15)))
				{
					if (!C::Load(szCurrentConfig))
					{
						L::PushConsoleColor(FOREGROUND_RED);
						L::Print(fmt::format(XorStr("[error] failed to load \"{}\" config"), szCurrentConfig));
						L::PopConsoleColor();
					}
				}

				if (ImGui::Button(XorStr("remove"), ImVec2(-1, 15)))
					ImGui::OpenPopup(XorStr("confirmation##config.remove"));

				if (ImGui::Button(XorStr("refresh"), ImVec2(-1, 15)))
					C::Refresh();

				ImGui::PopItemWidth();
				ImGui::PopStyleVar();
			}

			if (ImGui::BeginPopupModal(XorStr("confirmation##config.remove"), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text(XorStr("are you sure you want to remove \"%s\" configuration?"), szCurrentConfig.c_str());
				ImGui::Spacing();

				if (ImGui::Button(XorStr("no"), ImVec2(30, 0)))
					ImGui::CloseCurrentPopup();

				ImGui::SameLine();

				if (ImGui::Button(XorStr("yes"), ImVec2(30, 0)))
				{
					C::Remove(szCurrentConfig);
					// reset current configuration
					iSelectedConfig = 0;
					C::Refresh();
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			flConfigChildSize = ImGui::GetCursorPosY() + style.ItemSpacing.y;
			ImGui::EndChild();
		}
	}
	ImGui::Columns(1);
}

void T::SkinChanger()
{
	ImGuiStyle& style = ImGui::GetStyle();

	ImGui::BeginChild(XorStr("skinchanger"), ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar);
	{
		if (ImGui::BeginMenuBar())
		{
			ImGui::TextUnformatted(XorStr("skinchanger"));
			ImGui::EndMenuBar();
		}

		ImGui::Columns(2, nullptr, false);
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, -1.f));

			ImGui::PushItemWidth(106.f);
			ImGui::Combo("##1", &C::Get<int>(Vars.iSkinchangerWeapon),
				[](void* data, int idx, const char** out_text)
				{
					*out_text = arrWeaponNames[idx].second;
					return true;
				}, nullptr, 36, 8); int iIndex = arrWeaponNames[C::Get<int>(Vars.iSkinchangerWeapon)].first;

			SkinchangerVariables_t& WeaponVars = C::Get<std::map<int, SkinchangerVariables_t>>(Vars.mapSkinchangerVars)[iIndex];
			WeaponVars.iDefinitionIndex = iIndex;

			if (iIndex == WEAPON_KNIFE) // if knife is selected
			{
				ImGui::SameLine(125.f);
				ImGui::Combo("##2", &C::Get<int>(Vars.iSkinchangerKnife),
					[](void* data, int idx, const char** out_text)
					{
						*out_text = arrKnifeNames[idx].second;
						return true;
					}, nullptr, 20, 8); WeaponVars.iDefinitionIndexOverride = arrKnifeNames[C::Get<int>(Vars.iSkinchangerKnife)].first;
			}
			else if (iIndex == GLOVE_T)
			{
				ImGui::SameLine(125.f);
				ImGui::Combo("##2", &C::Get<int>(Vars.iSkinchangerGlove),
					[](void* data, int idx, const char** out_text)
					{
						*out_text = arrGlovesNames[idx].second;
						return true;
					}, nullptr, 8, 8); WeaponVars.iDefinitionIndexOverride = arrGlovesNames[C::Get<int>(Vars.iSkinchangerGlove)].first;
			}

			ImGui::PopItemWidth();

			ImGui::PushItemWidth(-1.f);

			ImGui::ListBox("##3", &WeaponVars.iPaintKitIndex,
				[](void* data, int idx, const char** out_text)
				{
					*out_text = arrWeaponNames[C::Get<int>(Vars.iSkinchangerWeapon)].first == GLOVE_T ? CSkinChanger::Get().vecGloveKits[idx].szName.data() : CSkinChanger::Get().vecSkinKits[idx].szName.data();
					return true;
				}, nullptr, iIndex == GLOVE_T ? CSkinChanger::Get().vecGloveKits.size() : CSkinChanger::Get().vecSkinKits.size(), 13); 
			WeaponVars.iPaintKit = iIndex == GLOVE_T ? CSkinChanger::Get().vecGloveKits[WeaponVars.iPaintKitIndex].iId : CSkinChanger::Get().vecSkinKits[WeaponVars.iPaintKitIndex].iId;

			ImGui::PopStyleVar();
		}
		ImGui::NextColumn();
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, -1.f));

			SkinchangerVariables_t& WeaponVars = C::Get<std::map<int, SkinchangerVariables_t>>(Vars.mapSkinchangerVars)[arrWeaponNames[C::Get<int>(Vars.iSkinchangerWeapon)].first];

			ImGui::Checkbox(XorStr("enable"), &WeaponVars.bEnabled);
			ImGui::SliderInt(XorStr("seed"), &WeaponVars.iSeed, 0, 1000);
			ImGui::SliderFloat(XorStr("wear"), &WeaponVars.flWear, 0.f, 1.f, "%.5f");

			if (arrWeaponNames[C::Get<int>(Vars.iSkinchangerWeapon)].first != GLOVE_T)
			{
				ImGui::Checkbox(XorStr("stattrak"), &WeaponVars.bStatTrak);
				if (WeaponVars.bStatTrak)
					ImGui::InputInt("", &WeaponVars.iStatTrak);

				ImGui::Checkbox(XorStr("nametag"), &WeaponVars.bNameTag);
				if (WeaponVars.bNameTag)
					ImGui::InputText(XorStr("text"), &WeaponVars.szNameTag);
			}

			ImGui::PopStyleVar();
		}
		ImGui::Columns(1);

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, 0.5f));

		ImGui::Separator();
		if (ImGui::Button(XorStr("update"), ImVec2(-1, 0)))
			U::ForceFullUpdate();

		ImGui::PopStyleVar();

		ImGui::EndChild();
	}
}
#pragma endregion
