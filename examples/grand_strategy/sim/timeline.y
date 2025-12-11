include sim.economy;
include sim.diplomacy;
include sim.war;
include ai.strategy;

class Timeline {
    gs = 0;
    econ = 0;
    war = 0;
    strat = 0;

    fn Timeline(_gs){
        gs = _gs;
        econ = sim.economy.Economy();
        war = sim.war.WarSystem();
        strat = ai.strategy.StrategyAI();
    }

    fn step_year(){
        gs.year += 1;

        econ.apply_year(gs);

        for(i=0;i<gs.civs.len();i+=1){
            civ = gs.civs[i];
            focus = strat.decide_focus(civ, gs.world_map);
            strat.apply_focus(civ, focus);
        }

        strat.adjust_diplomacy(gs);

        war.resolve_year(gs);

        gs.print_yearly_report();
    }
}
