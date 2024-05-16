#include "BasicSc2Bot.h"
#include <iostream>
BasicSc2Bot::BasicSc2Bot() : econ(*this)
{
}

void BasicSc2Bot::OnGameStart()
{
	home = Observation()->GetStartLocation();
	mapCenter = Point2D(Observation()->GetGameInfo().width / 2, Observation()->GetGameInfo().height / 2);
	econ.setFirstPylonLocation();
	gameStartTime = std::chrono::steady_clock::now();
	ourID = Observation()->GetPlayerID();

}

void BasicSc2Bot::OnStep()
{
    econ.OnStep();

    gameCurrentTime = std::chrono::steady_clock::now();
    auto totalElapsedTime = std::chrono::duration_cast<std::chrono::seconds>(gameCurrentTime - gameStartTime);

    // draw condition for testing

    /*	cout << totalElapsedTime.count() * 9 << std::endl;
        if (totalElapsedTime.count() * 9 >= timeLimit) {
            auto totalAttackTime = std::chrono::duration_cast<std::chrono::seconds>(attackTime - gameStartTime);
            std::cout << "draw" << std::endl;
            std::cout << "totalAttackTime: " << totalAttackTime.count() * 9 << " seconds" << std::endl;
            std::cout << "totalGameTime: " << timeLimit << " seconds" << std::endl;
            exit(0);
        }			*/

    // if its been around 10 minutes in game then we move on to endGame attack which is just attacking enemy structures we are able to see
    // good for if all enemies are revealed so we don't just keep attacking the enemy base location

    if (totalElapsedTime.count() * 9 >= attackRevealedTime) {
        earlyGame = false;
        Units units = Observation()->GetUnits(sc2::Unit::Alliance::Self);
        Units enemyStructures = Observation()->GetUnits(sc2::Unit::Alliance::Enemy);
        if (enemyStructures.size() > 0) {

            for (const auto& unit : units) {

                if (unit->unit_type == sc2::UNIT_TYPEID::PROTOSS_STALKER) {
                    Actions()->UnitCommand(unit, sc2::ABILITY_ID::ATTACK_ATTACK, enemyStructures.front()->pos);
                }
            }

        }

    }

}


void BasicSc2Bot::OnUnitIdle(const sc2::Unit* unit)
{
    switch (unit->unit_type.ToType())
    {
        case sc2::UNIT_TYPEID::PROTOSS_NEXUS:
        {   // make new probe
            if (Observation()->GetFoodUsed() <= 22)
                econ.TrainProbe(unit);

            break;
        }
        case sc2::UNIT_TYPEID::PROTOSS_PROBE: 
        {
            // go to nearest mineralpatch from our base
            const sc2::Unit* mineral_target = econ.FindNearestMineralPatch(home);
            if (!mineral_target) {
                break;
            }
            Actions()->UnitCommand(unit, sc2::ABILITY_ID::SMART, mineral_target);

            break;
        }
        case UNIT_TYPEID::PROTOSS_STALKER:
        {
            // if we have 4 stakers or more and its been less than 10 minutes in the game go attack enemy base
            if (amountStalkersCreated >= 4 && earlyGame) {
                Actions()->UnitCommand(unit, ABILITY_ID::ATTACK_ATTACK, enemy_base);

            }

            break;
        }
        case UNIT_TYPEID::PROTOSS_GATEWAY:
        {
            // if we have 2 stalkers created, create more from the idle gateways
            if (amountStalkersCreated == 2) {
                auto Gateways = econ.getGateways();
                for (size_t i = 0; i < Gateways.size(); ++i) {
                    if (unit == Gateways[i])
                        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_STALKER);
                }
            }

            break;
        }
        case UNIT_TYPEID::PROTOSS_CYBERNETICSCORE:
        {
            // research warpgate
            Actions()->UnitCommand(unit, ABILITY_ID::RESEARCH_WARPGATE);

            break;
        }
        case UNIT_TYPEID::PROTOSS_WARPGATE:
        {

            // warp stalkers near the proxy pylon
            Point2D location = econ.getproxypylonLocation();
            location.x = location.x + GetRandomScalar() * 2.0f;
            location.y = location.y + GetRandomScalar() * 2.0f;
            Actions()->UnitCommand(unit, ABILITY_ID::TRAINWARP_STALKER, location);

            break;
        }

        default:
        {
            break;
        }
    }
}

void BasicSc2Bot::OnUnitCreated(const sc2::Unit* unit)
{
    switch (unit->unit_type.ToType())
    {
        case UNIT_TYPEID::PROTOSS_STALKER:
        {
            // increment stalker count and add the stalker to army vector
            ++amountStalkersCreated;
            econ.addArmy1(unit);

            // if we created 4 or more stalkers and its been less than 10 minutes then attack enemy base
            if (amountStalkersCreated >= 4 && earlyGame) {
                for (const auto& stalker : econ.getArmy1()) {
                    if (enemy_base.x != 0) {
                        Actions()->UnitCommand(stalker, ABILITY_ID::ATTACK_ATTACK, enemy_base);
                        if (!attackReady) {
                            attackReady = true;
                            attackTime = std::chrono::steady_clock::now();          // attack time for gathering statistical data
                        }
                    }
                }
            }

            break;
        }
        case sc2::UNIT_TYPEID::PROTOSS_GATEWAY:
        {
            // increment gateway count 
            ++amountGatewaysCreated;

            // if we have less than or equal to 2 gateways created add them to the gateway vector
            if (amountGatewaysCreated <= 2) {
                econ.addGateway(unit);
            }
            if (amountGatewaysCreated == 3) {
                econ.setbuildLastGatewayToTrue();
            }
            if (amountGatewaysCreated == 4) {
                econ.setbuildLastGatewayToFalse();
            }
            break;
        }

        default:
        {
            break;
        }
    }
}

void BasicSc2Bot::OnUnitDestroyed(const sc2::Unit* unit)
{
    switch (unit->unit_type.ToType())
    {

        case sc2::UNIT_TYPEID::PROTOSS_PROBE:
        {
            // if setEnemyBase is true then we get the location of the enemy base by finding the closest enemy base location from our death position using eulicidean distance
            if (econ.getsetEnemyBase()) {
                econ.setEnemyBaseToFalse();
                Point2D unitPos = unit->pos;

                const sc2::GameInfo& game_info = Observation()->GetGameInfo();
                float minimumDistance = std::numeric_limits<float>::max();
                for (const auto& u : game_info.enemy_start_locations) {
                    float d = DistanceSquared2D(u, unitPos);
                    if (d < minimumDistance) {
                        minimumDistance = d;
                        enemy_base = u;
                    }

                }


            }

            break;
        }

        default:
        {
            break;
        }
    }
}

void BasicSc2Bot::OnUnitEnterVision(const sc2::Unit* unit) {

    // set enemy base location if we see their town hall, and if setEnemyBase is true
    if (unit->unit_type == UNIT_TYPEID::TERRAN_COMMANDCENTER ||
        unit->unit_type == UNIT_TYPEID::TERRAN_ORBITALCOMMAND ||
        unit->unit_type == UNIT_TYPEID::TERRAN_COMMANDCENTERFLYING ||
        unit->unit_type == UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING ||
        unit->unit_type == UNIT_TYPEID::TERRAN_PLANETARYFORTRESS ||
        unit->unit_type == UNIT_TYPEID::PROTOSS_NEXUS ||
        unit->unit_type == UNIT_TYPEID::ZERG_HATCHERY ||
        unit->unit_type == UNIT_TYPEID::ZERG_LAIR ||
        unit->unit_type == UNIT_TYPEID::ZERG_HIVE) {
        if (econ.getsetEnemyBase()) {
            econ.setEnemyBaseToFalse();
            Point2D enemy_townHall = unit->pos;

            const sc2::GameInfo& game_info = Observation()->GetGameInfo();
            float minimumDistance = std::numeric_limits<float>::max();
            for (const auto& u : game_info.enemy_start_locations) {
                float d = DistanceSquared2D(u, enemy_townHall);
                if (d < minimumDistance) {
                    minimumDistance = d;
                    enemy_base = u;
                }

            }


        }

    }

}



void BasicSc2Bot::OnBuildingConstructionComplete(const sc2::Unit* unit)
{
    switch (unit->unit_type.ToType())
    {

        case sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE:
        {
            // train stalkers and set our chronoBoost to true once cyber core is done building
            econ.TrainStalker();
            econ.setchronoBoostCoreTrue();
            econ.chronoBoost(unit);
            break;
        }

        default:
        {
            break;
        }
    }
}

void BasicSc2Bot::OnUpgradeCompleted(UpgradeID upgrade)
{

    switch (upgrade.ToType())
    {

        case sc2::UPGRADE_ID::WARPGATERESEARCH:
        {
            // once we finish researching warpgate we set our chronoBoost to false
            econ.setchronoBoostCoreFalse();
            break;
        }

        default:
        {
            break;
        }
    }
}


void BasicSc2Bot::OnGameEnd()
{
    // this is for collecting statistical data
    // getting results from who won and how long it took to start an attack and how long the game lasted

	gameEndTime = std::chrono::steady_clock::now();
	auto totalAttackTime = std::chrono::duration_cast<std::chrono::seconds>(attackTime - gameStartTime);
	auto totalGameTime = std::chrono::duration_cast<std::chrono::seconds>(gameEndTime - gameStartTime);
	auto results = Observation()->GetResults();
	for (auto x : results) {

		if (x.player_id == ourID) {
			if (x.result == 0) {
				std::cout << botName << " wins" << std::endl;
			}
			else {
				std::cout << "AI wins" << std::endl;
			}
		}
	}
	std::cout << "totalAttackTime: " << totalAttackTime.count() * 9 << " seconds" << std::endl;
	std::cout << "totalGameTime: " << totalGameTime.count() * 9 << " seconds" << std::endl;
}