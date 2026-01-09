include engine/util;
include world.map;
include world.city;
include world.civ;

class WorldGenerator {
    idg = 0;

    fn WorldGenerator(){
        idg = util.IdGen();
    }

    fn generate(gs){
        w = 28;
        h = 18;
        m = world.map.Map(w, h);
        gs.world_map = m;

        c1 = world.civ.Civ(1, "North Empire", "blue");
        c2 = world.civ.Civ(2, "Southern League", "red");
        c3 = world.civ.Civ(3, "Highland Pact", "green");

        spawn_city(m, c1, 5, 5, "Noria");
        spawn_city(m, c1, 7, 7, "Valen");
        spawn_city(m, c2, 20, 10, "Sola");
        spawn_city(m, c2, 22, 13, "Maros");
        spawn_city(m, c3, 12, 14, "Westra");

        gs.civs.push_back(c1);
        gs.civs.push_back(c2);
        gs.civs.push_back(c3);
    }

    fn spawn_city(map, civ, x, y, name){
        if(!map.in_bounds(x, y)) return;
        t = map.get(x, y);
        if(t.terrain == "ocean") return;
        cid = idg.next();
        city = world.city.City(cid, name, civ.id, x, y);
        civ.add_city(city);
        t.city_id = cid;
        t.owner_id = civ.id;
    }
}
