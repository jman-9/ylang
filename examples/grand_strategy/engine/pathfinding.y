include engine.math;

class NodeRecord {
    x = 0;
    y = 0;
    g = 0.0;
    h = 0.0;
    f = 0.0;
    parent_x = -1;
    parent_y = -1;
}

class PathFinding {
    fn heuristic(ax, ay, bx, by){
        dx = engine.math.Math().abs(ax - bx);
        dy = engine.math.Math().abs(ay - by);
        return dx + dy;
    }

    fn find_path(map, sx, sy, tx, ty){
        w = map.width;
        h = map.height;
        open = [];
        closed = [];

        start = NodeRecord();
        start.x = sx;
        start.y = sy;
        start.g = 0;
        start.h = heuristic(sx, sy, tx, ty);
        start.f = start.h;
        open.append(start);

        max_steps = w * h * 4;
        for(step=0;step<max_steps;step+=1){
            if(open.len() == 0){
                break;
            }

            best_index = 0;
            best_f = open[0].f;
            for(i=1;i<open.len();i+=1){
                if(open[i].f < best_f){
                    best_f = open[i].f;
                    best_index = i;
                }
            }

            current = open[best_index];
            open.remove(best_index);
            closed.append(current);

            if(current.x == tx && current.y == ty){
                return reconstruct_path(closed, tx, ty);
            }

            for(dy=-1;dy<=1;dy+=1){
                for(dx=-1;dx<=1;dx+=1){
                    if(dx == 0 && dy == 0) continue;
                    nx = current.x + dx;
                    ny = current.y + dy;
                    if(nx < 0 || ny < 0 || nx >= w || ny >= h) continue;
                    t = map.get(nx, ny);
                    if(t == null || !t.walkable) continue;

                    if(in_list(closed, nx, ny)) continue;

                    g = current.g + t.move_cost;
                    found = find_in(open, nx, ny);
                    if(found != null){
                        if(g < found.g){
                            found.g = g;
                            found.f = g + found.h;
                            found.parent_x = current.x;
                            found.parent_y = current.y;
                        }
                    } else {
                        nr = NodeRecord();
                        nr.x = nx;
                        nr.y = ny;
                        nr.g = g;
                        nr.h = heuristic(nx, ny, tx, ty);
                        nr.f = nr.g + nr.h;
                        nr.parent_x = current.x;
                        nr.parent_y = current.y;
                        open.append(nr);
                    }
                }
            }
        }
        return [];
    }

    fn in_list(list, x, y){
        for(i=0;i<list.len();i+=1){
            if(list[i].x == x && list[i].y == y) return true;
        }
        return false;
    }

    fn find_in(list, x, y){
        for(i=0;i<list.len();i+=1){
            if(list[i].x == x && list[i].y == y) return list[i];
        }
        return null;
    }

    fn reconstruct_path(closed, tx, ty){
        path = [];
        cx = tx;
        cy = ty;
        max_back = closed.len() + 4;
        for(step=0;step<max_back;step+=1){
            idx = -1;
            for(i=0;i<closed.len();i+=1){
                if(closed[i].x == cx && closed[i].y == cy){
                    idx = i;
                    break;
                }
            }
            if(idx == -1) break;
            rec = closed[idx];
            path.insert(0, { "x": rec.x, "y": rec.y });
            if(rec.parent_x == -1 && rec.parent_y == -1) break;
            cx = rec.parent_x;
            cy = rec.parent_y;
        }
        return path;
    }
}
