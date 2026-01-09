include world.civ;

class Economy {
    fn Economy(){}

    fn apply_year(gs){
        for(i=0;i<gs.civs.size();i+=1){
            civ = gs.civs[i];
            civ.apply_turn(gs.world_map);
        }
    }
}
