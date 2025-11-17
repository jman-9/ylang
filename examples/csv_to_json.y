// CSV <-> JSON full roundtrip test
// Covers file I/O, string, list, dict, and json modules.

include file;
include json;

fn save_text(path, text) {
    f = file.open(path, "w+");
    f.write(text);
    f.close();
}

fn read_text(path) {
    f = file.open(path, "r");
    s = f.read(100000);
    f.close();
    return s;
}

fn split_csv_line(line) {
    return line.split(",");
}

fn csv_to_json(csv_text) {
    lines = csv_text.trim().split("\n");
    header = split_csv_line(lines[0].trim());

    arr = [];

    for (i = 1; i < lines.len(); i += 1) {
        cols = split_csv_line(lines[i].trim());
        row = {};
        for (j = 0; j < header.len(); j += 1) {
            row[header[j]] = cols[j];
        }
        arr.append(row);
    }
    return arr;
}

fn json_to_csv(json_arr) {
    if (json_arr.len() == 0) {
        return "";
    }

    header = json_arr[0].keys();
    out = [",".join(header)];
    for (i=0; i<json_arr.len(); i += 1) {
        line = [];
        for (j=0; j<header.len(); j += 1) {
            line.append(json_arr[i][header[j]]);
        }
        out.append(",".join(line));
    }
    return "\n".join(out);
}

fn verify_csv_equivalence(a_text, b_text) {
    a_lines = a_text.trim().split("\n");
    b_lines = b_text.trim().split("\n");

    if (a_lines.len() != b_lines.len()) {
        println("[FAIL] Line count mismatch");
        return 0;
    }

    for (i = 0; i < a_lines.len(); i += 1) {
        if (a_lines[i].trim() != b_lines[i].trim()) {
            println("[FAIL] Row mismatch at line {i}");
            println("  A: " + a_lines[i]);
            println("  B: " + b_lines[i]);
            return 0;
        }
    }
    println("[PASS] CSV verified identical");
    return 1;
}

fn main() {
    println("=== CSV <-> JSON Roundtrip Demo ===");
    println("This example shows how to:");
    println("  1. Write a CSV file directly from code");
    println("  2. Convert CSV to JSON");
    println("  3. Save JSON to a file");
    println("  4. Read JSON back and convert it to CSV again");
    println("  5. Verify both CSV versions are identical\n");

    println("[1] Creating test CSV file...");	
    csv_data = '''
      name,passed,score
      Alice,true,91
      Bob,false,68
      Charlie,true,84
	''';
    save_text("data.csv", csv_data);

    println("[2] Reading CSV file...");
    csv_text = read_text("data.csv");

    println("[3] Converting CSV -> JSON...");
    json_arr = csv_to_json(csv_text);

    println("[4] Saving JSON file...");
    json_text = json.dump(json_arr, 2);
    save_text("data.json", json_text);

    println("[5] Displaying both CSV and JSON:");
    println("--- CSV Input ---");
    println(csv_text);
    println("--- JSON Dump ---");
    println(json_text);

    println("[6] Reading JSON file back and parsing...");
    json_text2 = read_text("data.json");
    json_arr2 = json.parse(json_text2);

    println("[7] Converting JSON -> CSV again...");
    csv_back = json_to_csv(json_arr2);
    save_text("data_back.csv", csv_back);

    println("[8] Verifying both CSV versions...");
    if (verify_csv_equivalence(csv_text, csv_back)) {
        println("[PASS] Roundtrip OK: CSV <-> JSON verified");
    } else {
        println("[FAIL] Roundtrip failed");
    }

    println("\nFiles generated:");
    println("  - data.csv       (original CSV)");
    println("  - data.json      (converted JSON)");
    println("  - data_back.csv  (reconstructed CSV)");
    println("====================================");
}
