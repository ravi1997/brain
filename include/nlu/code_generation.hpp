#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace dnn::nlu {

// Code generation from natural language
class CodeGeneration {
public:
    CodeGeneration() {
        // Initialize code templates
        templates_["function"] = "def {name}({params}):\n    {body}\n    return {return_val}";
        templates_["class"] = "class {name}:\n    def __init__(self{params}):\n        {init_body}";
        templates_["loop"] = "for {var} in {iterable}:\n    {body}";
        templates_["conditional"] = "if {condition}:\n    {body}";
    }
    
    // Generate code from natural language description
    std::string generate(const std::string& description) {
        std::string lower_desc = to_lower(description);
        
        if (lower_desc.find("function") != std::string::npos ||
            lower_desc.find("def") != std::string::npos) {
            return generate_function(description);
        } else if (lower_desc.find("class") != std::string::npos) {
            return generate_class(description);
        } else if (lower_desc.find("loop") != std::string::npos ||
                  lower_desc.find("iterate") != std::string::npos) {
            return generate_loop(description);
        } else if (lower_desc.find("if") != std::string::npos ||
                  lower_desc.find("condition") != std::string::npos) {
            return generate_conditional(description);
        }
        
        return "# TODO: Implement " + description;
    }
    
private:
    std::unordered_map<std::string, std::string> templates_;
    
    std::string generate_function(const std::string& desc) {
        std::string code = templates_["function"];
        replace_placeholder(code, "{name}", "my_function");
        replace_placeholder(code, "{params}", "param1, param2");
        replace_placeholder(code, "{body}", "# Function body");
        replace_placeholder(code, "{return_val}", "result");
        return code;
    }
    
    std::string generate_class(const std::string& desc) {
        std::string code = templates_["class"];
        replace_placeholder(code, "{name}", "MyClass");
        replace_placeholder(code, "{params}", ", param1, param2");
        replace_placeholder(code, "{init_body}", "pass");
        return code;
    }
    
    std::string generate_loop(const std::string& desc) {
        std::string code = templates_["loop"];
        replace_placeholder(code, "{var}", "item");
        replace_placeholder(code, "{iterable}", "items");
        replace_placeholder(code, "{body}", "print(item)");
        return code;
    }
    
    std::string generate_conditional(const std::string& desc) {
        std::string code = templates_["conditional"];
        replace_placeholder(code, "{condition}", "condition");
        replace_placeholder(code, "{body}", "pass");
        return code;
    }
    
    void replace_placeholder(std::string& str, const std::string& placeholder, 
                            const std::string& value) {
        size_t pos = str.find(placeholder);
        if (pos != std::string::npos) {
            str.replace(pos, placeholder.length(), value);
        }
    }
    
    std::string to_lower(std::string s) {
        for (char& c : s) c = std::tolower(c);
        return s;
    }
};

} // namespace dnn::nlu
