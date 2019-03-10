#pragma once
#include <BWAPI.h>

#include <BWTA.h>
#include <windows.h>

extern bool analyzed;
extern bool analysis_just_finished;
extern BWTA::Region* home;
extern BWTA::Region* enemy_base;
DWORD WINAPI AnalyzeThread();
DWORD workerBuildProjectThread(LPVOID lpParam);

using namespace BWAPI;
using namespace BWTA;

struct ProjectItem {
	UnitType unit;
	bool IsUnit;
	TechType tech;
	ProjectItem(UnitType ut, bool iu = true, TechType tt = NULL)
	{
		unit = ut;
		IsUnit = iu;
		tech = tt;
	}
};

struct BuildProjectParams {
	Unit worker;
	UnitType toBuild;
	TilePosition suggestedLocation;
	BuildProjectParams(Unit w, UnitType t, TilePosition s)
	{
		this->worker = w;
		this->toBuild = t;
		this->suggestedLocation = s;
	}
};

class ExampleAIModule : public BWAPI::AIModule
{
public:
	//Methods inherited from BWAPI:AIModule
	virtual void onStart();
	virtual void onEnd(bool isWinner);
	virtual void onFrame();
	virtual void onSendText(std::string text);
	virtual void onReceiveText(BWAPI::Player player, std::string text);
	virtual void onPlayerLeft(BWAPI::Player player);
	virtual void onNukeDetect(BWAPI::Position target);
	virtual void onUnitDiscover(BWAPI::Unit unit);
	virtual void onUnitEvade(BWAPI::Unit unit);
	virtual void onUnitShow(BWAPI::Unit unit);
	virtual void onUnitHide(BWAPI::Unit unit);
	virtual void onUnitCreate(BWAPI::Unit unit);
	virtual void onUnitDestroy(BWAPI::Unit unit);
	virtual void onUnitMorph(BWAPI::Unit unit);
	virtual void onUnitRenegade(BWAPI::Unit unit);
	virtual void onSaveGame(std::string gameName);
	virtual void onUnitComplete(BWAPI::Unit unit);

	//Own methods
	void drawStats();
	void drawDebugText();
	void drawTerrainData();
	void showPlayers();
	void showForces();
	void attackClosest(BWAPI::Unit unit, bool shiftQueueCommand = false);
	Position findGuardPoint();

};
