include engine.pathfinding;
include ai.strategy;

class WarSystem {
    pathfinder = 0;
    strat = 0;

    fn WarSystem(){
        pathfinder = engine.pathfinding.PathFinding();
        strat = ai.strategy.StrategyAI();
    }

    fn resolve_year(gs){
        map = gs.world_map;
        for(i=0;i<gs.civs.len();i+=1){
            attacker = gs.civs[i];
            target_id = strat.pick_war_target(gs, attacker);
            if(target_id == 0) continue;
            defender = gs.get_civ(target_id);
            if(defender == null) continue;

            println("War: {attacker.name} considers attacking {defender.name}");

            a_city = attacker.main_city();
            d_city = defender.main_city();
            if(a_city == null || d_city == null) continue;

            path = pathfinder.find_path(map, a_city.x, a_city.y, d_city.x, d_city.y);
            if(path.len() == 0){
                println("  No viable path for campaign.");
                continue;
            }

            println("  Campaign path length: {path.len()}");
            resolve_campaign(attacker, defender, path);
        }
    }

    fn resolve_campaign(attacker, defender, path){
        strength_a = attacker.military_power;
        strength_d = defender.military_power;
        if(strength_a <= 0 || strength_d <= 0) return;

        losses = path.len() / 3;
        strength_a -= losses;
        if(strength_a < 0) strength_a = 0;

        if(strength_a > strength_d){
            println("  {attacker.name} wins the war.");
            attacker.military_power = strength_a - strength_d / 2;
            defender.military_power = 0;
            if(defender.cities.len() > 0){
                taken = defender.cities[0];
                defender.cities.remove(0);
                taken.owner_id = attacker.id;
                attacker.cities.append(taken);
                println("  City {taken.name} captured by {attacker.name}");
            }
        } else {
            println("  {defender.name} defends successfully.");
            defender.military_power = strength_d - strength_a / 2;
            attacker.military_power = 0;
        }
    }
}
