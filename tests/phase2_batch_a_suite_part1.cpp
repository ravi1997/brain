#include <gtest/gtest.h>
#include "../include/neural/program_synthesis.hpp"
#include "../include/neural/diff_reasoning.hpp"
#include "../include/reasoning/causal_inference.hpp"
#include "../include/reasoning/counterfactual.hpp"
#include "../include/reasoning/theorem_proving.hpp"
#include "../include/reasoning/abductive.hpp"
#include "../include/reasoning/default_logic.hpp"
#include "../include/reasoning/nonmonotonic.hpp"
#include "../include/reasoning/unification.hpp"
#include "../include/reasoning/explanation.hpp"

// Neural Program Synthesis Test
TEST(Phase2_Neural, ProgramSynthesis) {
    dnn::neural::NeuralProgramSynthesis nps(10);
    
    dnn::neural::NeuralProgramSynthesis::Specification spec;
    spec.examples = {
        {{1.0f, 2.0f}, 3.0f},
        {{2.0f, 3.0f}, 5.0f},
        {{3.0f, 4.0f}, 7.0f}
    };
    
    auto program = nps.synthesize(spec);
    ASSERT_GT(program.instructions.size(), 0);
    
    float result = nps.execute(program, {1.0f, 2.0f});
    ASSERT_GT(result, 0);
}

// Differentiable Reasoning Test
TEST(Phase2_Neural, DifferentiableReasoning) {
    dnn::neural::DifferentiableReasoning dr;
    
    std::vector<float> inputs = {0.8f, 0.9f};
    
    float and_result = dr.fuzzy_and(inputs);
    ASSERT_GT(and_result, 0);
    ASSERT_LT(and_result, 1);
    
    float or_result = dr.fuzzy_or(inputs);
    ASSERT_GT(or_result, and_result);
}

// Causal Inference Test
TEST(Phase2_Reasoning, CausalInference) {
    dnn::reasoning::CausalInference ci;
    
    ci.add_variable("X", {}, [](const auto& vars) { return 0.5f; });
    ci.add_variable("Y", {"X"}, [](const auto& vars) {
        return vars.count("X") ? vars.at("X") * 2.0f : 0.0f;
    });
    
    dnn::reasoning::CausalInference::Intervention intervention("X", 1.0f);
    float y = ci.intervene(intervention, "Y");
    
    ASSERT_NEAR(y, 2.0f, 0.1f);
}

// Counterfactual Reasoning Test
TEST(Phase2_Reasoning, CounterfactualReasoning) {
    dnn::reasoning::CounterfactualReasoning cr;
    
    dnn::reasoning::CounterfactualReasoning::FactualWorld world;
    world.variables["X"] = 1.0f;
    world.variables["Y"] = 2.0f;
    world.mechanisms["Y"] = [](const auto& vars) {
        return vars.count("X") ? vars.at("X") * 2.0f : 0.0f;
    };
    
    dnn::reasoning::CounterfactualReasoning::CounterfactualQuery query;
    query.variable = "X";
    query.counterfactual_value = 0.5f;
    query.target = "Y";
    
    float counterfactual_y = cr.compute_counterfactual(query, world);
    ASSERT_NEAR(counterfactual_y, 1.0f, 0.1f);
}

// Automated Theorem Proving Test
TEST(Phase2_Reasoning, TheoremProving) {
    dnn::reasoning::AutomatedTheoremProving atp;
    
    using Literal = dnn::reasoning::AutomatedTheoremProving::Literal;
    using Clause = dnn::reasoning::AutomatedTheoremProving::Clause;
    
    Clause axiom1({Literal("P", {"x"}, false)});
    atp.add_axiom(axiom1);
    
    Clause goal({Literal("P", {"x"}, false)});
    bool proven = atp.prove(goal);
    
    ASSERT_TRUE(proven);
}

// Abductive Reasoning Test
TEST(Phase2_Reasoning, AbductiveReasoning) {
    dnn::reasoning::AbductiveReasoning ar;
    
    dnn::reasoning::AbductiveReasoning::Rule rule;
    rule.conditions = {"rain"};
    rule.conclusion = "wet_grass";
    rule.confidence = 0.9f;
    ar.add_rule(rule);
    
    std::vector<dnn::reasoning::AbductiveReasoning::Observation> obs = {
        {"wet_grass", 1.0f}
    };
    
    auto hypothesis = ar.abduce(obs);
    ASSERT_FALSE(hypothesis.explanation.empty());
}

// Default Logic Test
TEST(Phase2_Reasoning, DefaultLogic) {
    dnn::reasoning::DefaultLogic dl;
    
    dl.add_fact("bird");
    
    dnn::reasoning::DefaultLogic::Default def(
        {"bird"}, {"NOT_penguin"}, "can_fly"
    );
    dl.add_default(def);
    
    ASSERT_TRUE(dl.entails("can_fly"));
}

// Non-Monotonic Reasoning Test
TEST(Phase2_Reasoning, NonMonotonicReasoning) {
    dnn::reasoning::NonMonotonicReasoning nmr;
    
    dnn::reasoning::NonMonotonicReasoning::Belief belief("bird_flies", 0.9f);
    nmr.add_belief(belief);
    
    float strength = nmr.query("bird_flies");
    ASSERT_GT(strength, 0.5f);
    
    dnn::reasoning::NonMonotonicReasoning::Defeater defeater("penguin", "bird_flies", 0.8f);
    nmr.add_defeater(defeater);
    nmr.add_belief({"penguin", 1.0f});
    
    float new_strength = nmr.query("bird_flies");
    ASSERT_LT(new_strength, strength);
}

// Logical Unification Test
TEST(Phase2_Reasoning, LogicalUnification) {
    dnn::reasoning::LogicalUnification lu;
    
    auto x = lu.var("X");
    auto a = lu.constant("a");
    
    dnn::reasoning::LogicalUnification::Substitution subst;
    bool unified = lu.unify(x, a, subst);
    
    ASSERT_TRUE(unified);
    ASSERT_EQ(subst.size(), 1);
}

// Explanation Generation Test
TEST(Phase2_Reasoning, ExplanationGeneration) {
    dnn::reasoning::ExplanationGeneration eg;
    
    dnn::reasoning::ExplanationGeneration::Decision decision;
    decision.prediction = "positive";
    decision.confidence = 0.9f;
    decision.input_features = {
        {"feature1", 0.8f, 0.7f},
        {"feature2", 0.3f, 0.2f}
    };
    
    auto explanation = eg.explain(decision);
    ASSERT_FALSE(explanation.natural_language.empty());
    ASSERT_GT(explanation.key_features.size(), 0);
}
