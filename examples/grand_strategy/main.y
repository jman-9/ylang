include game.state;
include sim.timeline;
include game.scenario;
include game.ui;

fn main(){
    println("=== Grand Strategy ===");

    gs = game.state.GameState();
    loader = game.scenario.ScenarioLoader();
    loader.setup_default(gs);

    tl = sim.timeline.Timeline(gs);

    years = 60;
    for(i=0;i<years;i+=1){
        println("");
        println("===== YEAR {gs.year} =====");
        tl.step_year();
    }

    println("");
    println("=== Simulation finished at year {gs.year} ===");
    game.ui.ui().print_final_report(gs);
}
