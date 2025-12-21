#include "brain.hpp"
#include <iostream>
#include <string>

int main() {
    safe_print("Initializing Brain Replica...");
    
    try {
        Brain brain;
        safe_print("Brain initialized. Synapses ready.");
        safe_print("Talk to the brain (type 'exit' to quit):");

        std::string input;
        while (true) {
            // We use standard cin, which might block. 
            // The background thread will print to logs, so we can have a clean prompt.
            
            std::cout << "\n> User: "; 
            if (!std::getline(std::cin, input)) break;
            if (input == "exit") break;
            if (input.empty()) continue;

            if (input == "sleep") {
                brain.sleep();
                safe_print("Brain: [Woke up refreshed]");
                continue;
            }

            if (input == "status") {
                safe_print(brain.get_status());
                continue;
            }

            if (input.rfind("save ", 0) == 0) {
                 std::string filename = input.substr(5);
                 brain.save(filename);
                 continue;
            }

            if (input.rfind("load ", 0) == 0) {
                 std::string filename = input.substr(5);
                 brain.load(filename);
                 continue;
            }

            if (input.rfind("grow ", 0) == 0) {
                 std::string size_str = input.substr(5);
                 long long target_bytes = std::stoll(size_str) * 1024 * 1024; // MB to Bytes
                 
                 safe_print("[Brain]: Growth target set to " + std::to_string(target_bytes) + " bytes.");
                 safe_print("[Brain]: Starting intense research session...");

                 // Load dictionary
                 std::vector<std::string> words;
                 std::ifstream dict_file("/usr/share/dict/words");
                 if (dict_file) {
                     std::string word;
                     while(dict_file >> word) {
                         // Filter small words or weird chars
                         if (word.length() > 3 && isalpha(word[0])) words.push_back(word);
                     }
                 }
                 if (words.empty()) words = {"C++", "Linux", "Docker", "AI", "Brain", "Science", "History"}; // Fallback

                 while (true) {
                     long long current = brain.get_knowledge_size();
                     if (current >= target_bytes) {
                         safe_print("[Brain]: Target size reached! Current knowledge: " + std::to_string(current) + " bytes.");
                         break;
                     }
                     
                     std::string topic = words[static_cast<size_t>(rand()) % words.size()];
                     std::string result = brain.research(topic);
                     
                     // Print simple status, full result might be spammy
                     safe_print("[Brain Growth]: " + std::to_string(current) + " / " + std::to_string(target_bytes) + " bytes. researched: " + topic);
                     
                     // Sleep a bit to avoid hammering API
                     std::this_thread::sleep_for(std::chrono::seconds(1));
                 }
                 continue;
            }

            if (input.rfind("deep ", 0) == 0) {
                 std::string topic = input.substr(5);
                 std::cout << "> Brain: Starting deep dive into " + topic + " (check logs)..." << std::endl;
                 brain.deep_research(topic);
                 continue;
            }

            if (input.rfind("research ", 0) == 0) {
                 std::string topic = input.substr(9);
                 std::cout << "> Brain: Queued research on " + topic << std::endl;
                 brain.research(topic);
                 continue;
            }

            std::string response = brain.interact(input);
            std::cout << "> Brain: " << response << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Critical Failure: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
