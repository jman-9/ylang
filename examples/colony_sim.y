include sys;
include rand;
include math;

class Item {
    _name = "";
    _kind = "";
    _power = 0;
    _value = 0;

    fn Item(name, kind, power, value) {
        _name = name;
        _kind = kind;
        _power = power;
        _value = value;
    }

    fn name() {
        return _name;
    }

    fn info() {
        return "{_name}({_kind}, pow={_power}, val={_value})";
    }
}


class ResourceStack {
    _name = "";
    _amount = 0;

    fn ResourceStack(name, amount) {
        _name = name;
        _amount = amount;
    }

    fn name() { return _name; }
    fn amount() { return _amount; }

    fn add(v) { _amount += v; }
    fn remove(v) {
        if (_amount < v) return 0;
        _amount -= v;
        return 1;
    }

    fn info() { return "{_name}:{_amount}"; }
}

class Inventory {
    _items = {};

    fn Inventory() { }

    fn add(name, amt) {
        if (_items.contains(name))
            _items[name] = _items[name] + amt;
        else
            _items[name] = amt;
    }

    fn remove(name, amt) {
        if (!_items.contains(name)) return 0;
        if (_items[name] < amt) return 0;
        _items[name] = _items[name] - amt;
        return 1;
    }

    fn get(name) {
        if (!_items.contains(name)) return 0;
        return _items[name];
    }

    fn keys() { return _items.keys(); }

    fn describe() {
        ks = _items.keys();
        if (ks.len() == 0) return "{{}}";
        out = "{{";
        for (i = 0; i < ks.len(); i++) {
            k = ks[i];
            out += "{k}:{_items[k]}";
            if (i + 1 < ks.len()) out += ", ";
        }
        out += "}}";
        return out;
    }
}

class Colonist {
    _name = "";
    _health = 100;
    _energy = 100;
    _morale = 0;
    _role = "";
    _inventory = 0;

    fn Colonist(name, role) {
        _name = name;
        _role = role;
        _health = 100;
        _energy = 100;
        _morale = 0;
        _inventory = Inventory();
    }

    fn isAlive() { return _health > 0; }

    fn workEff() {
        eff = 1.0;
        if (_energy < 50) eff *= 0.7;
        if (_morale < 0) eff *= 0.8;
        return eff;
    }

    fn rest() {
        _energy += 20;
        if (_energy > 100) _energy = 100;
    }

    fn consume(inv, name, amt) {
        if (inv.remove(name, amt)) return 1;
        return 0;
    }

    fn dailyUpdate() {
        _energy -= 20;
        if (_energy < 0) _energy = 0;
        if (_energy == 0) _health -= 5;

        if (_health <= 0) {
            _health = 0;
        }
    }

    fn moraleUp(v) {
        _morale += v;
        if (_morale > 5) _morale = 5;
    }
    fn moraleDown(v) {
        _morale -= v;
        if (_morale < -5) _morale = -5;
    }

    fn info() {
        return "{_name}({_role}) HP={_health}, EN={_energy}, MO={_morale}";
    }
}

class Building {
    _name = "";
    _type = "";
    _inv = 0;

    fn Building(name, type) {
        _name = name;
        _type = type;
        _inv = Inventory();
    }

    fn name() { return _name; }

    fn inv() { return _inv; }

    fn describe() {
        return "{_name}({_type}) inv={_inv.describe()}";
    }
}

class Habitat {
    _colonists = [];
    _facilities = [];

    fn Habitat() { }

    fn addColonist(c) { _colonists.append(c); }
    fn addFacility(b) { _facilities.append(b); }

    fn allColonists() { return _colonists; }
    fn allFacilities() { return _facilities; }

    fn aliveColonist() {
        alive = [];
        for (i = 0; i < _colonists.len(); i++) {
            if (_colonists[i].isAlive())
                alive.append(_colonists[i]);
        }
        return alive;
    }

    fn describe() {
        println("== Habitat ==");
        for (i = 0; i < _colonists.len(); i++) {
            println("  " + _colonists[i].info());
        }
        for (i = 0; i < _facilities.len(); i++) {
            println("  " + _facilities[i].describe());
        }
    }
}

class Farm {
    _rate = 0;

    fn Farm(rate) {
        _rate = rate;
    }

    fn produce(colEff) {
        amt = math.floor(_rate * colEff);
        if (amt < 1) amt = 1;
        return amt;
    }
}

class Mine {
    _rate = 0;

    fn Mine(rate) {
        _rate = rate;
    }

    fn produce(colEff) {
        amt = math.floor(_rate * colEff);
        if (amt < 1) amt = 1;
        return amt;
    }
}

class PowerPlant {
    _output = 0;
    fn PowerPlant(out) { _output = out; }

    fn generate() {
        return _output + rand.get(-2, 2);
    }
}

class Rover {
    _name = "";
    _durability = 100;

    fn Rover(name) {
        _name = name;
        _durability = 100;
    }

    fn explore() {
        _durability -= rand.get(1, 6);
        if (_durability < 0) _durability = 0;

        r = rand.get(0.0, 1.0);
        if (r < 0.4)
            return Item("Ore Chunk", "resource", 0, 5);
        if (r < 0.6)
            return Item("Alien Relic", "rare", 0, 30);
        return null;
    }

    fn isBroken() { return _durability <= 0; }

    fn info() {
        return "{_name} DUR={_durability}";
    }
}

class Colony {
    _hab = 0;
    _stock = 0;
    _farm = 0;
    _mine = 0;
    _pp = 0;
    _rovers = [];

    fn Colony(hab, farm, mine, pp) {
        _hab = hab;
        _farm = farm;
        _mine = mine;
        _pp = pp;
        _stock = Inventory();
        _rovers = [];
    }

    fn addRover(r) {
        _rovers.append(r);
    }

    fn dailyProduction() {
        cols = _hab.aliveColonist();
        effSum = 0.0;
        for (i = 0; i < cols.len(); i++) {
            effSum += cols[i].workEff();
        }
        if (effSum == 0) return;

        food = _farm.produce(effSum);
        ore = _mine.produce(effSum);
        powerOut = _pp.generate();

        _stock.add("food", food);
        _stock.add("ore", ore);
        _stock.add("power", powerOut);

        println("  Food+{food}, Ore+{ore}, Power+{powerOut}");
    }

    fn dailyConsumption() {
        cols = _hab.aliveColonist();
        totalFood = cols.len();
        if (!_stock.remove("food", totalFood)) {
            for (i = 0; i < cols.len(); i++) {
                cols[i].moraleDown(1);
            }
        } else {
            for (i = 0; i < cols.len(); i++) {
                cols[i].moraleUp(1);
            }
        }
    }

    fn roverExpeditions() {
        for (i = 0; i < _rovers.len(); i++) {
            r = _rovers[i];
            if (r.isBroken()) continue;

            loot = r.explore();
            if (loot != null) {
                _stock.add(loot.name(), 1);
                println("  Rover {r.info()} found {loot.info()}");
            }
        }
    }

    fn dailyColonistUpdate() {
        cols = _hab.allColonists();
        for (i = 0; i < cols.len(); i++) {
            cols[i].dailyUpdate();
        }
    }

    fn describe() {
        println("== Colony Stock ==");
        println(_stock.describe());
    }

    fn state() {
        _hab.describe();
        describe();
    }
}

class EventSystem {
    _worldRand = 0;

    fn EventSystem() { _worldRand = 0; }

    fn dailyEvent(col) {
        r = rand.get(0.0, 1.0);

        if (r < 0.1) {
            println("!! Sandstorm damages rovers");
            for (i = 0; i < col._rovers.len(); i++) {
                col._rovers[i]._durability -= 10;
            }
        } else if (r < 0.15) {
            println("!! Minor quake affects morale");
            cs = col._hab.allColonists();
            for (i = 0; i < cs.len(); i++) {
                cs[i].moraleDown(1);
            }
        } else if (r < 0.18) {
            println("!! Food storage spoiled. -5 food");
            col._stock.remove("food", 5);
        }
    }
}

fn makeColony() {
    h = Habitat();
    h.addColonist(Colonist("Ava", "Botanist"));
    h.addColonist(Colonist("Leo", "Engineer"));
    h.addColonist(Colonist("Nia", "Miner"));

    h.addFacility(Building("Dormitory", "living"));
    h.addFacility(Building("SolarFarm", "power"));
    h.addFacility(Building("AgriDome", "farm"));

    farm = Farm(8);
    mine = Mine(5);
    pp = PowerPlant(12);

    c = Colony(h, farm, mine, pp);
    c.addRover(Rover("R1"));
    c.addRover(Rover("R2"));
    c.addRover(Rover("R3"));

    return c;
}

fn main() {
    rand.randomize_timer();

    col = makeColony();
    ev = EventSystem();

    for (day = 1; day <= 10; day++) {
        println("=== Day {day} ===");

        col.dailyProduction();
        col.dailyConsumption();
        col.roverExpeditions();
        col.dailyColonistUpdate();
        ev.dailyEvent(col);

        col.state();
        println("");
    }

    println("Simulation End.");
}
