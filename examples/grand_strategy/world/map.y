class Tile {
    terrain = "";
    fertility = 0.0;
    river = false;
    owner_id = 0;
    city_id = 0;
    walkable = true;
    move_cost = 1;
}

class Map {
    width = 0;
    height = 0;
    tiles = 0;

    fn Map(w, h){
        width = w;
        height = h;
        tiles = [];
        for(y=0;y<height;y+=1){
            row = [];
            for(x=0;x<width;x+=1){
                t = Tile();
                n = (x * 17 + y * 31) % 100 / 100.0;
                if(n < 0.2){
                    t.terrain = "ocean";
                    t.walkable = false;
                    t.move_cost = 999;
                } else if(n < 0.4){
                    t.terrain = "plains";
                    t.fertility = 0.7;
                    t.move_cost = 1;
                } else if(n < 0.7){
                    t.terrain = "forest";
                    t.fertility = 0.6;
                    t.move_cost = 2;
                } else {
                    t.terrain = "hill";
                    t.fertility = 0.3;
                    t.move_cost = 3;
                }
                if((x + y) % 9 == 0 && t.terrain != "ocean"){
                    t.river = true;
                    t.fertility += 0.1;
                }
                row.append(t);
            }
            tiles.append(row);
        }
    }

    fn in_bounds(x, y){
        if(x < 0 || y < 0) return false;
        if(x >= width || y >= height) return false;
        return true;
    }

    fn get(x, y){
        if(!in_bounds(x, y)) return null;
        return tiles[y][x];
    }

    fn overview(){
        land = 0;
        ocean = 0;
        for(y=0;y<height;y+=1){
            for(x=0;x<width;x+=1){
                t = tiles[y][x];
                if(t.terrain == "ocean") ocean += 1;
                else land += 1;
            }
        }
        println("Map {width}x{height} land={land} ocean={ocean}");
    }
}
