#include <iostream>
#include <fstream>
#include <sstream>
#include "config.h"

using namespace std;

bool Config::load_config(const string& filePath) {
    ifstream config_file(filePath);
    if( !config_file.is_open() ) {
        cout << "Error: could not open config file: " << filePath << endl;
        return false;
    } 

    string line;
    while( getline(config_file, line) ) {
        if( line.empty() || line[0] == '#' ) continue;
        istringstream stream(line);
        string key, value;
        stream >> key >> value; // need to remove after testing
        config_map[key] = value;
    }
    return true;
}

string Config::get_config_value(const string& key) {
    auto it = config_map.find(key);
    if( it != config_map.end() ) {
        // cout << key << ": " << it->second << endl;
        return it->second;
    } else {
        cout << "Error: config key not found: " << key << endl;
        return "";
    }
}