#pragma once
#include "tool_interface.hpp"
#include <array>
#include <memory>
#include <stdexcept>
#include <iostream>

namespace dnn {

    class ShellTool : public ToolInterface {
    public:
        std::string get_name() const override { return "SHELL"; }
        std::string get_description() const override { return "Executes bash commands. Use with caution."; }

        std::string execute(const std::string& cmd) override {
            // Safety: Block dangerous commands (rm -rf /, etc) 
            // Minimal filter for now
            if (cmd.find("rm -rf /") != std::string::npos) return "ERROR: Unsafe command blocked.";

            std::array<char, 128> buffer;
            std::string result;
            // Redirect stderr to stdout
            std::string full_cmd = cmd + " 2>&1";
            
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(full_cmd.c_str(), "r"), pclose);
            if (!pipe) {
                return "ERROR: popen() failed!";
            }
            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                result += buffer.data();
            }
            return result;
        }
    };

} // namespace dnn
