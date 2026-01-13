include world.city;

class Civ {
    id = 0;
    name = "";
    color = "";
    cities = 0;
    treasury = 0;
    science = 0;
    stability = 0;
    military_power = 0;

    fn Civ(_id, _name, _color){
        id = _id;
        name = _name;
        color = _color;
        cities = [];
        treasury = 40;
        science = 0;
        stability = 50;
        military_power = 10;
    }

    fn add_city(city){
        cities.push_back(city);
    }

    fn apply_turn(map){
        total_pop = 0;
        total_prod = 0;
        for(i=0;i<cities.size();i+=1){
            city = cities[i];
            city.apply_turn(map);
            total_pop += city.population;
            total_prod += city.production_stock;
        }

        treasury += total_prod / 2;
        science_gain = total_pop / 2;
        science += science_gain;

        if(total_pop >= 5){
            stability += 1;
        } else {
            stability -= 1;
        }

        if(stability > 80){
            military_power += 1;
        }
        if(stability < 30){
            military_power -= 1;
        }

        if(military_power < 0) military_power = 0;
        if(stability > 100) stability = 100;
        if(stability < 0) stability = 0;
    }

    fn main_city(){
        if(cities.size() == 0) return null;
        best = cities[0];
        best_score = best.population + best.stability / 2;
        for(i=1;i<cities.size();i+=1){
            c = cities[i];
            score = c.population + c.stability / 2;
            if(score > best_score){
                best_score = score;
                best = c;
            }
        }
        return best;
    }

    fn summary(){
        println("Civ #{id} {name}: cities={cities.size()} treasury={treasury} sci={science} stab={stability} mil={military_power}");
    }
}
