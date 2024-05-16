#include "Economy.h"
#include "BasicSc2Bot.h"

Economy::Economy(BasicSc2Bot &Bot) : Bot(Bot) {}


void Economy::BuildOrder()
{

    if (!doneB00)
    {
        
        doneB00 = ProbesToGroups() && getNeutralGeyser(getUnit(UNIT_TYPEID::PROTOSS_NEXUS)->pos);   // assign probes to MineralPatchProbes or VespeneGasProbes, and get the closet geyser from our nexus

        //    std::cout << "Step 0" << std::endl;
    }
    if (!doneB01 && conditionB01(14, 100))
    {
       
        doneB01 = BuildPylon(0);                                

        //    std::cout << "Step 1" << std::endl;
    }

    if (!doneB02 && conditionB01(16, 150))
    {

        doneB02 = BuildGateway(1, 1);                         

        // start scouting
        if (!Scouted) {
            TryScouting();
        }

        //    std::cout << "Step 2" << std::endl;

    }

    if (!doneB03 && conditionB01(17, 75))
    {
        
        doneB03 = TryBuildAssimilator(0, 0);

        //    std::cout << "Step 3" << std::endl;

    }

    if (!doneB04 && conditionB01(18, 75))
    {

        doneB04 = TryBuildAssimilator(1, 1);

        //    std::cout << "Step 4" << std::endl;

    }

    if (!doneB05 && conditionB01(20, 150))
    {

        doneB05 = BuildGateway(1, 1);

        //    std::cout << "Step 5" << std::endl;

    }

    if (!doneB06 && conditionB01(21, 150))
    {

        doneB06 = BuildCyberneticsCore();

        //    std::cout << "Step 6" << std::endl;

    }

    if (!doneB07 && conditionB01(22, 100))
    {

        doneB07 = BuildPylon(1);

        //    std::cout << "Step 7" << std::endl;

    }

    if (!doneB08 && conditionB01(23, 100)) 
    {

        doneB08 = AssignVespene();

        //    std::cout << "Step 8" << std::endl;

    }

    // chronoBoost at roughly every 5 seconds
    if (chronoBoostCore && (Bot.Observation()->GetGameLoop() % 500 == 0)) 
    {

        const auto& cyberCore = getUnit(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
        chronoBoost(cyberCore);

    }

    if (!doneB09 && conditionB01(27, 100))
    {

        // if we have the enemy base location then build our proxy pylon
        if (Bot.enemy_base.x != 0.0) {

            doneB09 = BuildPylon(2);

            //        std::cout << "Step 9" << std::endl;

        }

    }

    if (!doneB10 && conditionB01(27, 250))
    {

        doneB10 = BuildGateway(1, 5);

        //    std::cout << "Step 10" << std::endl;

    }

    // builds LastGateway
    if (buildLastGateway && conditionB01(27, 150)) {


        if (setOnce) {

            getLocationForLastGateway = true;

        }

        sc2::Point2D buildLocation = determineBuildLocation(sc2::ABILITY_ID::BUILD_GATEWAY, 1);         // get the LocationForLastGateway

        setOnce = false;

        const Unit* probe = MineralPatchProbes[2];
        if (probe)
        {

            Bot.Actions()->UnitCommand(probe, sc2::ABILITY_ID::BUILD_GATEWAY, LocationForLastGateway);

        }

    }

}


bool Economy::BuildPylon(int location_ID)
{

    sc2::Point2D buildLocation = determineBuildLocation(sc2::ABILITY_ID::BUILD_PYLON, location_ID);         // get buildlocation for pylon
    if (location_ID == 2) {
        proxypylonLocation = buildLocation;          // save the location of the proxy pylon
    }
    const Unit *probe = MineralPatchProbes[0];
    if (probe)
    {
        Bot.Actions()->UnitCommand(probe, sc2::ABILITY_ID::BUILD_PYLON, buildLocation);         // build pylon
        return true;
    }

    return false;
}


bool Economy::BuildGateway(int location_ID, int worker)
{
    sc2::Point2D buildLocation = determineBuildLocation(sc2::ABILITY_ID::BUILD_GATEWAY, location_ID);       // get buildlocation for gateway
    const Unit *probe = MineralPatchProbes[worker];
    if (probe)
    {
        Bot.Actions()->UnitCommand(probe, sc2::ABILITY_ID::BUILD_GATEWAY, buildLocation);       // build gateway
        return true;
    }
    return false;
}


bool Economy::getNeutralGeyser(const sc2::Point3D &near)
{
    sc2::Units units = Bot.Observation()->GetUnits(sc2::Unit::Alliance::Neutral);

    Units allGeysers;                                                       // all geysers
    std::vector<float> GeysersDistance;                                     // distance from near to geyser

    // get all geysers and their distance from near to geyser

    for (const auto &unit : units)
    {
        if ((unit->unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER) ||
            (unit->unit_type == UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER) ||
            (unit->unit_type == UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER) ||
            (unit->unit_type == UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER) ||
            (unit->unit_type == UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER) ||
            (unit->unit_type == UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER))
        {

            allGeysers.push_back(unit);
            float d = DistanceSquared3D(unit->pos, near);
            GeysersDistance.push_back(d);
        }
    }

    // get closest geyser index

    float smallest = std::numeric_limits<float>::max();
    size_t smallestIndex = 0;

    for (size_t i = 0; i < GeysersDistance.size(); ++i)
    {
        if (GeysersDistance[i] < smallest)
        {
            smallest = GeysersDistance[i];
            smallestIndex = i;
        }
    }

    // get second closest geyser index

    float secondSmallest = std::numeric_limits<float>::max();
    size_t secondSmallestIndex = 0;
    for (size_t i = 0; i < GeysersDistance.size(); ++i)
    {
        if (GeysersDistance[i] < secondSmallest && i != smallestIndex)
        {
            secondSmallest = GeysersDistance[i];
            secondSmallestIndex = i;
        }
    }

    // add closest geyser and second closest geyser to ClosestGeyser vector

    ClosestGeysers.push_back(allGeysers[smallestIndex]);
    ClosestGeysers.push_back(allGeysers[secondSmallestIndex]); 

    return true;
}


bool Economy::BuildCyberneticsCore(int location_ID)
{

    sc2::Point2D buildLocation = determineBuildLocation(sc2::ABILITY_ID::BUILD_CYBERNETICSCORE, location_ID);           // get buildlocation for Cybernetic core
    const Unit *probe = MineralPatchProbes[2];
    if (probe)
    {
        Bot.Actions()->UnitCommand(probe, sc2::ABILITY_ID::BUILD_CYBERNETICSCORE, buildLocation);
        return true;
    }
    return false;
}


Point2D Economy::closest(std::vector<sc2::QueryInterface::PlacementQuery> &qvect, std::vector<bool> &qvectCheck, Point2D &near)
{
    size_t N = qvect.size();
    Point2D closestLocation;

    if (!N || N != qvectCheck.size())
        return Point2D(0, 0); // invalid since empty or vects dont match

    // get closestLocation from near in PlacementQuery

    float minDist = Distance2D(qvect[0].target_pos, near);
    for (int i = 0; i < N; i++)
    {
        if (qvectCheck[i])
        {
            float dist = Distance2D(qvect[i].target_pos, near);
            if (dist <= minDist)
            {
                closestLocation = qvect[i].target_pos;
                minDist = dist;
            }
        }
    }

    if (getLocationForLastGateway == true) {                // if we are getting the location for the last gateway we only want to set it once, because for some reason
        LocationForLastGateway = closestLocation;           // when we keep calling this function repetitively, it'll give different values
        getLocationForLastGateway = false;                  // so this way we only save the first value and don't care about the other different values
    }

    return closestLocation;
}


Point2D Economy::farthest(std::vector<sc2::QueryInterface::PlacementQuery>& qvect, std::vector<bool>& qvectCheck, Point2D& near)
{
    size_t N = qvect.size();
    Point2D farthestLocation;

    if (!N || N != qvectCheck.size())
        return Point2D(0, 0); // invalid since empty or vects dont match

    // get farthestLocation from near in PlacementQuery

    float maxDist = Distance2D(qvect[0].target_pos, near);
    for (int i = 0; i < N; i++)
    {
        if (qvectCheck[i])
        {
            float dist = Distance2D(qvect[i].target_pos, near);
            if (dist >= maxDist)
            {
                farthestLocation = qvect[i].target_pos;
                maxDist = dist;
            }
        }
    }

    return farthestLocation;
}


sc2::Point2D Economy::determineBuildLocation(sc2::ABILITY_ID ID, int location_ID)
{

    sc2::Point2D near, location;

    switch (location_ID)
    {
        case (0):
        { // near our nexus
            near = getUnit(sc2::UNIT_TYPEID::PROTOSS_NEXUS)->pos;
            break;
        }
        case (1):
        { // near the position of the first pylon we placed
            near = firstPylonLocation;
            break;
        }
        case (2):
        { // near enemy
            if (map_name == "Cactus Valley LE (Void)") {
                if (Bot.enemy_base.y <= 50){
                    near.x = 90.0;
                    near.y = 33.5;
                }
                else{
                    near.x = 90.0;
                    near.y = 158.5;
                }

            }
            else if (map_name == "Bel'Shir Vestige LE (Void)") {
                if (Bot.enemy_base.x < 60.0){
                        near.x = 100.0;
                        near.y = 120.0;
                }
                else{
                    near.x = 50.0;
                    near.y = 20.0;
                }
            }
            else {
                if (Bot.enemy_base.x < 85.0){
                    near.x = 35.0;
                    near.y = 85.0;
                
                }
                else{
                    near.x = 150.0;
                    near.y = 70.0;
                }
            }
            break;
        }
    }

   

    float maxDist = 5.0f;       // location we build around near
    float minX;
    float minY;
    float maxX;
    float maxY;

    // minX, minY, maxX, maxY is the box we build at

    if (location_ID == 0) {

        // left side of the map
        if (near.x < Bot.getmapCenter().x) {
            minX = near.x;
            maxX = minX + maxDist;
        }
        // right side of the map
        else {
            minX = near.x - maxDist;
            maxX = minX + maxDist;
        }

        // bottom side of the map
        if (near.y < Bot.getmapCenter().y) {
            minY = near.y;
            maxY = minY + maxDist;
        }
        // top side of the map
        else {
            minY = near.y - maxDist;
            maxY = minY + maxDist;
        }

    }

    else {
        minX = near.x - maxDist;
        maxX = near.x + maxDist;
        minY = near.y - maxDist;
        maxY = near.y + maxDist;

    }

    // find placeable locations given minX, minY, maxX, maxY

    std::vector<sc2::QueryInterface::PlacementQuery> queries;
    for (float y = minY; y <= maxY; y += 1.0f)
    {
        for (float x = minX; x <= maxX; x += 1.0f)
        {
            if (x > 0 && x < 2 * Bot.getmapCenter().x && y > 0 && y < 2 * Bot.getmapCenter().y) {
            
                sc2::Point2D loc = sc2::Point2D(x, y);
                queries.push_back(sc2::QueryInterface::PlacementQuery(ID, loc));
            }
         
        }
    }

    std::vector<bool> results = Bot.Query()->Placement(queries);

    // farthest position from the vector of placeable location
    if (location_ID == 0) {
        location = farthest(queries, results, near);
    }
    // closest position from the vector of placeable locations
    else {
        location = closest(queries, results, near);
    }

    return location;

}


const sc2::Unit *Economy::getUnit(sc2::UNIT_TYPEID type)
{
    // Iterate through all my units, find unit
    const sc2::Unit *wantedUnit = nullptr;

    const std::vector<const sc2::Unit *> &all = Bot.Observation()->GetUnits(sc2::Unit::Alliance::Self);

    for (auto unit : all)
    {
       
        if (unit->unit_type == type)
        {
            wantedUnit = unit;
            break;
        }
    }
    return wantedUnit;
}


bool Economy::TrainProbe(const sc2::Unit *nexus)
{
    if (nexus)
    {
        if (Bot.Observation()->GetMinerals() >= 50)
        {
            Bot.Actions()->UnitCommand(nexus, sc2::ABILITY_ID::TRAIN_PROBE);
            return true;
        }
    }
    return false;
}


void Economy::TrainStalker()
{

    // Train Stalkers from multiple Gateways/Warpgates
    size_t N = Gateways.size();

    for (size_t i = 0; i < N; ++i)
    {
        Bot.Actions()->UnitCommand(Gateways[i], ABILITY_ID::TRAIN_STALKER);
    }

}


bool Economy::conditionB01(int food_used, int required_minerals)
{

    return Bot.Observation()->GetFoodUsed() >= food_used && Bot.Observation()->GetMinerals() >= required_minerals;
}


bool Economy::AssignVespene()
{

    // allocate more Probes to gas mining 

    auto vespene_collecting_unit3 = VespeneGasProbes[2];
    auto vespene_collecting_unit4 = VespeneGasProbes[3];
    auto vespene_collecting_unit5 = VespeneGasProbes[4];
    auto vespene_collecting_unit6 = VespeneGasProbes[5];

    Units assimilators;     // all assimilators

    // get all assimilators
    sc2::Units units = Bot.Observation()->GetUnits(sc2::Unit::Alliance::Self);
    for (const auto &u : units)
    {
        if (u->unit_type == sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR)
        {
            assimilators.push_back(u);
        }
    }

    // allocate the probes to mine at the assimilators

    Bot.Actions()->UnitCommand(vespene_collecting_unit3, sc2::ABILITY_ID::SMART, assimilators[0]);
    Bot.Actions()->UnitCommand(vespene_collecting_unit4, sc2::ABILITY_ID::SMART, assimilators[0]);
    Bot.Actions()->UnitCommand(vespene_collecting_unit5, sc2::ABILITY_ID::SMART, assimilators[1]);
    Bot.Actions()->UnitCommand(vespene_collecting_unit6, sc2::ABILITY_ID::SMART, assimilators[1]);

    return true;

}


bool Economy::chronoBoost(const sc2::Unit *target)
{
    // get a nexus to chronoBoost a target
    const Unit *nexus = getUnit(UNIT_TYPEID::PROTOSS_NEXUS);

    if (nexus && target && nexus->energy >= 50)
    {
        Bot.Actions()->UnitCommand(nexus, AbilityID(3755), target);
        return true;
    }
    else
    {
        return false;
    }

}


void Economy::OnStep()
{

    BuildOrder();

}


bool Economy::TryBuildAssimilator(int where, int worker)
{

    // Get a probe to build the structure.
    const sc2::Unit *unit_to_build = VespeneGasProbes[worker];
    const sc2::Unit *target = ClosestGeysers[where];

    Bot.Actions()->UnitCommand(unit_to_build, sc2::ABILITY_ID::BUILD_ASSIMILATOR, target);

    return true;

}


void Economy::TryScouting() {

    // if we are in Cactus Valley we need to scout for enemybase location, else if we are on the other maps we already know the enemybase location 
    Scouted = true;
    const ObservationInterface* observation = Bot.Observation();
    const GameInfo& game_info = Bot.Observation()->GetGameInfo();
    const sc2::Unit* wantedUnit = nullptr;
    wantedUnit = MineralPatchProbes[3];

    if (map_name == "") {
        map_name = game_info.map_name;
    }

    if ((map_name == "Cactus Valley LE (Void)") && (wantedUnit != nullptr)) {
        setEnemyBase = true;
        for (auto location : game_info.enemy_start_locations){
            Bot.Actions()->UnitCommand(wantedUnit, ABILITY_ID::MOVE_MOVE, location, true);      // scout by going to every single possible enemy start location
        }
    }
    else {
        Bot.enemy_base = game_info.enemy_start_locations.front();
    }

}


const sc2::Unit *Economy::FindNearestMineralPatch(const sc2::Point2D &start)
{

    sc2::Units units = Bot.Observation()->GetUnits(sc2::Unit::Alliance::Neutral);
    float distance = std::numeric_limits<float>::max();
    const sc2::Unit *target = nullptr;
    for (const auto &u : units)
    {
        if (u->unit_type == sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD)
        {
            float d = DistanceSquared2D(u->pos, start);
            if (d < distance)
            {
                distance = d;
                target = u;
            }
        }
    }
    return target;
}


bool Economy::ProbesToGroups()
{
    // Assign the 12 probes at the start of the game to 2 groups, MineralPatchProbes or VespeneGasProbes
    const sc2::ObservationInterface *observation = Bot.Observation();

    sc2::Units units = observation->GetUnits(sc2::Unit::Alliance::Self);
    int i = 0;
    for (auto unit : units)
    {
        if (unit->unit_type == UNIT_TYPEID::PROTOSS_PROBE)
        {

            if (i < 6)
            {
                MineralPatchProbes.push_back(unit);
            }
            else
            {
                VespeneGasProbes.push_back(unit);
            }

            ++i;
        }
    }

    return true;
}


void Economy::setFirstPylonLocation() {
    // set the first pylon location
    firstPylonLocation = determineBuildLocation(ABILITY_ID::BUILD_PYLON, 0);
}


void Economy::setbuildLastGatewayToTrue() {
    // set buildLastGateway to true
    buildLastGateway = true;
}


void Economy::setbuildLastGatewayToFalse() {
    // set buildLastGateway to false
    buildLastGateway = false;
}


void Economy::addGateway(const sc2::Unit* unit) {
    // add unit to gateway vector
    Gateways.push_back(unit);
}


void Economy::addArmy1(const sc2::Unit* unit) {
    // add unit to army vector
    army1.push_back(unit);
}


std::vector<const sc2::Unit*> Economy::getArmy1() {
    // get army vector
    return army1;
}


std::vector<const sc2::Unit*> Economy::getGateways() {
    // get gateway vector
    return Gateways;
}


Point2D Economy::getproxypylonLocation() {
    // get proxy pylon location
    return proxypylonLocation;
}


void Economy::setchronoBoostCoreTrue() {
    // set chronoBoostCore to true
    chronoBoostCore = true;
}


void Economy::setchronoBoostCoreFalse() {
    // set chronoBoostCore to false
    chronoBoostCore = false;
}


bool Economy::getsetEnemyBase() {
    // get setEnemyBase bool
    return setEnemyBase;
}


void Economy::setEnemyBaseToFalse() {
    // set EnemyBase to false
    setEnemyBase = false;
}


