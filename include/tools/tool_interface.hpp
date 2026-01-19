#pragma once
#include <string>
#include <vector>

namespace dnn {

    class ToolInterface {
    public:
        virtual ~ToolInterface() = default;
        
        virtual std::string get_name() const = 0;
        virtual std::string get_description() const = 0;
        
        // Execute the tool with a string argument
        // Returns the output/result of the tool execution
        virtual std::string execute(const std::string& args) = 0;
    };

} // namespace dnn
