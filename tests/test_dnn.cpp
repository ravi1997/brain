#include <gtest/gtest.h>
#include "dnn.hpp"

TEST(DNNTest, PlasticLayerConstructor) {
    std::mt19937_64 rng(42);
    dnn::PlasticLayer layer(10, 5, rng);
    EXPECT_EQ(layer.in_size, 10);
    EXPECT_EQ(layer.out_size, 5);
    EXPECT_EQ(layer.weights.size(), 50);
}

TEST(DNNTest, NeuralNetworkForward) {
    dnn::NeuralNetwork net({2, 4, 1});
    std::vector<double> input = {0.5, -0.5};
    auto output = net.predict(input);
    EXPECT_EQ(output.size(), 1);
}

TEST(DNNTest, Plasticity) {
    dnn::NeuralNetwork net({2, 2});
    net.set_plasticity(true);
    
    // Check if plasticity runs without crashing
    std::vector<std::vector<double>> X = {{1.0, 0.0}};
    std::vector<std::vector<double>> Y = {{0.0, 1.0}};
    
    net.train(X, Y, 1, 1, 0.1);
    SUCCEED();
}
