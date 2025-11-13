fn clear_screen() { print("\033[H\033[J"); }
fn hide_cursor() { print("\033[?25l"); }
fn show_cursor() { print("\033[?25h"); }
fn move_cursor_home() { print("\033[H"); }

fn make_grid(w, h) {
    g = [];
    for (i = 0; i < h; i += 1) {
        row = [];
        for (j = 0; j < w; j += 1) {
            row.append(0);
        }
        g.append(row);
    }
    return g;
}

fn show_grid(g, ax, ay) {
    for (y = 0; y < g.len(); y += 1) {
        line = "";
        for (x = 0; x < g[0].len(); x += 1) {
            if (x == ax && y == ay) {
                line += "\033[33m@\033[0m";
            } else if (g[y][x] == 1) {
                line += "\033[37m#\033[0m";
            } else {
                line += "\033[90m.\033[0m";
            }
        }
        println(line);
    }
}

fn turn_right(d) { return (d + 1) % 4; }
fn turn_left(d)  { return (d + 3) % 4; }

fn dir_name(d) {
    if (d == 0) return "UP   ";
    if (d == 1) return "RIGHT";
    if (d == 2) return "DOWN ";
    return "LEFT ";
}

fn step(g, x, y, d) {
    c = g[y][x];
    if (c == 0) {
        d = turn_right(d);
        g[y][x] = 1;
    } else {
        d = turn_left(d);
        g[y][x] = 0;
    }

    if (d == 0) { y -= 1; }
    if (d == 1) { x += 1; }
    if (d == 2) { y += 1; }
    if (d == 3) { x -= 1; }

    if (x < 0) { x = g[0].len() - 1; }
    if (x >= g[0].len()) { x = 0; }
    if (y < 0) { y = g.len() - 1; }
    if (y >= g.len()) { y = 0; }

    return {'x': x, 'y': y, 'd': d};
}


fn main() {
	W = 40;
	H = 20;

	grid = make_grid(W, H);
	x = W / 2;
	y = H / 2;
	d = 0;

	clear_screen();
	hide_cursor();

	for (t = 0; t < 500; t += 1) {
		move_cursor_home();
		show_grid(grid, x, y);

		println("\nstep={t}, pos=({x},{y}), dir={dir_name(d)}        ");

		s = step(grid, x, y, d);
		x = s['x'];
		y = s['y'];
		d = s['d'];
	}

	show_cursor();
	println("done");
}
