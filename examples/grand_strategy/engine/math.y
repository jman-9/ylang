class Math {
    fn clamp(v, lo, hi){
        if(v < lo) return lo;
        if(v > hi) return hi;
        return v;
    }

    fn lerp(a, b, t){
        return a + (b - a) * t;
    }

    fn abs(v){
        if(v < 0) return -v;
        return v;
    }

    fn max(a, b){
        if(a > b) return a;
        return b;
    }

    fn min(a, b){
        if(a < b) return a;
        return b;
    }

    fn pseudo(seed){
        x = seed * 1103515245 + 12345;
        if(x < 0) x = -x;
        return x;
    }

    fn rand01(seed){
        p = pseudo(seed);
        return (p % 10000) / 10000.0;
    }

    fn rand_range(seed, lo, hi){
        t = rand01(seed);
        return lo + (hi - lo) * t;
    }

    fn distance2(ax, ay, bx, by){
        dx = ax - bx;
        dy = ay - by;
        return dx * dx + dy * dy;
    }
}
