class City {
    id = 0;
    name = "";
    owner_id = 0;
    x = 0;
    y = 0;
    population = 0;
    food_stock = 0;
    production_stock = 0;
    science_stock = 0;
    stability = 0;

    fn City(_id, _name, oid, px, py){
        id = _id;
        name = _name;
        owner_id = oid;
        x = px;
        y = py;
        population = 3;
        food_stock = 10;
        production_stock = 0;
        science_stock = 0;
        stability = 50;
    }

    fn food_yield(map){
        t = map.get(x, y);
        if(t == null) return 0;
        base = 2;
        if(t.terrain == "plains") base += 2;
        if(t.terrain == "forest") base += 1;
        if(t.river) base += 1;
        return base + population / 2;
    }

    fn prod_yield(map){
        t = map.get(x, y);
        if(t == null) return 0;
        base = 1;
        if(t.terrain == "hill") base += 2;
        if(t.terrain == "forest") base += 1;
        return base + population / 3;
    }

    fn sci_yield(){
        return 1 + population / 3;
    }

    fn apply_turn(map){
        food_gain = food_yield(map);
        prod_gain = prod_yield(map);
        sci_gain = sci_yield();

        food_stock += food_gain;
        production_stock += prod_gain;
        science_stock += sci_gain;

        need_food = population * 2;
        if(food_stock >= need_food){
            food_stock -= need_food;
            if(food_stock > 8){
                population += 1;
                food_stock -= 4;
            }
        } else {
            deficit = need_food - food_stock;
            food_stock = 0;
            if(deficit > 0){
                population -= 1;
                if(population < 1) population = 1;
                stability -= 4;
            }
        }

        if(stability > 80){
            production_stock += 1;
        }
        if(stability < 30){
            production_stock -= 1;
            if(production_stock < 0) production_stock = 0;
        }

        if(stability > 100) stability = 100;
        if(stability < 0) stability = 0;
    }

    fn summary(){
        println("City #{id} {name}: pop={population} food={food_stock} prod={production_stock} sci={science_stock} stab={stability}");
    }
}
