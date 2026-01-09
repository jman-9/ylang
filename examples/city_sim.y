include rand;
include math;

class Job {
    _name = "";
    _salary = 0;

    fn Job(name, salary) {
        _name = name;
        _salary = salary;
    }

    fn getSalary() {
        return _salary;
    }

    fn info() {
        return "{_name} ({_salary})";
    }
}

class Citizen {
    _name = "";
    _age = 0;
    _job = 0;
    _money = 0;

    fn Citizen(name, age, job) {		
		println(job);
		
        _name = name;
        _age = age;
        _job = job;
		
        _money = 0;
    }

    fn work() {
        salary = _job.getSalary();
        _money += salary;
    }

    fn pay(amount) {
        if (_money >= amount) {
            _money -= amount;
            return 1;
        }
        return 0;
    }

    fn getTax() {
        return math.floor(_money * 0.1);
    }

    fn info() {
        return "{_name} age={_age} money={_money} job={_job.info()}";
    }
}

class Building {
    _name = "";
    _cost = 0;
    _maintenance = 0;

    fn Building(name, cost, maintenance) {
        _name = name;
        _cost = cost;
        _maintenance = maintenance;
    }

    fn upkeep() {
        return _maintenance;
    }

    fn info() {
        return "{_name} (cost={_cost}, upkeep={_maintenance})";
    }
}

class City {
    _name = "";
    _citizens = [];
    _buildings = [];
    _treasury = 0;

    fn City(name) {
        _name = name;
        _treasury = 1000;
    }

    fn addCitizen(c) {
        _citizens.push_back(c);
    }

    fn addBuilding(b) {
        _buildings.push_back(b);
    }

    fn collectTaxes() {
        total = 0;
        for (i = 0; i < _citizens.size(); i++) {
            tax = _citizens[i].getTax();
            total = total + tax;
        }
        _treasury = _treasury + total;
        return total;
    }

    fn payMaintenance() {
        cost = 0;
        for (i = 0; i < _buildings.size(); i++) {
            cost = cost + _buildings[i].upkeep();
        }
        if (_treasury >= cost) {
            _treasury = _treasury - cost;
        } else {
            _treasury = _treasury - cost;
        }
        return cost;
    }

    fn simulateDay() {
        for (i = 0; i < _citizens.size(); i++) {
            _citizens[i].work();
        }
        taxes = collectTaxes();
        upkeep = payMaintenance();
        println("Collected tax = {taxes}, Maintenance = {upkeep}, treasury = {_treasury}");
    }

    fn info() {
        println("=== City {_name} ===");
        for (i = 0; i < _citizens.size(); i++) {
            println(_citizens[i].info());
        }
        for (i = 0; i < _buildings.size(); i++) {
            println(_buildings[i].info());
        }
        println("Treasury: {_treasury}");
    }
}

fn main() {
    rand.randomize_timer();

    farmer = Job("Farmer", 30);
    engineer = Job("Engineer", 60);
    artist = Job("Artist", 20);

    c1 = Citizen("Alice", 30, farmer);
    c2 = Citizen("Bob",   42, engineer);
    c3 = Citizen("Eve",   24, artist);

    townhall = Building("Town Hall", 500, 20);
    farm = Building("Farm", 200, 10);
    workshop = Building("Workshop", 400, 15);

    city = City("Sampletown");
    city.addCitizen(c1);
    city.addCitizen(c2);
    city.addCitizen(c3);

    city.addBuilding(townhall);
    city.addBuilding(farm);
    city.addBuilding(workshop);

    city.info();
    println();

    for (day = 1; day <= 5; day++) {
        println("---- Day {day} ----");
        city.simulateDay();
    }

    println();
    city.info();
}
