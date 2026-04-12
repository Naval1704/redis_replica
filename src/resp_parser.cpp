#include <iostream>
#include <vector>
#include <string>
#include "resp_parser.h"
using namespace std;

string RESPParser::serialize_simple_string(const string& str){
    return "+" + str + "\r\n";
}

string RESPParser::serialize_error(const string& str) {
    return "-ERR " + str + "\r\n";
}

string RESPParser::serialize_integer(int num) {
    return ":" + to_string(num) + "\r\n";
}

string RESPParser::serialize_bulk_string(const string& str) {
    return "$" + to_string(str.length()) + "\r\n" + str + "\r\n";
}

string RESPParser::serialize_null() {
    return "$-1\r\n";
}

size_t find_pos(string& command) {
    size_t pos = 0;
    pos = command.find("\r\n", pos);
    return pos;
}

vector<string> RESPParser::parse(string command) {
    vector<string> result ;
    if( command.empty() ) return result;
    if( command[0] == '*' ) {
        size_t pos = find_pos(command);
        int num_elements = stoi(command.substr(1, pos-1));
        command = command.substr(pos+2);

        for( int i=0; i<num_elements; i++ ){
            if( command.empty() ) break;
            if( command[0] != '$' ) break;

            pos = find_pos(command);
            int str_length = stoi(command.substr(1, pos-1));
            command = command.substr(pos+2);

            if( command.length() < str_length + 2 ) break;
            
            string element = command.substr(0, str_length);
            result.push_back(element);

            command = command.substr(str_length + 2);
        } 
    }
    return result;
}