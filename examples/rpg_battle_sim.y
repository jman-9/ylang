include rand;

class Character {
    _name = "";
    _hp = 100.0;
    _attack = 10.0;
    _defense = 3.0;
    _crit = 0.1;
    _alive = 1;

    fn Character(name, hp, attack, defense, crit) {
        _name = name;
        _hp = hp;
        _attack = attack;
        _defense = defense;
        _crit = crit;
        _alive = 1;
    }

    fn isAlive() {
        return _alive;
    }

    fn takeDamage(dmg) {
        _hp = _hp - dmg;
        if (_hp <= 0) {
            _hp = 0;
            _alive = 0;
        }
    }

    fn attackTarget(target) {
        if (!_alive) {
            println("{_name} is unable to attack.");
            return;
        }

        base = _attack;
        real = base;

        if (rand.get(0.0, 1.0) < _crit) {
            real = real * 2.0;
            println("{_name} lands a CRITICAL HIT!");
        }

        dmg = real - target._defense;
        if (dmg < 0) dmg = 0;

        println("{_name} attacks {target._name} for {dmg}");
        target.takeDamage(dmg);
    }

    fn status() {
        return "{_name} [HP={_hp}]";
    }
}

class Party {
    _members = [];

    fn add(c) {
        _members.append(c);
    }

    fn aliveCount() {
        count = 0;
        for (i = 0; i < _members.len(); i++) {
            if (_members[i].isAlive()) count++;
        }
        return count;
    }

    fn getRandomAlive() {
        alive_list = [];
        for (i = 0; i < _members.len(); i++) {
            if (_members[i].isAlive())
                alive_list.append(_members[i]);
        }

        if (alive_list.len() == 0)
            return null;

        idx = rand.get(0, alive_list.len() - 1);
        return alive_list[idx];
    }

    fn debug() {
        for (i = 0; i < _members.len(); i++) {
            println(_members[i].status());
        }
    }
}

class Battle {
    _partyA = 0;
    _partyB = 0;

    fn Battle(a, b) {
        _partyA = a;
        _partyB = b;
    }

    fn step() {
        if (_partyA.aliveCount() == 0 || _partyB.aliveCount() == 0) {
            return 0;
        }

        a = _partyA.getRandomAlive();
        b = _partyB.getRandomAlive();
        if (a != null && b != null) {
            a.attackTarget(b);
        }

        aa = _partyA.getRandomAlive();
        bb = _partyB.getRandomAlive();
        if (aa != null && bb != null) {
            bb.attackTarget(aa);
        }
    }

    fn run() {
        for (turn = 1; _partyA.aliveCount() > 0 && _partyB.aliveCount() > 0; turn++) {
            println("------ Turn {turn} ------");
            step();

            println("Party A:");
            _partyA.debug();

            println("Party B:");
            _partyB.debug();

            println("");
        }

        if (_partyA.aliveCount() > 0)
            println("Party A wins!");
        else
            println("Party B wins!");
    }
}

fn main() {
    rand.randomize_timer();

    a = Party();
    a.add(Character("Alice", 120.0, 12.0, 4.0, 0.2));
    a.add(Character("Bob",   100.0, 10.0, 3.0, 0.1));
    a.add(Character("Clara",  80.0, 15.0, 2.0, 0.3));

    b = Party();
    b.add(Character("Orc",    130.0, 11.0, 3.0, 0.15));
    b.add(Character("Goblin",  70.0,  8.0, 1.0, 0.05));
    b.add(Character("Troll",  160.0, 14.0, 4.0, 0.12));

    battle = Battle(a, b);
    battle.run();
}