
#ifndef BASIC_SC2_BOT_H_
#define BASIC_SC2_BOT_H_
#pragma once
#include "Economy.h"
#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"
#include <chrono>

class BasicSc2Bot : public sc2::Agent
{

    // data

    Economy econ;

    Point2D home;
    Point2D mapCenter;

    const std::string botName = "Virtruvius Rex";

    std::chrono::steady_clock::time_point gameStartTime;
    std::chrono::steady_clock::time_point attackTime;               
    std::chrono::steady_clock::time_point gameCurrentTime;
    std::chrono::steady_clock::time_point gameEndTime;

    long long timeLimit = 1200;
    long long attackRevealedTime = 600;

    bool attackReady = false;           // this is for the timer for gathering statistical data of when we started attacking
    bool earlyGame = true;

    int ourID;
    int amountStalkersCreated = 0;
    int amountGatewaysCreated = 0;


public:

    BasicSc2Bot();

    // enemy base location
    Point2D enemy_base = { 0.0,0.0 };

    // center of the map
    Point2D getmapCenter() { return mapCenter;  }

    // Called when a game is started or restarted.
    virtual void OnGameStart();


    virtual void OnStep();

    // Called when a unit becomes idle
    virtual void OnUnitIdle(const Unit* unit);

    // Called when a Unit has been created by the player.
    virtual void OnUnitCreated(const Unit* unit);

    // Called whenever one of the player's units has been destroyed.
    virtual void OnUnitDestroyed(const Unit* unit);

    // Called when an enemy unit enters vision from out of fog of war.
    virtual void OnUnitEnterVision(const Unit* unit);

    virtual void OnBuildingConstructionComplete(const Unit* unit);

    // Called when an upgrade is finished, ex. warp gate
    virtual void OnUpgradeCompleted(UpgradeID upgrade);

    // Called when a game has ended.
    virtual void OnGameEnd();

};

#endif