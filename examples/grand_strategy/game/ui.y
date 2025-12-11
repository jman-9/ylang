class ui {
    fn print_final_report(gs){
        println("=== FINAL REPORT ===");
        println("Year: {gs.year}");
        println("Civs: {gs.civs.len()}");
        for(i=0;i<gs.civs.len();i+=1){
            gs.civs[i].summary();
        }
        println("====================");
    }
}
