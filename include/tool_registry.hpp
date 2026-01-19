#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include "tools/shell_tool.hpp"
#include "tools/file_tool.hpp"

namespace dnn {

    struct Tool {
        std::string name;
        std::string description;
        // Function takes arguments string, returns result string
        std::function<std::string(std::string)> execute; 
    };

    class ToolRegistry {
    public:
        ToolRegistry() {
            register_defaults();
        }

        void register_tool(const Tool& t) {
            tools_[t.name] = t;
        }

        std::vector<std::string> valailable_tools() const {
            std::vector<std::string> names;
            for(const auto& kv : tools_) names.push_back(kv.first);
            return names;
        }

        std::string use_tool(const std::string& name, const std::string& args) {
            if (tools_.find(name) != tools_.end()) {
                return tools_[name].execute(args);
            }
            return "ERROR: Tool not found";
        }

    private:
        std::map<std::string, Tool> tools_;

        void register_defaults() {
             tools_["CALCULATOR"] = {
                 "CALCULATOR", "Performs basic math", 
                 [](std::string args) {
                     if (args.find("+") != std::string::npos) return "RESULT: [Math Result Stub]";
                     return "RESULT: 42";
                 }
             };
             
             tools_["WEB_SEARCH"] = {
                 "WEB_SEARCH", "Simulates searching the web",
                 [](std::string query) {
                     return "SEARCH_RESULT: Found 5 articles about " + query;
                 }
             };

             // Register Real Tools
             auto shell = std::make_shared<ShellTool>();
             tools_[shell->get_name()] = {
                 shell->get_name(), shell->get_description(),
                 [shell](std::string args) { return shell->execute(args); }
             };

             auto file = std::make_shared<FileTool>();
             tools_[file->get_name()] = {
                 file->get_name(), file->get_description(),
                 [file](std::string args) { return file->execute(args); }
             };
        }
    };

} // namespace dnn
