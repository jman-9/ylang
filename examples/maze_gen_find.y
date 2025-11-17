// Visual Maze Generation + A* Pathfinding
// ANSI-based frame rendering like langton_ant.y

include json;
include file;
include math;
include rand;

fn clear_screen() { print("\033[H\033[J"); }
fn move_home() { print("\033[H"); }
fn hide_cursor() { print("\033[?25l"); }
fn show_cursor() { print("\033[?25h"); }

// -------- file helpers --------
fn save_text(path, text) {
    f = file.open(path, "w+");
    f.write(text);
    f.close();
}

// -------- data helpers --------
fn in_bounds(x, y, w, h) {
    return (x >= 0 && y >= 0 && x < w && y < h);
}

fn make_grid(w, h, fill) {
    g = [];
    for (yy = 0; yy < h; yy += 1) {
        row = [];
        for (xx = 0; xx < w; xx += 1) {
            row.append(fill);
        }
        g.append(row);
    }
    return g;
}

// -------- maze generation (DFS) --------
fn shuffle_dirs(dirs) {
    for (i = dirs.len() - 1; i > 0; i -= 1) {
        j = rand.get(0, i);
        tmp = dirs[i];
        dirs[i] = dirs[j];
        dirs[j] = tmp;
    }
    return dirs;
}

fn carve_maze(w, h, sx, sy) {
    grid = make_grid(w, h, 0);
    stack = [];
    grid[sy][sx] = 1;
    stack.append([sx, sy]);

    for (; stack.len() > 0;) {
        cur = stack[stack.len() - 1];
        cx = cur[0]; cy = cur[1];
        dirs = [[0,-2],[2,0],[0,2],[-2,0]];
        dirs = shuffle_dirs(dirs);
        progressed = 0;

        for (i=0;i<dirs.len();i+=1) {
            d = dirs[i];
            nx = cx + d[0]; ny = cy + d[1];
            if (!in_bounds(nx, ny, w, h)) continue;
            if (grid[ny][nx] == 0) {
                mx = (cx+nx)/2; my=(cy+ny)/2;
                grid[my][mx]=1; grid[ny][nx]=1;
                stack.append([nx,ny]);
                progressed=1; break;
            }
        }
        if(!progressed) stack.pop_back();
    }
    return grid;
}

fn key(x,y) { return "{x},{y}"; }

// -------- A* + Visual Rendering --------
fn render_visual(grid, start, goal, open_list, visited_mark, path_mark) {
    w = grid[0].len();
    h = grid.len();

    // Build marks for open nodes
    open_mark = {};
    for (i = 0; i < open_list.len(); i += 1) {
        ox = open_list[i][1];
        oy = open_list[i][2];
        k = key(ox, oy);
        open_mark[k] = 1;
    }

    // Draw grid with colors
    for (y = 0; y < h; y += 1) {
        line = "";
        for (x = 0; x < w; x += 1) {
            k = key(x, y);

            // Start & Goal
            if (x == start[0] && y == start[1]) {
                line += "\033[33mS\033[0m";      // Yellow: Start
            }
            else if (x == goal[0] && y == goal[1]) {
                line += "\033[31mG\033[0m";      // Red: Goal
            }
            // Final path
            else if (path_mark.contains(k)) {
                line += "\033[36m@\033[0m";      // Cyan: Path
            }
            // Frontier (open set)
            else if (open_mark.contains(k)) {
                line += "\033[44m+\033[0m";      // Blue: Frontier
            }
            // Visited (closed set)
            else if (visited_mark.contains(k)) {
                line += "\033[32m.\033[0m";      // Green: Visited
            }
            // Wall
            else if (grid[y][x] == 0) {
                line += "\033[90m#\033[0m";      // Dark gray wall
            }
            // Empty space
            else {
                line += " ";
            }
        }
        println(line);
    }
}

fn heuristic(ax, ay, bx, by) {
    dx=ax-bx; dy=ay-by;
    //return math.sqrt(dx*dx+dy*dy);
	return dx*dx+dy*dy;
	//if(dx < 0) dx = -dx;
	//if(dy < 0) dy = -dy;
	//return dx + dy;
}

fn neighbors4(x,y) { return [[x+1,y],[x-1,y],[x,y+1],[x,y-1]]; }

fn visual_astar(grid, sx, sy, gx, gy) {
    w = grid[0].len();
    h = grid.len();

    open_list = [];               // list of [f, x, y]
    came = {};                    // "x,y" -> [px, py]
    gscore = {};                  // "x,y" -> g
    fscore = {};                  // "x,y" -> f
    visited_mark = {};            // "x,y" -> 1

    ks = key(sx, sy);
    gscore[ks] = 0.0;
    fscore[ks] = heuristic(sx, sy, gx, gy);
    open_list.append([fscore[ks], sx, sy]);

    for (step = 0; step < 100000; step += 1) {
        if (open_list.len() == 0) {
            // no path
            move_home();
            println("Step: {step}");
            render_visual(grid, [sx,sy], [gx,gy], open_list, visited_mark, {});
            return 0;
        }

        // pick best (min f) from open_list
        best_i = 0;
        for (i = 1; i < open_list.len(); i += 1) {
            if (open_list[i][0] < open_list[best_i][0]) best_i = i;
        }
        node = open_list[best_i];
        last = open_list[open_list.len() - 1];
        open_list[best_i] = last;
        open_list.pop_back();

        x = node[1];
        y = node[2];

        // render current frontier/explored state
        move_home();
        println("Step: {step}");

        if (x == gx && y == gy) {
            // reconstruct path
            path_mark = {};
            k = key(x, y);
            path_mark[k] = 1;
            for ( ; came.contains(k); ) {
                prev = came[k];
                k = key(prev[0], prev[1]);
                path_mark[k] = 1;
            }

			open_list = [];
			visited_mark = {};

            move_home();
            println("Step: {step} (path found)");
            render_visual(grid, [sx,sy], [gx,gy], open_list, visited_mark, path_mark);
            return 0;
        }

        // mark as visited
        visited_mark[key(x, y)] = 1;

        // relax neighbors
        nb = neighbors4(x, y);
        for (i = 0; i < nb.len(); i += 1) {
            nx = nb[i][0];
            ny = nb[i][1];
            if (!in_bounds(nx, ny, w, h)) continue;
            if (grid[ny][nx] == 0) continue;

            kn = key(nx, ny);
            kc = key(x, y);
            tentative = gscore[kc] + 1.0;

            need_push = !gscore.contains(kn);
            if (!need_push) {
                if (tentative < gscore[kn]) need_push = 1;
            }
            if (need_push) {
                came[kn] = [x, y];
                gscore[kn] = tentative;
                f = tentative + heuristic(nx, ny, gx, gy);
                fscore[kn] = f;
                open_list.append([f, nx, ny]);
            }
        }

		render_visual(grid, [sx,sy], [gx,gy], open_list, visited_mark, {});
    }
}


fn main() {
    W=61; H=25;
    //rand.seed(1337);
	rand.randomize_timer();
    grid=carve_maze(W,H,1,1);
    sx=1; sy=1; gx=W-2; gy=H-2;

    clear_screen();
    hide_cursor();
    visual_astar(grid,sx,sy,gx,gy);
    show_cursor();
    println("done");
}
