#pragma once
#include "tool_interface.hpp"
#include <fstream>
#include <sstream>

namespace dnn {

    class FileTool : public ToolInterface {
    public:
        std::string get_name() const override { return "FILE_IO"; }
        std::string get_description() const override { return "Reads/Writes files. Syntax: READ <path> or WRITE <path> <content>"; }

        std::string execute(const std::string& args) override {
            std::stringstream ss(args);
            std::string op;
            ss >> op;

            if (op == "READ") {
                std::string path;
                std::getline(ss >> std::ws, path);
                std::ifstream f(path);
                if (!f) return "ERROR: File not found or unreadable: " + path;
                std::stringstream buffer;
                buffer << f.rdbuf();
                return buffer.str();
            }
            else if (op == "WRITE") {
                 std::string path;
                 ss >> path; // Space delimited path first? Or strict syntax?
                 // Let's assume path is second token
                 std::string content;
                 std::getline(ss >> std::ws, content);
                 
                 std::ofstream f(path);
                 if (!f) return "ERROR: Cannot open file for writing: " + path;
                 f << content;
                 return "SUCCESS: Wrote to " + path;
            }

            return "ERROR: Unknown operation. Use READ or WRITE.";
        }
    };

} // namespace dnn
