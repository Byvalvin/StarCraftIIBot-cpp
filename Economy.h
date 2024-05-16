#pragma once

#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"
#include <sc2api/sc2_unit_filters.h>
#include <iostream>

using namespace std;
using namespace sc2;
class BasicSc2Bot;

class Economy
{

    // data

    BasicSc2Bot &Bot;
    string map_name = "";

    bool chronoBoostCore = false;                           // used to tell when to chronoBoost
    bool buildLastGateway = false;                          // used to tell when to build the last gateway
    bool getLocationForLastGateway = false;                 // used to tell when to get the location for last gateway
    bool setOnce = true;                                    // only get the location for the last gateway once
    bool Scouted = false;                                   // use to tell when to scout
    bool setEnemyBase = false;                              // use to tell if we need to set enemy base

    bool doneB00 = false;                                   // doneB00 - B10 are our steps we use in the build order
    bool doneB01 = false;
    bool doneB02 = false;
    bool doneB03 = false;
    bool doneB04 = false;
    bool doneB05 = false;
    bool doneB06 = false;
    bool doneB07 = false;
    bool doneB08 = false;
    bool doneB09 = false;
    bool doneB10 = false;

    Units MineralPatchProbes;                               // 6 probes that are mining minerals
    Units VespeneGasProbes;                                 // 6 probes that are gathering vespene
    Units ClosestGeysers;                                   // 2 of the closest geysers
    Units Gateways;                                         // 2 gateways that we use to train the first 4 stalkers
    Units army1;                                            // all stalkers

    Point2D firstPylonLocation;                             // location for first pylon
    Point2D LocationForLastGateway;                         // location for last gateway
    Point2D proxypylonLocation;                             // location for proxy pylon

public:
    Economy(BasicSc2Bot &Bot);

    void OnStep();

    // build order for our bot
    void BuildOrder();

    // get a unit
    const Unit *getUnit(UNIT_TYPEID type);

    // find a placeable build location
    Point2D determineBuildLocation(ABILITY_ID ID, int location_ID = 1);

    // closest placeable point from near on a PlacementQuery
    Point2D closest(std::vector<sc2::QueryInterface::PlacementQuery> &qvect, std::vector<bool> &qvectCheck, Point2D &near);

    // farthest placeable point from near on a PlacementQuery 
    Point2D farthest(std::vector<sc2::QueryInterface::PlacementQuery>& qvect, std::vector<bool>& qvectCheck, Point2D& near);

    // builds a pylon depending on the location_ID default is 1 which builds the pylon close to the first pylon
    bool BuildPylon(int location_ID = 1);

    // build gateway using a worker specified and depending on location_ID, default is 1 which builds the gateway close to the first pylon
    bool BuildGateway(int location_ID = 1, int worker = 1);

    // build cybernetic core depending on location_ID, default is 1 which builds the gateway close to the first pylon
    bool BuildCyberneticsCore(int location_ID = 1);

    // get all the geysers on the map, and finds the closest 2 from near and puts those into an vector
    bool getNeutralGeyser(const Point3D &near);

    // assign 4 workers to gather vespene
    bool AssignVespene();

    // trains probe
    bool TrainProbe(const Unit *trainer_unit);

    // trains stalkers
    void TrainStalker();

    // checks if we have enough supply and minerals
    bool conditionB01(int food_used, int required_minerals = 0);

    // applies chronoBoost
    bool chronoBoost(const sc2::Unit *target);

    // builds assimilator, where is an index position for closestgeysers
    bool TryBuildAssimilator(int where, int worker);

    // scouts
    void TryScouting();

    // finds closest mineralpatch
    const Unit* FindNearestMineralPatch(const sc2::Point2D& start);

    // assigns probes to 2 vectors, MineralPatchProbes or VespeneGasProbes
    bool ProbesToGroups();

    // sets the first pylon location
    void setFirstPylonLocation();

    // set buildLastGateway to true
    void setbuildLastGatewayToTrue();

    // set buildLastGateway to false
    void setbuildLastGatewayToFalse();

    // add unit to gateway vector
    void addGateway(const sc2::Unit* unit);

    // add unit to army vector
    void addArmy1(const sc2::Unit* unit);

    // get army vector
    std::vector<const sc2::Unit*> getArmy1();

    // get gateway vector
    std::vector<const sc2::Unit*> getGateways();

    // get proxy pylon location
    Point2D getproxypylonLocation();

    // set chronoBoostCore to true
    void setchronoBoostCoreTrue();

    // set chronoBoostCore to false
    void setchronoBoostCoreFalse();

    // get setEnemyBase bool
    bool getsetEnemyBase();

    // set setEnemyBase to false
    void setEnemyBaseToFalse();
};