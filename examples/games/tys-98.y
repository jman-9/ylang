include sys;
include json;
include file;
include fs;

// -----------------------------
// Global System State
// -----------------------------

world = {
    "running": true,
    "turn": 0,
};

system = {
    "nodes": {},          // id -> node
    "connections": [],    // [from, to]
    "logs": []            // runtime only
};

// -----------------------------
// Components (Node Templates)
// -----------------------------

components = {
    "input": {
        "type": "input",
        "desc": "produces signal each step",
        "rate": 1
    },
    "processor": {
        "type": "processor",
        "desc": "consumes 1 signal -> emits 1 signal",
        "capacity": 4
    },
    "output": {
        "type": "output",
        "desc": "collects signal into received",
		"capacity": 8
    }
};

// -----------------------------
// Utility
// -----------------------------

fn separator() {
    println("------------------------------");
}

fn log(msg) {
    // runtime only
    system["logs"].push_back("T{world['turn']}: {msg}");
}

// -----------------------------
// Save / Load
// -----------------------------

fn save_game(path) {
    // logs are NOT saved
    data = {
        "world": world,
        "system": {
            "nodes": system["nodes"],
            "connections": system["connections"]
        }
    };

    file.write_all(path, json.dump(data));
    println("Saved: " + path);
}

fn load_game(path) {
    if (!fs.exists(path)) {
        println("Save file not found: " + path);
        return;
    }

    data = json.parse(file.read_all(path));

    if (!data.contains("world") || !data.contains("system")) {
        println("Invalid save file.");
        return;
    }

    world = data["world"];

    // keep system container, replace fields
    if (!data["system"].contains("nodes")) { data["system"]["nodes"] = {}; }
    if (!data["system"].contains("connections")) { data["system"]["connections"] = []; }

    system["nodes"] = data["system"]["nodes"];
    system["connections"] = data["system"]["connections"];
    system["logs"] = []; // runtime only, cleared on load

    // invariants
    if (!world.contains("running")) { world["running"] = true; }
    if (!world.contains("turn")) { world["turn"] = 0; }

    // backward compatibility: initialize missing node fields
    keys = system["nodes"].keys();
    for (i = 0; i < keys.size(); i++) {
        id = keys[i];
        node = system["nodes"][id];

        if (!node.contains("out")) { node["out"] = 0; }
        if (!node.contains("buffer")) { node["buffer"] = 0; }
        if (!node.contains("state")) { node["state"] = "idle"; }
    }

    println("Loaded: " + path);
}

// -----------------------------
// Node Management
// -----------------------------

fn add_node(id, comp_type) {
    if (!components.contains(comp_type)) {
        println("Unknown component type.");
        return;
    }

    if (system["nodes"].contains(id)) {
        println("Node already exists.");
        return;
    }

    node = {
        "id": id,
        "component": comp_type,
        "state": "idle",
        "buffer": 0,
        "out": 0
    };

    system["nodes"][id] = node;
    log("Node added: " + id + " (" + comp_type + ")");
}

fn connect_nodes(a, b) {
    if (!system["nodes"].contains(a) || !system["nodes"].contains(b)) {
        println("Invalid node id.");
        return;
    }

    system["connections"].push_back([a, b]);
    log("Connected " + a + " -> " + b);
}

// -----------------------------
// Simulation Step (Flow)
// -----------------------------

fn phase_nodes() {
    keys = system["nodes"].keys();
    for (i = 0; i < keys.size(); i++) {
        id = keys[i];
        node = system["nodes"][id];
        comp = components[node["component"]];

        // reset per-step output
        node["out"] = 0;

        // persistent error state
        if (node["state"] == "overflow") {
            continue;
        }

        if (comp["type"] == "input") {
            // single-buffer semantics: input adds slots to its buffer
            node["buffer"] = node["buffer"] + comp["rate"];
			
			if (node["buffer"] > 0) {
				node["buffer"] -= 1;
				node["out"] = 1;
			}
			
            node["state"] = "producing";
        } else if (comp["type"] == "processor") {
            // overflow check first
            if (node["buffer"] > comp["capacity"]) {
                node["state"] = "overflow";
                log("Overflow at " + id + " (buffer=" + node["buffer"] + " cap=" + comp["capacity"] + ")");
            } else if (node["buffer"] > 0) {
                // consume one slot, try to forward it via out=1
                node["buffer"] = node["buffer"] - 1;
                node["out"] = 1;
                node["state"] = "processing";
            } else {
                node["state"] = "idle";
            }
        } else if (comp["type"] == "output") {
			// initialize received
			if (!node.contains("received")) {
				node["received"] = 0;
			}

			// capacity check (output can overflow too)
			if (node["buffer"] > comp["capacity"]) {
				node["state"] = "overflow";
				log("Overflow at output " + id + " (buffer=" + node["buffer"] + ")");
				return;
			}

			// consume ONE slot per step (like processor)
			if (node["buffer"] > 0) {
				node["buffer"] -= 1;
				node["received"] += 1;
				node["state"] = "receiving";
			} else {
				node["state"] = "idle";
			}
        }
    }
}

fn phase_transfer() {
    // Move src.out along each connection.
    // If a node has multiple outgoing edges, it will deliver to the first edge only.
    // Blocked semantics:
    //   - If downstream is overflow, the transfer fails.
    //   - The slot is returned to src.buffer (so pressure accumulates upstream).
    for (i = 0; i < system["connections"].size(); i++) {
        c = system["connections"][i];
        from = c[0];
        to = c[1];

        src = system["nodes"][from];
        dst = system["nodes"][to];

        if (src["out"] > 0) {
            if (dst["state"] == "overflow") {
                // cannot deliver; return slot back to source buffer
                src["buffer"] = src["buffer"] + src["out"];
                src["out"] = 0;
                src["state"] = "blocked";
                log("Blocked at " + from + " (downstream overflow: " + to + ")");
            } else {
                dst["buffer"] = dst["buffer"] + src["out"];
                src["out"] = 0;
            }
        }
    }
}

fn step_system() {
    world["turn"]++;
    phase_nodes();
    phase_transfer();
}

// -----------------------------
// Views
// -----------------------------

fn show_system() {
    separator();
    println("SYSTEM OVERVIEW");
    println("Turn: " + world["turn"]);

    println("Nodes:");
    keys = system["nodes"].keys();
    for (i = 0; i < keys.size(); i++) {
        id = keys[i];
        n = system["nodes"][id];

        line = "  " + id + " [" + n["component"] + "] state=" + n["state"] + " buffer=" + n["buffer"];
        if (n.contains("received")) {
            line = line + " received=" + n["received"];
        }
        println(line);
    }

    println("Connections:");
    if (system["connections"].size() == 0) {
        println("  none");
    } else {
        for (i = 0; i < system["connections"].size(); i++) {
            c = system["connections"][i];
            println("  " + c[0] + " -> " + c[1]);
        }
    }
}

fn show_components() {
    separator();
    println("AVAILABLE COMPONENTS");
    keys = components.keys();
    for (i = 0; i < keys.size(); i++) {
        name = keys[i];
        comp = components[name];
        println("  " + name + " : " + comp["desc"]);
    }
}

fn show_logs(limit) {
    separator();
    println("LOGS");
    n = system["logs"].size();
    start = 0;

    if (limit > 0 && n > limit) {
        start = n - limit;
    }

    for (i = start; i < n; i++) {
        println("  " + system["logs"][i]);
    }
}

fn inspect_node(id) {
    if (!system["nodes"].contains(id)) {
        println("No such node.");
        return;
    }

    node = system["nodes"][id];
    comp = components[node["component"]];

    separator();
    println("INSPECT " + id);
    println("Component: " + node["component"]);
    println("Type: " + comp["type"]);
    println("State: " + node["state"]);
    println("Buffer: " + node["buffer"]);
    println("Out: " + node["out"]);

    if (comp.contains("capacity")) {
        println("Capacity: " + comp["capacity"]);
    }
    if (comp.contains("rate")) {
        println("Rate: " + comp["rate"]);
    }
    if (node.contains("received")) {
        println("Received: " + node["received"]);
    }

    println("Incoming:");
    found = false;
    for (i = 0; i < system["connections"].size(); i++) {
        c = system["connections"][i];
        if (c[1] == id) {
            println("  " + c[0]);
            found = true;
        }
    }
    if (!found) {
        println("  none");
    }

    println("Outgoing:");
    found = false;
    for (i = 0; i < system["connections"].size(); i++) {
        c = system["connections"][i];
        if (c[0] == id) {
            println("  " + c[1]);
            found = true;
        }
    }
    if (!found) {
        println("  none");
    }
}

// -----------------------------
// Commands
// -----------------------------

fn cmd_help() {
    println("Commands:");
    println("  components");
    println("  add <id> <component>");
    println("  connect <a> <b>");
    println("  step [n]");
    println("  show");
    println("  inspect <id>");
    println("  logs [n]");
    println("  save <file>");
    println("  load <file>");
    println("  quit");
}

fn cmd_components() { show_components(); }

fn cmd_add(args) {
    if (args.size() < 3) {
        println("Usage: add <id> <component>");
        return;
    }
    add_node(args[1], args[2]);
}

fn cmd_connect(args) {
    if (args.size() < 3) {
        println("Usage: connect <a> <b>");
        return;
    }
    connect_nodes(args[1], args[2]);
}

fn cmd_step(args) {
    times = 1;
    if (args.size() >= 2) {
        times = args[1].to_int();
        if (times <= 0) { times = 1; }
    }

    for (i = 0; i < times; i++) {
        step_system();
    }
    println("Stepped: " + times);
}

fn cmd_show() { show_system(); }

fn cmd_inspect(args) {
    if (args.size() < 2) {
        println("Usage: inspect <id>");
        return;
    }
    inspect_node(args[1]);
}

fn cmd_logs(args) {
    limit = 0;
    if (args.size() >= 2) {
        limit = args[1].to_int();
        if (limit < 0) { limit = 0; }
    }
    show_logs(limit);
}

fn cmd_save(args) {
    if (args.size() < 2) {
        println("Usage: save <file>");
        return;
    }
    save_game(args[1]);
}

fn cmd_load(args) {
    if (args.size() < 2) {
        println("Usage: load <file>");
        return;
    }
    load_game(args[1]);
}

fn cmd_quit() {
    world["running"] = false;
    println("Goodbye.");
}

// -----------------------------
// Command Dispatcher
// -----------------------------

fn dispatch_command(line) {
    input = line.trim();
    if (input.size() == 0) {
        return;
    }

    args = input.split(" ");
    verb = args[0];

    if (verb == "help") { cmd_help(); }
    else if (verb == "components") { cmd_components(); }
    else if (verb == "add") { cmd_add(args); }
    else if (verb == "connect") { cmd_connect(args); }
    else if (verb == "step") { cmd_step(args); }
    else if (verb == "show") { cmd_show(); }
    else if (verb == "inspect") { cmd_inspect(args); }
    else if (verb == "logs") { cmd_logs(args); }
    else if (verb == "save") { cmd_save(args); }
    else if (verb == "load") { cmd_load(args); }
    else if (verb == "quit") { cmd_quit(); }
    else { println("Unknown command."); }
}

// -----------------------------
// Top-level execution
// -----------------------------

println("TYS-98 - homage to TIS-100");
println("==========================");
println("Type 'help' for commands.");

for (; world["running"]; ) {
    print("> ");
    line = readln();
    dispatch_command(line);
}
