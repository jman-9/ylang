include file;
include json;
include sys;


fn csv_to_json(csv_text) {
    lines = csv_text.trim().split("\n");
    header = lines[0].trim().split(",");

    arr = [];

    for (i = 1; i < lines.size(); i++) {
		cols = lines[i].trim().split(",");
        row = {};
        for (j = 0; j < header.size(); j++) {
            row[header[j]] = cols[j];
        }
        arr.push_back(row);
    }
    return arr;
}

fn main()
{
	if(sys.args.size() < 2)
	{
		eprintln("usage) {sys.args[0]} <csv_path> [indent]");
		exit(1);
	}
    csv_text = file.read_all(sys.args[1]);
	if(!csv_text)
	{
		eprintln("File not found or empty file");
		exit(1);
	}
	
	indent = -1;
	if(sys.args.size() > 2)
	{
		indent = sys.args[2].to_int();
	}
	
    js = csv_to_json(csv_text);
    json_text = json.dump(js, indent);
    println(json_text);
}
