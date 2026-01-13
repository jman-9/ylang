class GrandPlanner {
    fn GrandPlanner(){}

    fn expansion_value(civ, map){
        score = 0;
        for(i=0;i<civ.cities.size();i+=1){
            c = civ.cities[i];
            t = map.get(c.x, c.y);
            if(t != null){
                if(t.terrain == "plains") score += 3;
                if(t.terrain == "forest") score += 2;
                if(t.terrain == "hill") score += 1;
                if(t.river) score += 2;
            }
            score += c.population;
            score += c.stability / 2;
        }
        score += civ.treasury / 2;
        score += civ.science / 4;
        return score;
    }

    fn internal_stability(civ){
        val = civ.stability;
        if(civ.treasury < 10) val -= 5;
        if(civ.treasury > 80) val += 3;
        if(civ.military_power < 5) val -= 2;
        if(civ.military_power > 30) val += 2;
        return val;
    }

    fn choose_focus(civ, map){
        exp = expansion_value(civ, map);
        stab = internal_stability(civ);
        if(stab < 35) return "stability";
        if(civ.military_power < civ.cities.size() * 3) return "military";
        if(exp > 40) return "economy";
        if(civ.science < 40) return "science";
        return "balanced";
    }
}
