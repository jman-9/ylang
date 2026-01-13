class IdGen {
    _next = 0;

    fn IdGen(){
        _next = 1;
    }

    fn next(){
        id = _next;
        _next += 1;
        return id;
    }
}

class Stats {
    fn avg(list){
        if(list.len() == 0) return 0;
        s = 0;
        for(i=0;i<list.len();i+=1){
            s += list[i];
        }
        return s / list.len();
    }

    fn min(list){
        if(list.len() == 0) return 0;
        m = list[0];
        for(i=1;i<list.len();i+=1){
            if(list[i] < m) m = list[i];
        }
        return m;
    }

    fn max(list){
        if(list.len() == 0) return 0;
        m = list[0];
        for(i=1;i<list.len();i+=1){
            if(list[i] > m) m = list[i];
        }
        return m;
    }
}
