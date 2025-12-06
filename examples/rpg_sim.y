include sys;
include math;
include rand;

// =========================
// Basic utility
// =========================

fn clamp(v, min, max) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

fn random_choice(list) {
    if (list.len() == 0) return null;
    idx = rand.get(0, list.len() - 1);
    return list[idx];
}


class Item {
    _name = "";
    _kind = "";
    _power = 0;
    _value = 0;

    fn Item(name_, kind_, power_, value_) {
        _name = name_;
        _kind = kind_;
        _power = power_;
        _value = value_;
    }

    fn name() {
        return _name;
    }

    fn kind() {
        return _kind;
    }

    fn power() {
        return _power;
    }

    fn value() {
        return _value;
    }

    fn info() {
        return "{_name}({_kind}, pow={_power}, val={_value})";
    }
}

class Inventory {
    _items = [];

    fn Inventory() { }

    fn add(item) {
        _items.append(item);
    }

    fn removeByIndex(index) {
        if (index < 0 || index >= _items.len()) return null;
        item = _items[index];
        lastIndex = _items.len() - 1;
        if (index != lastIndex) {
            tmp = _items[lastIndex];
            _items[lastIndex] = _items[index];
            _items[index] = tmp;
        }
        _items.pop_back();
        return item;
    }

    fn removeByName(name) {
        found = null;
        foundIndex = -1;
        for (i = 0; i < _items.len(); i++) {
            if (_items[i].name() == name) {
                found = _items[i];
                foundIndex = i;
                break;
            }
        }
        if (foundIndex == -1) return null;
        return removeByIndex(foundIndex);
    }

    fn findFirst(kind) {
        for (i = 0; i < _items.len(); i++) {
            if (_items[i].kind() == kind) return _items[i];
        }
        return null;
    }

    fn isEmpty() {
        return _items.len() == 0;
    }

    fn len() {
        return _items.len();
    }

    fn get(index) {
        if (index < 0 || index >= _items.len()) return null;
        return _items[index];
    }

    fn describe() {
        if (_items.len() == 0) return "[]";
        parts = [];
        for (i = 0; i < _items.len(); i++) {
            parts.append(_items[i].info());
        }
        text = "[";
        for (i = 0; i < parts.len(); i++) {
            text += parts[i];
            if (i + 1 < parts.len()) text += ", ";
        }
        text += "]";
        return text;
    }
}


class Stats {
    _hp = 0;
    _maxHp = 0;
    _atk = 0;
    _def = 0;
    _spd = 0;

    fn Stats(hp_, atk_, def_, spd_) {
        _hp = hp_;
        _maxHp = hp_;
        _atk = atk_;
        _def = def_;
        _spd = spd_;
    }

    fn isAlive() {
        return _hp > 0;
    }

    fn takeDamage(dmg) {
        _hp = _hp - dmg;
        if (_hp < 0) _hp = 0;
    }

    fn heal(amount) {
        _hp = _hp + amount;
        if (_hp > _maxHp) _hp = _maxHp;
    }

    fn hp() {
        return _hp;
    }

    fn maxHp() {
        return _maxHp;
    }

    fn atk() {
        return _atk;
    }

    fn def() {
        return _def;
    }

    fn spd() {
        return _spd;
    }

    fn info() {
        return "HP={_hp}/{_maxHp}, ATK={_atk}, DEF={_def}, SPD={_spd}";
    }
}

class Npc {
    _name = "";
    _role = "";
    _stats = 0;
    _inventory = 0;
    _aiKind = "";
    _mood = 0;

    fn Npc(name_, role_, stats_, aiKind_) {
        _name = name_;
        _role = role_;
        _stats = stats_;
        _aiKind = aiKind_;
        _inventory = Inventory();
        _mood = 0;
    }

    fn name() {
        return _name;
    }

    fn stats() {
        return _stats;
    }

    fn inv() {
        return _inventory;
    }

    fn isAlive() {
        return _stats.isAlive();
    }

    fn healSmall() {
        _stats.heal(5);
    }

    fn describe() {
        return "{_name}({_role}) { _stats.info() } inv={ _inventory.describe() }";
    }

    fn mood() {
        return _mood;
    }

    fn changeMood(delta) {
        _mood = _mood + delta;
        _mood = clamp(_mood, -5, 5);
    }

    fn decideActionInBattle() {
        if (!_stats.isAlive()) return "dead";

        if (_stats.hp() < _stats.maxHp() * 0.3) {
            if (_inventory.findFirst("potion") != null) {
                if (rand.get(0.0, 1.0) < 0.7) return "heal";
            }
        }

        r = rand.get(0.0, 1.0);
        if (_aiKind == "aggressive") {
            if (r < 0.8) return "attack";
            return "wait";
        } else if (_aiKind == "cautious") {
            if (r < 0.5) return "attack";
            if (r < 0.8) return "wait";
            return "defend";
        } else {
            if (r < 0.6) return "attack";
            if (r < 0.8) return "heal";
            return "wait";
        }
    }
}


class Enemy {
    _name = "";
    _kind = "";
    _stats = 0;
    _lootTable = [];

    fn Enemy(name_, kind_, stats_) {
        _name = name_;
        _kind = kind_;
        _stats = stats_;
        _lootTable = [];
    }

    fn addLoot(item) {
        _lootTable.append(item);
    }

    fn name() {
        return _name;
    }

    fn stats() {
        return _stats;
    }

    fn isAlive() {
        return _stats.isAlive();
    }

    fn randomLoot() {
        if (_lootTable.len() == 0) return null;
        idx = rand.get(0, _lootTable.len() - 1);
        return _lootTable[idx];
    }

    fn describe() {
        return "{_name}({_kind}) { _stats.info() }";
    }
}

class Encounter {
    _enemies = [];

    fn Encounter() { }

    fn addEnemy(e) {
        _enemies.append(e);
    }

    fn allEnemies() {
        return _enemies;
    }

    fn isCleared() {
        for (i = 0; i < _enemies.len(); i++) {
            if (_enemies[i].isAlive()) return 0;
        }
        return 1;
    }

    fn describe() {
        if (_enemies.len() == 0) return "No enemies.";
        for (i = 0; i < _enemies.len(); i++) {
            println("  " + _enemies[i].describe());
        }
    }

    fn getRandomAliveEnemy() {
        alive = [];
        for (i = 0; i < _enemies.len(); i++) {
            if (_enemies[i].isAlive()) alive.append(_enemies[i]);
        }
        if (alive.len() == 0) return null;
        idx = rand.get(0, alive.len() - 1);
        return alive[idx];
    }

    fn getRandomAliveEnemyIndex() {
        aliveIndexes = [];
        for (i = 0; i < _enemies.len(); i++) {
            if (_enemies[i].isAlive()) aliveIndexes.append(i);
        }
        if (aliveIndexes.len() == 0) return -1;
        idx = rand.get(0, aliveIndexes.len() - 1);
        return aliveIndexes[idx];
    }
}


class DialogueLine {
    _speaker = "";
    _text = "";

    fn DialogueLine(speaker, text) {
        _speaker = speaker;
        _text = text;
    }

    fn render() {
        println("{_speaker}: {_text}");
    }
}

class Dialogue {
    _lines = [];

    fn Dialogue() { }

    fn addLine(speaker, text) {
        _lines.append(DialogueLine(speaker, text));
    }

    fn play() {
        for (i = 0; i < _lines.len(); i++) {
            _lines[i].render();
        }
    }
}


class Party {
    _members = [];
    _gold = 0;

    fn Party() { }

    fn addMember(npc) {
        _members.append(npc);
    }

    fn members() {
        return _members;
    }

    fn aliveCount() {
        count = 0;
        for (i = 0; i < _members.len(); i++) {
            if (_members[i].isAlive()) count++;
        }
        return count;
    }

    fn isWiped() {
        return aliveCount() == 0;
    }

    fn gainGold(amount) {
        _gold = _gold + amount;
    }

    fn gold() {
        return _gold;
    }

    fn findFirstAlive() {
        for (i = 0; i < _members.len(); i++) {
            if (_members[i].isAlive()) return _members[i];
        }
        return null;
    }

    fn randomAlive() {
        alive = [];
        for (i = 0; i < _members.len(); i++) {
            if (_members[i].isAlive()) alive.append(_members[i]);
        }
        if (alive.len() == 0) return null;
        idx = rand.get(0, alive.len() - 1);
        return alive[idx];
    }

    fn describe() {
        println("Party (gold={_gold}):");
        for (i = 0; i < _members.len(); i++) {
            println("  " + _members[i].describe());
        }
    }

    fn distributeLoot(item) {
        if (item == null) return;
        target = randomAlive();
        if (target == null) return;
        target.inv().add(item);
        println("  -> {target.name()} obtained {item.info()}");
    }
}


class Battle {
    _party = 0;
    _enc = 0;
    _turn = 1;

    fn Battle(party, enc) {
        _party = party;
        _enc = enc;
        _turn = 1;
    }

    fn isOver() {
        if (_party.isWiped()) return 1;
        if (_enc.isCleared()) return 1;
        return 0;
    }

    fn partyAct() {
        for (i = 0; i < _party.members().len(); i++) {
            actor = _party.members()[i];
            if (!actor.isAlive()) continue;

            action = actor.decideActionInBattle();

            if (action == "attack") {
                target = _enc.getRandomAliveEnemy();
                if (target == null) return;

                atk = actor.stats().atk();
                def = target.stats().def();
                base = atk - def;
                if (base < 1) base = 1;
                dmg = base + rand.get(0, 3);
                println("{actor.name()} attacks {target.name()} for {dmg}");
                target.stats().takeDamage(dmg);
            } else if (action == "heal") {
                potion = actor.inv().findFirst("potion");
                if (potion == null) {
                    println("{actor.name()} wanted to heal, but has no potion.");
                } else {
                    actor.inv().removeByName(potion.name());
                    actor.healSmall();
                    println("{actor.name()} uses {potion.name()} and heals.");
                }
            } else if (action == "defend") {
                println("{actor.name()} defends cautiously.");
            } else if (action == "wait") {
                println("{actor.name()} is watching the situation.");
            } else if (action == "dead") {
                // do nothing
            }
        }
    }

    fn enemiesAct() {
        for (i = 0; i < _enc.allEnemies().len(); i++) {
            e = _enc.allEnemies()[i];
            if (!e.isAlive()) continue;

            target = _party.randomAlive();
            if (target == null) return;

            atk = e.stats().atk();
            def = target.stats().def();
            base = atk - def;
            if (base < 1) base = 1;
            variance = rand.get(0, 2);
            dmg = base + variance;
            println("{e.name()} hits {target.name()} for {dmg}");
            target.stats().takeDamage(dmg);
        }
    }

    fn dropLoot() {
        println("Battle cleared. Checking loot...");
        for (i = 0; i < _enc.allEnemies().len(); i++) {
            e = _enc.allEnemies()[i];
            if (!e.isAlive()) continue;
        }
        for (i = 0; i < _enc.allEnemies().len(); i++) {
            e = _enc.allEnemies()[i];
            if (!e.isAlive()) continue;
        }

        for (i = 0; i < _enc.allEnemies().len(); i++) {
            e = _enc.allEnemies()[i];
            if (e.isAlive()) continue;

            loot = e.randomLoot();
            if (loot != null) {
                println(" Enemy {e.name()} dropped {loot.info()}");
                _party.distributeLoot(loot);
            }
        }

        goldReward = rand.get(10, 40);
        _party.gainGold(goldReward);
        println("Party gained {goldReward} gold.");
    }

    fn run() {
        println("=== Battle Start ===");
        _enc.describe();
        _party.describe();

        for (; !isOver(); ) {
            println("--- Turn {_turn} ---");
            partyAct();
            if (_enc.isCleared()) break;
            enemiesAct();
            _turn++;
        }

        if (_party.isWiped()) {
            println("The party was wiped out...");
        } else {
            println("The party survived the encounter.");
            dropLoot();
        }

        println("=== Battle End ===");
    }
}


class Location {
    _name = "";
    _kind = "";
    _danger = 0;
    _neighbors = [];
    _npcDialogues = [];

    fn Location(name_, kind, danger) {
        _name = name_;
        _kind = kind;
        _danger = danger;
        _neighbors = [];
        _npcDialogues = [];
    }

    fn name() {
        return _name;
    }

    fn addNeighbor(loc) {
        _neighbors.append(loc);
    }

    fn randomNeighbor() {
        if (_neighbors.len() == 0) return null;
        idx = rand.get(0, _neighbors.len() - 1);
        return _neighbors[idx];
    }

    fn dangerLevel() {
        return _danger;
    }

    fn addDialogue(dialog) {
        _npcDialogues.append(dialog);
    }

    fn randomDialogue() {
        if (_npcDialogues.len() == 0) return null;
        idx = rand.get(0, _npcDialogues.len() - 1);
        return _npcDialogues[idx];
    }

    fn describe() {
        return "{_name} ({_kind}, danger={_danger})";
    }
}

class World {
    _locations = [];
    _start = 0;

    fn World() { }

    fn addLocation(loc) {
        _locations.append(loc);
        if (!_start) _start = loc;
    }

    fn startLocation() {
        return _start;
    }

    fn allLocations() {
        return _locations;
    }
}


class AdventureContext {
    _party = 0;
    _world = 0;
    _currentLocation = 0;

    fn AdventureContext(party_, world_, startLoc) {
        _party = party_;
        _world = world_;
        _currentLocation = startLoc;
    }

    fn party() {
        return _party;
    }

    fn world() {
        return _world;
    }

    fn currentLocation() {
        return _currentLocation;
    }

    fn moveTo(loc) {
        _currentLocation = loc;
    }

    fn describeState() {
        println();
        println("== Adventure State ==");
        println("Location: { _currentLocation.describe() }");
        _party.describe();
        println();
    }
}

class Adventure {
    _ctx = 0;

    fn Adventure(ctx) {
        _ctx = ctx;
    }

    fn randomEncounter() {
        loc = _ctx.currentLocation();
        d = loc.dangerLevel();
        if (d <= 0) return;

        chance = d * 0.25;
        r = rand.get(0.0, 1.0);
        if (r > chance) return;

        eStats = Stats(20 + d * 5, 4 + d, 1 + d, 5);
        e = Enemy("Goblin", "beast", eStats);
        e.addLoot(Item("Minor Potion", "potion", 0, 5));
        e.addLoot(Item("Rusty Dagger", "weapon", 1, 2));

        enc = Encounter();
        enc.addEnemy(e);

        battle = Battle(_ctx.party(), enc);
        battle.run();
    }

    fn randomTreasure() {
        loc = _ctx.currentLocation();
        d = loc.dangerLevel();
        r = rand.get(0.0, 1.0);
        if (r > 0.2 + d * 0.1) return;

        println("The party found a small chest.");

        roll = rand.get(0, 100);
        item = null;
        if (roll < 40) {
            item = Item("Herb", "potion", 0, 3);
        } else if (roll < 70) {
            item = Item("Iron Sword", "weapon", 3, 15);
        } else {
            item = Item("Shiny Gem", "treasure", 0, 25);
        }

        if (item != null) {
            _ctx.party().distributeLoot(item);
        }
    }

    fn randomNpcInteraction() {
        loc = _ctx.currentLocation();
        r = rand.get(0.0, 1.0);
        if (r > 0.25) return;

        dialog = loc.randomDialogue();
        if (dialog == null) return;

        println("The party meets someone at {loc.name()}.");
        dialog.play();

        moodDelta = rand.get(-1, 2);
        for (i = 0; i < _ctx.party().members().len(); i++) {
            _ctx.party().members()[i].changeMood(moodDelta);
        }
    }

    fn randomTravel() {
        loc = _ctx.currentLocation();
        next = loc.randomNeighbor();
        if (next == null) {
            println("The party remains at {loc.name()}.");
            return;
        }

        println("The party travels from {loc.name()} to {next.name()}.");
        _ctx.moveTo(next);
    }

    fn step(day, stepCount) {
        println("== Day {day}, Step {stepCount} ==");
        _ctx.describeState();

        randomEncounter();
        if (_ctx.party().isWiped()) return;

        randomTreasure();
        randomNpcInteraction();
        randomTravel();
    }

    fn run(days, stepsPerDay) {
        for (d = 1; d <= days; d++) {
            for (s = 1; s <= stepsPerDay; s++) {
                if (_ctx.party().isWiped()) {
                    println("Adventure ends: party wiped out.");
                    return;
                }
                step(d, s);
            }
        }
        println("Adventure ends: the party survived.");
    }
}

// =========================
// World setup
// =========================

fn build_sample_world() {
    town = Location("Oak Town", "town", 0);
    forest = Location("Greenwood", "forest", 1);
    cave = Location("Dark Cave", "dungeon", 2);
    ruins = Location("Ancient Ruins", "dungeon", 3);

    town.addNeighbor(forest);
    forest.addNeighbor(town);
    forest.addNeighbor(cave);
    cave.addNeighbor(forest);
    cave.addNeighbor(ruins);
    ruins.addNeighbor(cave);

    d1 = Dialogue();
    d1.addLine("Old Man", "Travelers, beware the cave ahead.");
    d1.addLine("Old Man", "Many have entered, few have returned.");

    d2 = Dialogue();
    d2.addLine("Merchant", "Fresh potions and gear! Take a look.");
    d2.addLine("Merchant", "You never know when you'll need them.");

    d3 = Dialogue();
    d3.addLine("Mysterious Stranger", "The ruins remember the old king.");
    d3.addLine("Mysterious Stranger", "Do you seek power or answers?");

    town.addDialogue(d1);
    town.addDialogue(d2);
    ruins.addDialogue(d3);

    world = World();
    world.addLocation(town);
    world.addLocation(forest);
    world.addLocation(cave);
    world.addLocation(ruins);

    return world;
}

// =========================
// Party setup
// =========================

fn build_sample_party() {
    s1 = Stats(35, 7, 3, 5);
    s2 = Stats(28, 5, 4, 6);
    s3 = Stats(22, 4, 2, 7);

    n1 = Npc("Ela", "Warrior", s1, "aggressive");
    n2 = Npc("Mio", "Cleric", s2, "balanced");
    n3 = Npc("Rin", "Rogue", s3, "cautious");

    n2.inv().add(Item("Small Potion", "potion", 0, 5));
    n2.inv().add(Item("Small Potion", "potion", 0, 5));
    n3.inv().add(Item("Throwing Knife", "weapon", 1, 3));

    party = Party();
    party.addMember(n1);
    party.addMember(n2);
    party.addMember(n3);

    return party;
}


fn main() {
    rand.randomize_timer();

    println("=== ylang Text RPG Adventure Simulation ===");

    world = build_sample_world();
    party = build_sample_party();

    ctx = AdventureContext(party, world, world.startLocation());
    adv = Adventure(ctx);

    adv.run(5, 3);

    println("=== Final Party State ===");
    party.describe();
    println("Gold: {party.gold()}");
}
