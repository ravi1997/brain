#include <gtest/gtest.h>
#include "optimization/nas.hpp"
#include "neural/hypernetwork.hpp"
#include "reasoning/mcts.hpp"
#include "reasoning/csp_solver.hpp"
#include "perception/optical_flow.hpp"

// Neural Architecture Search Test
TEST(Phase1_Optimization, NeuralArchitectureSearch) {
    dnn::optimization::NeuralArchitectureSearch nas(10, 5);  // Small population
    
    // Simple fitness function: prefer fewer layers
    auto fitness = [](const dnn::optimization::NeuralArchitectureSearch::Architecture& arch) {
        return 10.0f / (arch.num_layers + 1.0f);
    };
    
    auto best_arch = nas.search(fitness, 5);  // 5 generations
    ASSERT_GT(best_arch.fitness, 0.0f);
    ASSERT_GT(best_arch.num_layers, 0);
}

// Hypernetwork Test
TEST(Phase1_Neural, Hypernetwork) {
    dnn::neural::Hypernetwork hyper(64, 100);
    
    std::vector<float> task_embedding(64, 0.5f);
    auto weights = hyper.generate_weights(task_embedding);
    
    ASSERT_EQ(weights.size(), 100);
}

// Monte Carlo Tree Search Test
TEST(Phase1_Reasoning, MCTS) {
    dnn::reasoning::MonteCarloTreeSearch mcts(100);  // 100 simulations
    
    // Simple game: state is a number, actions increment it
    auto get_actions = [](const dnn::reasoning::MonteCarloTreeSearch::State& s) {
        if (s.terminal) return std::vector<int>{};
        return std::vector<int>{1, 2, 3};
    };
    
    auto apply_action = [](const dnn::reasoning::MonteCarloTreeSearch::State& s, int action) {
        dnn::reasoning::MonteCarloTreeSearch::State new_state;
        new_state.features = {s.features.empty() ? 0.0f : s.features[0] + action};
        new_state.terminal = (new_state.features[0] >= 10.0f);
        return new_state;
    };
    
    auto get_reward = [](const dnn::reasoning::MonteCarloTreeSearch::State& s) {
        return s.features.empty() ? 0.0f : s.features[0];
    };
    
    dnn::reasoning::MonteCarloTreeSearch::State initial;
    initial.features = {0.0f};
    
    int action = mcts.search(initial, get_actions, apply_action, get_reward);
    ASSERT_GE(action, 1);
    ASSERT_LE(action, 3);
}

// CSP Solver Test  
TEST(Phase1_Reasoning, CSPSolver) {
    dnn::reasoning::CSPSolver csp;
    
    // Simple map coloring: 3 regions, 3 colors
    csp.add_variable("A", {1, 2, 3});
    csp.add_variable("B", {1, 2, 3});
    csp.add_variable("C", {1, 2, 3});
    
    // Constraint: adjacent regions must have different colors
    auto not_equal = [](int a, int b) { return a != b; };
    csp.add_constraint("A", "B", not_equal);
    csp.add_constraint("B", "C", not_equal);
    csp.add_constraint("A", "C", not_equal);
    
    bool solved = csp.solve();
    ASSERT_TRUE(solved);
    
    auto solution = csp.get_solution();
    ASSERT_EQ(solution.size(), 3);
}

// Optical Flow Test
TEST(Phase1_Perception, OpticalFlow) {
    dnn::perception::OpticalFlow flow(32, 32, 5);
    
    std::vector<float> frame1(32 * 32, 0.5f);
    std::vector<float> frame2(32 * 32, 0.6f);  // Slight shift
    
    auto flow_vectors = flow.compute_flow(frame1, frame2);
    ASSERT_EQ(flow_vectors.size(), 32 * 32);
}
