include world.generator;

class ScenarioLoader {
    fn ScenarioLoader(){}

    fn setup_default(gs){
        gen = world.generator.WorldGenerator();
        gen.generate(gs);
        gs.world_map.overview();
    }
}
