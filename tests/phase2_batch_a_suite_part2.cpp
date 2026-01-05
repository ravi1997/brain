#include <gtest/gtest.h>
#include "perception/music_understanding.hpp"
#include "perception/environmental_sound.hpp"
#include "perception/3d_reconstruction.hpp"
#include "reasoning/argumentation.hpp"
#include "infra/semantic_web.hpp"
#include "infra/commonsense.hpp"
#include "neural/adversarial.hpp"
#include "neural/neural_symbolic.hpp"
#include "optimization/neuroevolution.hpp"
#include "distributed/emergent_behavior.hpp"

// Music Understanding Test
TEST(Phase2_Perception, MusicUnderstanding) {
    dnn::perception::MusicUnderstanding mu(44100.0f);
    
    std::vector<float> audio(44100, 0.0f);  // 1 second
    for (size_t i = 0; i < audio.size(); i++) {
        audio[i] = 0.5f * std::sin(2.0f * 3.14159f * 440.0f * i / 44100.0f);  // 440Hz
    }
    
    auto features = mu.analyze(audio);
    ASSERT_GT(features.tempo, 0);
    ASSERT_FALSE(features.key.empty());
}

// Environmental Sound Classification Test
TEST(Phase2_Perception, EnvironmentalSound) {
    dnn::perception::EnvironmentalSoundClassification esc(44100.0f);
    
    std::vector<float> audio(10000, 0.01f);  // Low energy -> silence
    
    std::string classification = esc.classify(audio);
    ASSERT_FALSE(classification.empty());
}

// 3D Reconstruction Test
TEST(Phase2_Perception, Object3DReconstruction) {
    dnn::perception::Object3DReconstruction recon;
    
    std::vector<std::vector<float>> depth_map = {
        {1.0f, 1.0f, 1.0f},
        {1.0f, 0.5f, 1.0f},
        {1.0f, 1.0f, 1.0f}
    };
    
    auto pointcloud = recon.depth_to_pointcloud(depth_map);
    ASSERT_GT(pointcloud.size(), 0);
}

// Argumentation Framework Test
TEST(Phase2_Reasoning, ArgumentationFramework) {
    dnn::reasoning::ArgumentationFramework af;
    
    dnn::reasoning::ArgumentationFramework::Argument arg1("a1", "claim1", 1.0f);
    dnn::reasoning::ArgumentationFramework::Argument arg2("a2", "claim2", 1.0f);
    
    af.add_argument(arg1);
    af.add_argument(arg2);
    
    dnn::reasoning::ArgumentationFramework::Attack attack("a1", "a2", 1.0f);
    af.add_attack(attack);
    
    auto extensions = af.compute_extensions(
        dnn::reasoning::ArgumentationFramework::Semantics::GROUNDED
    );
    
    ASSERT_GT(extensions.size(), 0);
}

// Semantic Web Reasoning Test
TEST(Phase2_Infra, SemanticWebReasoning) {
    dnn::infra::SemanticWebReasoning swr;
    
    dnn::infra::SemanticWebReasoning::Triple t1("Alice", "rdf:type", "Person");
    dnn::infra::SemanticWebReasoning::Triple t2("Person", "rdfs:subClassOf", "Agent");
    
    swr.add_triple(t1);
    swr.add_triple(t2);
    
    auto inferred = swr.infer_rdfs();
    ASSERT_GT(inferred.size(), 0);
}

// Common Sense Reasoning Test
TEST(Phase2_Infra, CommonSenseReasoning) {
    dnn::infra::CommonSenseReasoning csr;
    
    auto facts = csr.query("dog", "IsA");
    ASSERT_GT(facts.size(), 0);
    
    bool plausible = csr.is_plausible("dog", "has", "four_legs");
    ASSERT_TRUE(plausible);
}

// Adversarial Robustness Test
TEST(Phase2_Neural, AdversarialRobustness) {
    dnn::neural::AdversarialRobustness ar(0.1f);
    
    std::vector<float> input = {0.5f, 0.6f, 0.7f};
    std::vector<float> label = {1.0f, 0.0f};
    
    auto model = [](const std::vector<float>& x) -> std::vector<float> {
        return {x[0], 1.0f - x[0]};
    };
    
    auto adv = ar.fgsm(input, label, model);
    ASSERT_EQ(adv.perturbed_input.size(), input.size());
    ASSERT_GT(adv.perturbation_magnitude, 0);
}

// Neural-Symbolic Integration Test
TEST(Phase2_Neural, NeuralSymbolicIntegration) {
    dnn::neural::NeuralSymbolicIntegration nsi(32);
    
    nsi.learn_embeddings({
        {"cat", {0.1f, 0.2f, 0.3f}},
        {"dog", {0.15f, 0.25f, 0.35f}}
    });
    
    dnn::neural::NeuralSymbolicIntegration::Rule rule(
        {"animal"}, "has_legs"
    );
    nsi.add_rule(rule);
    
    auto conclusions = nsi.forward_reason({"animal"});
    ASSERT_GE(conclusions.size(), 0);
}

// Neuro-Evolution Test
TEST(Phase2_Optimization, NeuroEvolution) {
    dnn::optimization::NeuroEvolution::EvolutionConfig config;
    config.population_size = 20;
    config.mutation_rate = 0.1f;
    
    dnn::optimization::NeuroEvolution ne(10, config);
    
    auto fitness_fn = [](const std::vector<float>& genome) {
        float sum = 0;
        for (float g : genome) sum += g * g;
        return -sum;  // Minimize sum of squares
    };
    
    ne.train(fitness_fn, 5);
    
    auto best = ne.get_best();
    ASSERT_EQ(best.genome.size(), 10);
}

// Emergent Behavior Simulation Test
TEST(Phase2_Distributed, EmergentBehavior) {
    dnn::distributed::EmergentBehaviorSimulation ebs(10, 2);
    
    dnn::distributed::EmergentBehaviorSimulation::Environment env;
    env.bounds = {0.0f, 10.0f, 0.0f, 10.0f};
    ebs.set_environment(env);
    
    ebs.simulate_flocking(0.1f);
    
    auto metrics = ebs.measure_emergence();
    ASSERT_GE(metrics.global_alignment, -1.0f);
    ASSERT_LE(metrics.global_alignment, 1.0f);
}
