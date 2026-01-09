class Diplomacy {
    relations = 0;

    fn Diplomacy(){
        relations = {};
    }

    fn key(a, b){
        if(a < b) return "{a}-{b}";
        return "{b}-{a}";
    }

    fn get(a, b){
        if(a == b) return 100;
        k = key(a, b);
        if(!relations.contains(k)) relations[k] = 0;
        return relations[k];
    }

    fn add(a, b, delta){
        if(a == b) return;
        k = key(a, b);
        if(!relations.contains(k)) relations[k] = 0;
        v = relations[k] + delta;
        if(v > 100) v = 100;
        if(v < -100) v = -100;
        relations[k] = v;
    }

    fn summary(civs){
        println("--- Diplomacy ---");
        for(i=0;i<civs.size();i+=1){
            for(j=i+1;j<civs.size();j+=1){
                ca = civs[i];
                cb = civs[j];
                r = get(ca.id, cb.id);
                println(" {ca.name} - {cb.name}: {r}");
            }
        }
    }
}
