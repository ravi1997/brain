#pragma once

#include <string>
#include <cstdlib>

namespace dnn::infra {

/**
 * @brief Simple utility to fetch environment variables with defaults.
 */
class Config {
public:
    static std::string get(const std::string& key, const std::string& defaultValue = "") {
        char* val = std::getenv(key.c_str());
        return val ? std::string(val) : defaultValue;
    }

    static int get_int(const std::string& key, int defaultValue = 0) {
        char* val = std::getenv(key.c_str());
        return val ? std::stoi(val) : defaultValue;
    }

    static double get_double(const std::string& key, double defaultValue = 0.0) {
        char* val = std::getenv(key.c_str());
        return val ? std::stod(val) : defaultValue;
    }

    static std::string get_db_conn_str() {
        std::string host = get("DB_HOST", "postgres");
        std::string port = get("DB_PORT", "5432");
        std::string dbname = get("DB_NAME", "brain_db");
        std::string user = get("DB_USER", "brain_user");
        std::string pass = get("DB_PASS", "brain_password");

        return "host=" + host + " port=" + port + " dbname=" + dbname + " user=" + user + " password=" + pass;
    }
};

} // namespace dnn::infra
