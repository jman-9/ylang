include sim.diplomacy;

class GameState {
    year = 0;
    world_map = 0;
    civs = 0;
    diplomacy = 0;

    fn GameState(){
        year = 0;
        world_map = null;
        civs = [];
        diplomacy = sim.diplomacy.Diplomacy();
    }

    fn get_civ(id){
        for(i=0;i<civs.size();i+=1){
            if(civs[i].id == id) return civs[i];
        }
        return null;
    }

    fn print_yearly_report(){
        println("");
        println("=== YEARLY REPORT {year} ===");
        println("Civs: {civs.size()}");
        for(i=0;i<civs.size();i+=1){
            civs[i].summary();
            for(j=0;j<civs[i].cities.size();j+=1){
                civs[i].cities[j].summary();
            }
        }
        diplomacy.summary(civs);
        println("============================");
    }
}
