include ai.planner;

class StrategyAI {
    planner = 0;

    fn StrategyAI(){
        planner = ai.planner.GrandPlanner();
    }

    fn decide_focus(civ, map){
        return planner.choose_focus(civ, map);
    }

    fn apply_focus(civ, focus){
        if(focus == "economy"){
            civ.treasury += 6;
            civ.stability -= 1;
        }
        if(focus == "science"){
            civ.science += 5;
            civ.treasury -= 2;
        }
        if(focus == "military"){
            civ.military_power += 4;
            civ.treasury -= 2;
        }
        if(focus == "stability"){
            civ.stability += 3;
            civ.treasury -= 1;
        }
        if(focus == "balanced"){
            civ.treasury += 2;
            civ.science += 2;
            civ.military_power += 2;
        }

        if(civ.treasury < 0) civ.treasury = 0;
        if(civ.military_power < 0) civ.military_power = 0;
        if(civ.stability > 100) civ.stability = 100;
        if(civ.stability < 0) civ.stability = 0;
    }

    fn pick_war_target(gs, civ){
        best_id = 0;
        best_score = 0;
        for(i=0;i<gs.civs.size();i+=1){
            other = gs.civs[i];
            if(other.id == civ.id) continue;
            rel = gs.diplomacy.get(civ.id, other.id);
            if(rel > 10) continue;

            score = other.cities.size() * 5 + other.military_power;
            if(score > best_score && civ.military_power > other.military_power + 5){
                best_score = score;
                best_id = other.id;
            }
        }
        return best_id;
    }

    fn adjust_diplomacy(gs){
        for(i=0;i<gs.civs.size();i+=1){
            for(j=i+1;j<gs.civs.size();j+=1){
                a = gs.civs[i];
                b = gs.civs[j];
				
                rel = gs.diplomacy.get(a.id, b.id);
                /*if(a.stability > 50 && b.stability > 50){
                    gs.diplomacy.add(a.id, b.id, 1);
                }
                if(a.stability < 30 || b.stability < 30){
                    gs.diplomacy.add(a.id, b.id, -1);
                }*/
				
				avg_stab = (a.stability + b.stability) / 2;

				if(avg_stab < 32){
					gs.diplomacy.add(a.id, b.id, -1);
				}

				if(avg_stab > 36){
					gs.diplomacy.add(a.id, b.id, +1);
				}					
            }
        }
    }
}
