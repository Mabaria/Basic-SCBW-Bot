#include "ExampleAIModule.h" 
#include <thread>
#include <chrono>
using namespace BWAPI;

bool analyzed;
bool analysis_just_finished;
BWTA::Region* home;
BWTA::Region* enemy_base;
int reservedMinerals = 0;
int reservedGas = 0;
int incomingSupply = 0;
std::vector<ProjectItem> projectQueue;
Position currentObjectivePos;
bool allOutAttacked = false;
//This is the startup method. It is called once
//when a new game has been started with the bot.
void ExampleAIModule::onStart()
{
	// Increase speed of the game
	Broodwar->setLocalSpeed(5);
	Broodwar->sendText("Hello world!");
	//Enable flags
	Broodwar->enableFlag(Flag::UserInput);
	//Uncomment to enable complete map information
	//Broodwar->enableFlag(Flag::CompleteMapInformation);

	//Start analyzing map data
	BWTA::readMap();
	analyzed = false;
	analysis_just_finished = false;
	//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL); //Threaded version
	AnalyzeThread();

	//Send each worker to the mineral field that is closest to it
	//Also set rally point for command center to minerals
	for (auto u : Broodwar->self()->getUnits())
	{
		if (u->getType().isWorker() || u->getType().isResourceDepot())
		{
			Unit closestMineral = NULL;
			for (auto m : Broodwar->getMinerals())
			{
				if (closestMineral == NULL || u->getDistance(m) < u->getDistance(closestMineral))
				{
					closestMineral = m;
				}
			}
			if (closestMineral != NULL)
			{
				u->rightClick(closestMineral);
				Broodwar->printf("Send worker %d to mineral %d", u->getID(), closestMineral->getID());
			}
		}
	}

	Position dir = findGuardPoint() - home->getCenter();
	dir.x *= 0.2;
	dir.y *= 0.2;
	currentObjectivePos = findGuardPoint() - dir;

	projectQueue.push_back(ProjectItem(UnitTypes::Terran_SCV));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_SCV));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_SCV));


	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Barracks));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Bunker));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Supply_Depot));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Marine));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Marine));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Marine));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_SCV));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Supply_Depot));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Marine));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Marine));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Marine));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Refinery));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Academy));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_SCV));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Marine));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Marine));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_SCV));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Marine));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_SCV));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Supply_Depot));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Marine));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Firebat));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Firebat));
	projectQueue.push_back(UnitTypes::Terran_Medic);
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_SCV));
	projectQueue.push_back(UnitTypes::Terran_Medic);
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Firebat));
	projectQueue.push_back(UnitTypes::Terran_SCV);
	projectQueue.push_back(UnitTypes::Terran_Medic);
	projectQueue.push_back(UnitTypes::Terran_Factory);
	projectQueue.push_back(UnitTypes::Terran_SCV);
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Engineering_Bay));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Bunker));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_SCV));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Supply_Depot));
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Missile_Turret));
	projectQueue.push_back(UnitTypes::Terran_Machine_Shop);
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Barracks));
	projectQueue.push_back(UnitTypes::Terran_Siege_Tank_Tank_Mode);
	projectQueue.push_back(ProjectItem(UnitTypes::Terran_Starport));
	projectQueue.push_back(UnitTypes::Terran_Siege_Tank_Tank_Mode);
	projectQueue.push_back(UnitTypes::Terran_Siege_Tank_Tank_Mode);
	projectQueue.push_back(ProjectItem(NULL, false, TechTypes::Tank_Siege_Mode));
	projectQueue.push_back(UnitTypes::Terran_Wraith);
	projectQueue.push_back(UnitTypes::Terran_Wraith);
	projectQueue.push_back(UnitTypes::Terran_Wraith);

	for (int i = 0; i < 1; i++)
	{
		projectQueue.push_back(ProjectItem(UnitTypes::Terran_Marine));
		projectQueue.push_back(ProjectItem(UnitTypes::Terran_Marine));
		projectQueue.push_back(ProjectItem(UnitTypes::Terran_Marine));
		projectQueue.push_back(ProjectItem(UnitTypes::Terran_Marine));
		projectQueue.push_back(ProjectItem(UnitTypes::Terran_Medic));
	}

	projectQueue.push_back(UnitTypes::Unknown);


}

//Called when a game is ended.
//No need to change this.
void ExampleAIModule::onEnd(bool isWinner)
{
	if (isWinner)
	{
		Broodwar->sendText("I won!");
	}
}

//Finds a guard point around the home base.
//A guard point is the center of a chokepoint surrounding
//the region containing the home base.
Position ExampleAIModule::findGuardPoint()
{
	//Get the chokepoints linked to our home region
	std::set<BWTA::Chokepoint*> chokepoints = home->getChokepoints();
	double min_length = 10000;
	BWTA::Chokepoint* choke = NULL;

	//Iterate through all chokepoints and look for the one with the smallest gap (least width)
	for (std::set<BWTA::Chokepoint*>::iterator c = chokepoints.begin(); c != chokepoints.end(); c++)
	{
		double length = (*c)->getWidth();
		if (length < min_length || choke == NULL)
		{
			min_length = length;
			choke = *c;
		}
	}

	return choke->getCenter();
}



//This is the method called each frame. This is where the bot's logic
//shall be called.
void ExampleAIModule::onFrame()
{
	//Call every 100:th frame
	if (Broodwar->getFrameCount() % 100 == 0)
	{

		// As long as items remain in the build order
		if (projectQueue.begin()->unit != UnitTypes::Unknown)
		{
			// If we accidentally supply cap, build supply
			if (Broodwar->self()->supplyUsed() + 2 >= Broodwar->self()->supplyTotal() + incomingSupply)
			{
				projectQueue.insert(projectQueue.begin(), UnitTypes::Terran_Supply_Depot);
			}


			// Check if we have enough minerals to build, if so send one worker to do this
			int availableMinerals = Broodwar->self()->minerals() - reservedMinerals;
			int availableGas = Broodwar->self()->gas() - reservedGas;
			if (projectQueue.begin()->IsUnit)
			{
				UnitType nextProject = projectQueue.begin()->unit;

				if (availableMinerals >= nextProject.mineralPrice() && availableGas >= nextProject.gasPrice()) {

					for (auto u : Broodwar->self()->getUnits())
					{
						if (nextProject.isAddon()) {
							// Find suitable building to put the addon on
							if (u->getType() == nextProject.whatBuilds().first && u->canBuildAddon())
							{
								// Reserving minerals for semantic reasons
								reservedMinerals += nextProject.mineralPrice();
								reservedGas += nextProject.gasPrice();
								u->buildAddon(nextProject);
								projectQueue.erase(projectQueue.begin());
							}
						}
						else if (nextProject.isBuilding()) {
							//Check if unit is a worker.
							if (u->getType().isWorker())
							{
								// Find one that isn't busy constructing
								if (u->isGatheringMinerals() || u->isIdle()) {
									TilePosition desiredPos;
									TilePosition toBuild;

									if (nextProject == UnitTypes::Terran_Bunker || nextProject == UnitTypes::Terran_Missile_Turret)
									{
										toBuild = Broodwar->getBuildLocation(nextProject, (TilePosition)findGuardPoint(), 1000);

									}
									else
									{

										desiredPos = u->getClosestUnit(Filter::IsResourceDepot)->getTilePosition();
										toBuild = Broodwar->getBuildLocation(nextProject, desiredPos, 100);

									}



									// Reserve minerals for the build
									reservedMinerals += nextProject.mineralPrice();
									reservedGas += nextProject.gasPrice();
									if (nextProject == UnitTypes::Terran_Supply_Depot)
									{
										incomingSupply += 8;
									}
									// Assign worker to build, queue up gathering once finished
									/*u->build(nextProject, toBuild);
									u->gather(u->getClosestUnit(Filter::IsMineralField || Filter::IsRefinery), true);*/
									BuildProjectParams* projectParams = new BuildProjectParams(u, nextProject, toBuild);
									CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)workerBuildProjectThread, (LPVOID)projectParams, 0, NULL); //Threaded version
									projectQueue.erase(projectQueue.begin()); // Build thread started, move on

									break;
								}
							}
						}

						else // Is not a building
						{
							// Find the building that trains this unit
							if (u->getType() == nextProject.whatBuilds().first && Broodwar->canMake(nextProject))
							{
								// u->getTrainingQueue().max_size
								if (u->getTrainingQueue().size() < 5)
								{
									// Broodwar->printf("Unit can be trained! Removing from projectQueue");
									// Broodwar->printf("Queue size: %i", u->getTrainingQueue().size());
									u->train(nextProject);
									projectQueue.erase(projectQueue.begin());
								}

							}
						}

					}
				}
			}
			// Project is not a unit, it is tech
			else
			{
				TechType nextProject = projectQueue.begin()->tech;
				for (auto u : Broodwar->self()->getUnits())
				{
					if (u->getType() == nextProject.whatResearches() && Broodwar->canResearch(nextProject))
					{
						// u->getTrainingQueue().max_size
						if (u->getTrainingQueue().size() < 5)
						{
							// Broodwar->printf("Unit can be trained! Removing from projectQueue");
							// Broodwar->printf("Queue size: %i", u->getTrainingQueue().size());
							u->research(nextProject);
							projectQueue.erase(projectQueue.begin());
						}

					}
				}
			}
		}
		else // ProjectQueue = unknown, move in for the strike
		{
			if (!allOutAttacked)
			{
				for (auto a : BWTA::getStartLocations())
				{
					if (a->getRegion() != home)
					{
						currentObjectivePos = a->getRegion()->getCenter();
						break;
					}
				}
				allOutAttacked = true;
			}
				
		}
		// Tell all idle workers to gather
		for (auto u : Broodwar->self()->getUnits())
		{
			//Check if unit is a worker.
			if (u->getType().isWorker())
			{
				if (u->isIdle()) {
					u->gather(u->getClosestUnit(Filter::IsMineralField || Filter::IsRefinery));
				}
			}
			// Tell marines units to attack closest target
			else if (u->getType() == UnitTypes::Terran_Marine || u->getType() == UnitTypes::Terran_Firebat)
			{
				if (u->getHitPoints() < u->getType().maxHitPoints())
				{
					if (u->isUnderAttack())
					{
						if (!u->isBeingHealed())
						{
							u->move(home->getCenter());
						}
					}
					else
					{
						u->attack(currentObjectivePos);
					}

				}
				attackClosest(u, false);
			}
			// Tell medics to heal closest damaged unit
			else if (u->getType() == UnitTypes::Terran_Medic)
			{
				Unit toHeal = u->getClosestUnit((Filter::IsAlly && Filter::IsOrganic && Filter::HP_Percent < 100), 500);
				if (toHeal != NULL)
				{
					//Broodwar->printf("MEDIC FOUND UNIT TO HEAL");
					u->rightClick(toHeal);
				}
				else
				{
					u->attack(currentObjectivePos);
				}
				
			}
			// Tell bunkers to attack closest target, and load nearby marines if not full
			else if (u->getType() == UnitTypes::Terran_Bunker)
			{
				if (u->getSpaceRemaining() > 0)
				{
					int spaceLeft = u->getSpaceRemaining();
					// Load nearby marines
					for (auto n : u->getUnitsInRadius(500, Filter::IsAlly))
					{
						if (n->getType() == UnitTypes::Terran_Marine)
						{
							n->load(u);
							spaceLeft--;
						}
						if (spaceLeft <= 0)
						{
							break;
						}
					}
				}
				attackClosest(u, false);
			}
			// Tell siege tanks to siege
			else if (u->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode || u->getType() == UnitTypes::Terran_Siege_Tank_Siege_Mode)
			{
				Unit closestEnemy = u->getClosestUnit(Filter::IsEnemy, 1500);
				if (closestEnemy != NULL)
				{
					if (u->canSiege())
						u->siege();
					else
						u->attack(closestEnemy);
				}
				else
				{
					if (u->isSieged())
					{
						u->unsiege();
					}
					u->attack(currentObjectivePos);
				}
			}
		}

		
	}

	//Draw lines around regions, chokepoints etc.
	if (analyzed)
	{
		drawTerrainData();
	}

	drawDebugText();
}

//Is called when text is written in the console window.
//Can be used to toggle stuff on and off.
void ExampleAIModule::onSendText(std::string text)
{
	if (text == "/show players")
	{
		showPlayers();
	}
	else if (text == "/show forces")
	{
		showForces();
	}
	else
	{
		Broodwar->printf("You typed '%s'!", text.c_str());
		Broodwar->sendText("%s", text.c_str());
	}
}

//Called when the opponent sends text messages.
//No need to change this.
void ExampleAIModule::onReceiveText(BWAPI::Player player, std::string text)
{
	Broodwar->printf("%s said '%s'", player->getName().c_str(), text.c_str());
}

//Called when a player leaves the game.
//No need to change this.
void ExampleAIModule::onPlayerLeft(BWAPI::Player player)
{
	Broodwar->sendText("%s left the game.", player->getName().c_str());
}

//Called when a nuclear launch is detected.
//No need to change this.
void ExampleAIModule::onNukeDetect(BWAPI::Position target)
{
	if (target != Positions::Unknown)
	{
		Broodwar->printf("Nuclear Launch Detected at (%d,%d)", target.x, target.y);
	}
	else
	{
		Broodwar->printf("Nuclear Launch Detected");
	}
}

//No need to change this.
void ExampleAIModule::onUnitDiscover(BWAPI::Unit unit)
{
	//Broodwar->sendText("A %s [%x] has been discovered at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x,unit->getPosition().y);
}

//No need to change this.
void ExampleAIModule::onUnitEvade(BWAPI::Unit unit)
{
	//Broodwar->sendText("A %s [%x] was last accessible at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x,unit->getPosition().y);
}

//No need to change this.
void ExampleAIModule::onUnitShow(BWAPI::Unit unit)
{
	//Broodwar->sendText("A %s [%x] has been spotted at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x,unit->getPosition().y);
}

//No need to change this.
void ExampleAIModule::onUnitHide(BWAPI::Unit unit)
{
	//Broodwar->sendText("A %s [%x] was last seen at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x,unit->getPosition().y);
}

//Called when a new unit has been created.
//Note: The event is called when the new unit is built, not when it
//has been finished.
void ExampleAIModule::onUnitCreate(BWAPI::Unit unit)
{
	if (unit->getPlayer() == Broodwar->self())
	{
		//Broodwar->sendText("A %s [%x] has been created at (%d,%d)", unit->getType().getName().c_str(), unit, unit->getPosition().x, unit->getPosition().y);
		if (Broodwar->getFrameCount() > 500) // prevents crazy things from happening with start of match unit creations
		{
			if (unit->getType().isBuilding()) {

				reservedMinerals -= unit->getType().mineralPrice();
				reservedGas -= unit->getType().gasPrice();
			}
			//// Find the unit in the project queue and remove it, if it is not a building, those are handled when threads are launched
			//if (!unit->getType().isBuilding())
			//{
			//	for (auto& a = projectQueue.begin(); a != projectQueue.end(); a++)
			//	{
			//		if (*a == unit->getType()) {
			//			projectQueue.erase(a);
			//			break;
			//		}
			//	}
			//}

		}
	}
}

//Called when a unit has been destroyed.
void ExampleAIModule::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getPlayer() == Broodwar->self())
	{
		Broodwar->sendText("My unit %s [%x] has been destroyed at (%d,%d)", unit->getType().getName().c_str(), unit, unit->getPosition().x, unit->getPosition().y);
	}
	else
	{
		Broodwar->sendText("Enemy unit %s [%x] has been destroyed at (%d,%d)", unit->getType().getName().c_str(), unit, unit->getPosition().x, unit->getPosition().y);
	}
}

//Only needed for Zerg units.
/// THATS A LIE! Refineries and siege tanks also morph
//No need to change this.
void ExampleAIModule::onUnitMorph(BWAPI::Unit unit)
{
	// Because of how refineries are created, unreserve minerals here
	if (unit->getType() == UnitTypes::Terran_Refinery && unit->getPlayer() == Broodwar->self()) {
		reservedMinerals -= UnitTypes::Terran_Refinery.mineralPrice();
	}
	//Broodwar->sendText("A %s [%x] has been morphed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x,unit->getPosition().y);

}

//No need to change this.
void ExampleAIModule::onUnitRenegade(BWAPI::Unit unit)
{

	//Broodwar->sendText("A %s [%x] is now owned by %s",unit->getType().getName().c_str(),unit,unit->getPlayer()->getName().c_str());
}

//No need to change this.
void ExampleAIModule::onSaveGame(std::string gameName)
{
	Broodwar->printf("The game was saved to \"%s\".", gameName.c_str());
}

//Analyzes the map.
//No need to change this.
DWORD WINAPI AnalyzeThread()
{
	BWTA::analyze();

	//Self start location only available if the map has base locations
	if (BWTA::getStartLocation(BWAPI::Broodwar->self()) != NULL)
	{
		home = BWTA::getStartLocation(BWAPI::Broodwar->self())->getRegion();
	}
	//Enemy start location only available if Complete Map Information is enabled.
	if (BWTA::getStartLocation(BWAPI::Broodwar->enemy()) != NULL)
	{
		enemy_base = BWTA::getStartLocation(BWAPI::Broodwar->enemy())->getRegion();
	}
	analyzed = true;
	analysis_just_finished = true;
	return 0;
}

// Thread function for active building project
// Will run until the project is completed
DWORD workerBuildProjectThread(LPVOID lpParam)
{

	BuildProjectParams* params = (BuildProjectParams*)lpParam;
	Broodwar->printf("Building %s", params->toBuild.c_str());

	while(!Broodwar->canMake(params->toBuild, params->worker))
	{
		Broodwar->printf("%s requirements not met, waiting", params->toBuild.c_str());
		std::this_thread::sleep_for(std::chrono::seconds(1));

	}
	while (params->worker->isConstructing() || !params->worker->exists()) // Worker got snatched up by some other thread, get a new one
	{
		Broodwar->printf("Finding new worker to build %s", params->toBuild.c_str());
		params->worker = params->worker->getClosestUnit(Filter::IsWorker && Filter::IsGatheringMinerals);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	// Unit worker, UnitType toBuild, TilePosition suggestedLocation

	//// Deposit cargo if deposit is close
	//if (params->worker->canReturnCargo())
	//{
	//	Unit depot = params->worker->getClosestUnit(Filter::IsResourceDepot);
	//	if (params->worker->getDistance(depot) < 500)
	//	{
	//		Broodwar->printf("Returning cargo first, please hold");
	//		while (params->worker->canReturnCargo())
	//		{
	//			params->worker->returnCargo();
	//		}
	//	}
	//}
	params->worker->move((Position)params->suggestedLocation);
	// Keep trying to build until construction starts
	while (params->worker->getBuildUnit() == nullptr)
	{
		if (!params->worker->isConstructing())
		{
			Broodwar->printf("Initiating construction of %s by %s %i, please hold", params->toBuild.c_str(), params->worker->getType().c_str(), params->worker->getID());
			if (params->suggestedLocation == TilePositions::Invalid)
			{
				params->suggestedLocation = Broodwar->getBuildLocation(params->toBuild, params->suggestedLocation);
				Broodwar->printf("%s failed. TilePos Invalid. Retrying...", params->toBuild.c_str());
			}
			else
			{
				params->worker->build(params->toBuild, params->suggestedLocation);
			}
		}
		else
		{
			Broodwar->printf("%s %i moving to construct %s, please hold", params->worker->getType().c_str(), params->worker->getID(), params->toBuild.c_str());
		}

		//Error lastError = Broodwar->getLastError();
		//Broodwar->setLastError(); // Clear the parsed error
		//if (lastError == Errors::Insufficient_Space || lastError == Errors::Unbuildable_Location || lastError == Errors::Unreachable_Location) // Problem with the location, find new location
		//{
		//	//Broodwar->printf("%s failed. %s Retrying...", params->toBuild.c_str(), lastError.c_str());

		//}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	params->worker->gather(params->worker->getClosestUnit(Filter::IsMineralField || Filter::IsRefinery), true); // Send the worker back to gather once he's done
	//while (params->worker->isConstructing())
	//{
	//	std::this_thread::sleep_for(std::chrono::seconds(1));
	//}

	
	Broodwar->printf("Build project under way!");

	delete lpParam;
	return 0;
}

//Prints some stats about the units the player has.
//No need to change this.
void ExampleAIModule::drawStats()
{
	BWAPI::Unitset myUnits = Broodwar->self()->getUnits();
	Broodwar->drawTextScreen(5, 0, "I have %d units:", myUnits.size());
	std::map<UnitType, int> unitTypeCounts;
	for (auto u : myUnits)
	{
		if (unitTypeCounts.find(u->getType()) == unitTypeCounts.end())
		{
			unitTypeCounts.insert(std::make_pair(u->getType(), 0));
		}
		unitTypeCounts.find(u->getType())->second++;
	}
	int line = 1;
	for (std::map<UnitType, int>::iterator i = unitTypeCounts.begin(); i != unitTypeCounts.end(); i++)
	{
		Broodwar->drawTextScreen(5, 16 * line, "- %d %ss", i->second, i->first.getName().c_str());
		line++;
	}
}

void ExampleAIModule::drawDebugText() {
	// Draw total reserved minerals
	if(projectQueue.begin()->IsUnit)
		Broodwar->drawTextScreen(5, 16, "Current project: %s", projectQueue.begin()->unit.c_str());
	else
		Broodwar->drawTextScreen(5, 16, "Current project: %s", projectQueue.begin()->tech.c_str());
	Broodwar->drawTextScreen(400, 16, "Reserved minerals: %d", reservedMinerals);
	Broodwar->drawTextScreen(400, 32, "Reserved gas: %d", reservedGas);
	Broodwar->drawTextScreen(550, 16, "Incoming Supply: %d", incomingSupply);
	Broodwar->drawTextMap(findGuardPoint(), "Guard point here %i , %i", findGuardPoint().x, findGuardPoint().y);
	for (auto u : Broodwar->self()->getUnits())
	{
		if (u->getType() == UnitTypes::Terran_SCV)
		{
			Broodwar->drawTextMap(u->getPosition(), "%i", u->getID());
		}
		else if (u->getType() == UnitTypes::Terran_Command_Center)
		{
			Broodwar->drawTextMap(u->getPosition(), "%i , %i", u->getPosition().x, u->getPosition().y);
		}
	}


}

//Draws terrain data aroung regions and chokepoints.
//No need to change this.
void ExampleAIModule::drawTerrainData()
{
	//Iterate through all the base locations, and draw their outlines.
	for (auto bl : BWTA::getBaseLocations())
	{
		TilePosition p = bl->getTilePosition();
		Position c = bl->getPosition();
		//Draw outline of center location
		Broodwar->drawBox(CoordinateType::Map, p.x * 32, p.y * 32, p.x * 32 + 4 * 32, p.y * 32 + 3 * 32, Colors::Blue, false);
		//Draw a circle at each mineral patch
		for (auto m : bl->getStaticMinerals())
		{
			Position q = m->getInitialPosition();
			Broodwar->drawCircle(CoordinateType::Map, q.x, q.y, 30, Colors::Cyan, false);
		}
		//Draw the outlines of vespene geysers
		for (auto v : bl->getGeysers())
		{
			TilePosition q = v->getInitialTilePosition();
			Broodwar->drawBox(CoordinateType::Map, q.x * 32, q.y * 32, q.x * 32 + 4 * 32, q.y * 32 + 2 * 32, Colors::Orange, false);
		}
		//If this is an island expansion, draw a yellow circle around the base location
		if (bl->isIsland())
		{
			Broodwar->drawCircle(CoordinateType::Map, c.x, c.y, 80, Colors::Yellow, false);
		}
	}
	//Iterate through all the regions and draw the polygon outline of it in green.
	for (auto r : BWTA::getRegions())
	{
		BWTA::Polygon p = r->getPolygon();
		for (int j = 0; j<(int)p.size(); j++)
		{
			Position point1 = p[j];
			Position point2 = p[(j + 1) % p.size()];
			Broodwar->drawLine(CoordinateType::Map, point1.x, point1.y, point2.x, point2.y, Colors::Green);
		}
	}
	//Visualize the chokepoints with red lines
	for (auto r : BWTA::getRegions())
	{
		for (auto c : r->getChokepoints())
		{
			Position point1 = c->getSides().first;
			Position point2 = c->getSides().second;
			Broodwar->drawLine(CoordinateType::Map, point1.x, point1.y, point2.x, point2.y, Colors::Red);
		}
	}
}

//Show player information.
//No need to change this.
void ExampleAIModule::showPlayers()
{
	for (auto p : Broodwar->getPlayers())
	{
		Broodwar->printf("Player [%d]: %s is in force: %s", p->getID(), p->getName().c_str(), p->getForce()->getName().c_str());
	}
}

//Show forces information.
//No need to change this.
void ExampleAIModule::showForces()
{
	for (auto f : Broodwar->getForces())
	{
		BWAPI::Playerset players = f->getPlayers();
		Broodwar->printf("Force %s has the following players:", f->getName().c_str());
		for (auto p : players)
		{
			Broodwar->printf("  - Player [%d]: %s", p->getID(), p->getName().c_str());
		}
	}
}

void ExampleAIModule::attackClosest(BWAPI::Unit unit, bool shiftQueueCommand)
{
	Unit closestEnemy = unit->getClosestUnit(Filter::IsEnemy, 1500);
	if (closestEnemy != NULL)
	{
		//Broodwar->printf("attackClosest found a unit!");
		if (closestEnemy->getDistance(unit) < unit->getType().groundWeapon().maxRange() + 50)
		{
			unit->attack(closestEnemy, shiftQueueCommand);
		}
	}
	else
	{
		unit->attack(currentObjectivePos, shiftQueueCommand);
	}

}

//Called when a unit has been completed, i.e. finished built.
void ExampleAIModule::onUnitComplete(BWAPI::Unit unit)
{
	if (unit->getType() == UnitTypes::Terran_Refinery && unit->getPlayer() == Broodwar->self()) {
		Broodwar->sendText("A refinery has been built, sending SCVs to it.");
		// Tell nearby mineral gatherer to harvest gas
		unit->getClosestUnit(Filter::IsWorker && Filter::IsGatheringMinerals)->gather(unit);
	}
	else if (unit->getType() == UnitTypes::Terran_Barracks || unit->getType() == UnitTypes::Terran_Factory || unit->getType() == UnitTypes::Terran_Starport)
	{
		// Set rally point to guard point
		unit->rightClick(currentObjectivePos);
	}
	else if (unit->getType() == UnitTypes::Terran_Supply_Depot)
	{
		incomingSupply -= 8;
	}
	//Broodwar->sendText("A %s [%x] has been completed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x,unit->getPosition().y);
}
