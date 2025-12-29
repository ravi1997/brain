#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <cmath>

namespace dnn::neural {

// Neural Module Networks for compositional reasoning
class NeuralModuleNetwork {
public:
    // Base module interface
    class Module {
    public:
        virtual ~Module() = default;
        virtual std::vector<float> execute(const std::vector<std::vector<float>>& inputs) = 0;
        virtual std::string get_name() const = 0;
    };
    
    // Find module - locate objects
    class FindModule : public Module {
    public:
        FindModule(const std::string& object_type) : object_type_(object_type) {}
        
        std::vector<float> execute(const std::vector<std::vector<float>>& inputs) override {
            // Returns attention map highlighting the object
            std::vector<float> attention(100, 0.0f);  // 10x10 grid
            
            // Simulate finding object based on type
            int hash = 0;
            for (char c : object_type_) hash += c;
            int center = (hash % 80) + 10;
            
            // Create blob around center
            for (int i = -2; i <= 2; i++) {
                for (int j = -2; j <= 2; j++) {
                    int idx = center + i * 10 + j;
                    if (idx >= 0 && idx < 100) {
                        attention[idx] = 1.0f / (1.0f + std::abs(i) + std::abs(j));
                    }
                }
            }
            
            return attention;
        }
        
        std::string get_name() const override { return "find[" + object_type_ + "]"; }
        
    private:
        std::string object_type_;
    };
    
    // Relate module - find spatial relationships
    class RelateModule : public Module {
    public:
        RelateModule(const std::string& relation) : relation_(relation) {}
        
        std::vector<float> execute(const std::vector<std::vector<float>>& inputs) override {
            if (inputs.empty()) return std::vector<float>(100, 0.0f);
            
            const auto& attention = inputs[0];
            std::vector<float> related(100, 0.0f);
            
            // Find peak of input attention
            int peak_idx = 0;
            float peak_val = 0.0f;
            for (size_t i = 0; i < attention.size(); i++) {
                if (attention[i] > peak_val) {
                    peak_val = attention[i];
                    peak_idx = i;
                }
            }
            
            int row = peak_idx / 10;
            int col = peak_idx % 10;
            
            // Apply spatial relationship
            if (relation_ == "left") {
                for (int c = 0; c < col; c++) {
                    related[row * 10 + c] = 1.0f - (col - c) * 0.1f;
                }
            } else if (relation_ == "right") {
                for (int c = col + 1; c < 10; c++) {
                    related[row * 10 + c] = 1.0f - (c - col) * 0.1f;
                }
            } else if (relation_ == "above") {
                for (int r = 0; r < row; r++) {
                    related[r * 10 + col] = 1.0f - (row - r) * 0.1f;
                }
            } else if (relation_ == "below") {
                for (int r = row + 1; r < 10; r++) {
                    related[r * 10 + col] = 1.0f - (r - row) * 0.1f;
                }
            }
            
            return related;
        }
        
        std::string get_name() const override { return "relate[" + relation_ + "]"; }
        
    private:
        std::string relation_;
    };
    
    // And module - intersection of attention maps
    class AndModule : public Module {
    public:
        std::vector<float> execute(const std::vector<std::vector<float>>& inputs) override {
            if (inputs.empty()) return std::vector<float>(100, 0.0f);
            
            std::vector<float> result = inputs[0];
            
            for (size_t i = 1; i < inputs.size(); i++) {
                for (size_t j = 0; j < result.size() && j < inputs[i].size(); j++) {
                    result[j] = std::min(result[j], inputs[i][j]);
                }
            }
            
            return result;
        }
        
        std::string get_name() const override { return "and"; }
    };
    
    // Or module - union of attention maps
    class OrModule : public Module {
    public:
        std::vector<float> execute(const std::vector<std::vector<float>>& inputs) override {
            if (inputs.empty()) return std::vector<float>(100, 0.0f);
            
            std::vector<float> result = inputs[0];
            
            for (size_t i = 1; i < inputs.size(); i++) {
                for (size_t j = 0; j < result.size() && j < inputs[i].size(); j++) {
                    result[j] = std::max(result[j], inputs[i][j]);
                }
            }
            
            return result;
        }
        
        std::string get_name() const override { return "or"; }
    };
    
    // Answer module - classify based on attention
    class AnswerModule : public Module {
    public:
        AnswerModule(const std::string& answer_type) : answer_type_(answer_type) {}
        
        std::vector<float> execute(const std::vector<std::vector<float>>& inputs) override {
            if (inputs.empty()) return {0.0f};
            
            const auto& attention = inputs[0];
            
            if (answer_type_ == "exists") {
                // Check if attention is focused anywhere
                float max_attention = *std::max_element(attention.begin(), attention.end());
                return {max_attention > 0.5f ? 1.0f : 0.0f};
            } else if (answer_type_ == "count") {
                // Count number of peaks
                int count = 0;
                for (float val : attention) {
                    if (val > 0.7f) count++;
                }
                return {static_cast<float>(count)};
            }
            
            return {0.0f};
        }
        
        std::string get_name() const override { return "answer[" + answer_type_ + "]"; }
        
    private:
        std::string answer_type_;
    };
    
    // Module network execution graph
    struct ExecutionNode {
        std::shared_ptr<Module> module;
        std::vector<int> input_nodes;  // Indices of nodes that provide input
        std::vector<float> output;
        
        ExecutionNode(std::shared_ptr<Module> m) : module(m) {}
    };
    
    NeuralModuleNetwork() {}
    
    // Add module to network
    int add_module(std::shared_ptr<Module> module, const std::vector<int>& inputs = {}) {
        ExecutionNode node(module);
        node.input_nodes = inputs;
        network_.push_back(node);
        return network_.size() - 1;
    }
    
    // Execute the network
    std::vector<float> execute() {
        // Topological execution
        for (auto& node : network_) {
            std::vector<std::vector<float>> inputs;
            
            for (int input_idx : node.input_nodes) {
                if (input_idx >= 0 && input_idx < static_cast<int>(network_.size())) {
                    inputs.push_back(network_[input_idx].output);
                }
            }
            
            node.output = node.module->execute(inputs);
        }
        
        // Return output of last module
        return network_.empty() ? std::vector<float>{} : network_.back().output;
    }
    
    // Clear network
    void clear() {
        network_.clear();
    }
    
    // Get network description
    std::string describe() const {
        std::string desc;
        for (size_t i = 0; i < network_.size(); i++) {
            desc += std::to_string(i) + ": " + network_[i].module->get_name() + "\n";
        }
        return desc;
    }
    
private:
    std::vector<ExecutionNode> network_;
};

} // namespace dnn::neural
