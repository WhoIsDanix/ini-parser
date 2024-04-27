#ifndef INIPARSER_INIPARSER_H
#define INIPARSER_INIPARSER_H

#include <iostream>
#include <string>
#include <fstream>
#include <unordered_map>

namespace INI {
    using Values_T = std::unordered_map<std::string, std::string>;
    using Sections_T = std::unordered_map<std::string, Values_T>;

    class File {
    private:
        Sections_T sections;

    public:
        File() : File(Sections_T{}) {

        }

        File(Sections_T sections) : sections(sections) {

        }

        Sections_T getSections() {
            return sections;
        }

        bool getValue(std::string section, std::string key, std::string& out) {
            auto found = sections[section].find(key);

            if (found == sections[section].end()) {
                std::cout << "[ERROR] Variable \"" << key << "\"";
                if (section.size() > 0) {
                    std::cout << " in section \"" << section << "\"";
                }
                std::cout << " does not exist\n";

                return false;
            }

            out = found->second;
            return true;
        }

        bool getValueInt(std::string section, std::string key, int& out) {
            std::string value;

            if (!getValue(section, key, value)) {
                return false;
            }

            try {
                out = std::stoi(value.c_str());
            }
            catch (std::exception const& exc) {
                std::cout << "[ERROR] Failed to parse integer variable \"" << key << "\"";
                if (section.size() > 0) {
                    std::cout << " in section \"" << section << "\"";
                }
                std::cout << "\n";

                return false;
            }

            return true;
        }


        void setValue(std::string section, std::string key, std::string value) {
            sections[section][key] = value;
        }

        void saveToFile(std::string filename) {
            std::ofstream stream(filename);

            for (const auto sectionPair : sections) {
                if (sectionPair.first != "") {
                    stream << "[" << sectionPair.first << "]\n";
                }

                for (const auto valuePair : sectionPair.second) {
                    stream << valuePair.first << "=" << valuePair.second << "\n";
                }

                if (sectionPair.second.size() > 0) {
                    stream << "\n";
                }
            }

            stream.close();
        }
    };

    class Parser {
    private:
        bool isOk = true;
        std::string content;
        int currIdx = -1;
        char currChar = '\0';
        int currLine = 1;

        void advance() {
            ++currIdx;
            if (currIdx < content.size()) {
                currChar = content[currIdx];
            }
        }

        bool isEOF() {
            return currIdx >= content.size();
        }

        void skipComment() {
            while (!isEOF() && currChar != '\n') {
                advance();
            }
        }

        std::string parseSectionName() {
            advance();

            int begin = currIdx;
            int len = 0;

            while (!isEOF() && (currChar != ']' && currChar != '\n')) {
                ++len;
                advance();
            }

            if (currChar != ']') {
                std::cout << "[ERROR] At line " << currLine << ": expected ] for section end\n";
                isOk = false;
                return "";
            }

            return content.substr(begin, len);
        }
        
        std::pair<std::string, std::string> parseVariable() {
            int begin = currIdx;
            int len = 0;

            while (!isEOF() && (currChar != '=' && currChar != '\n')) {
                ++len;
                advance();
            }

            if (currChar != '=') {
                std::cout << "[ERROR] At line " << currLine << ": expected = for variable assignment\n";
                isOk = false;
                return std::make_pair("", "");
            }

            std::string name = content.substr(begin, len);
            
            advance();
            begin = currIdx;
            len = 0;

            while (!isEOF() && currChar != '\n') {
                ++len;
                advance();
            }

            std::string value = content.substr(begin, len);
            
            // Maybe there's a better way to trim a string?
            name.erase(name.begin(), std::find_if(name.begin(), name.end(), [](unsigned char c) { return !std::isspace(c); }));
            name.erase(std::find_if(name.rbegin(), name.rend(), [](unsigned char c) { return !std::isspace(c); }).base(), name.end());

            value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char c) { return !std::isspace(c); }));
            value.erase(std::find_if(value.rbegin(), value.rend(), [](unsigned char c) { return !std::isspace(c); }).base(), value.end());

            return std::make_pair(name, value);
        }

    public:
        Parser(std::string content) : content(content) {
            advance();
        }

        static Parser fromFile(std::string filename) {
            constexpr std::size_t readSize = 4096;

            std::ifstream stream = std::ifstream(filename);
            stream.exceptions(std::ios_base::badbit);

            if (!stream.is_open()) {
                std::cout << "[ERROR] Failed to open file \"" << filename << "\"\n";
                Parser parser("");
                parser.isOk = false;
                return parser;
            }

            std::string out;
            std::string buf(readSize, '\0');

            while (stream.read(&buf[0], readSize)) {
                out.append(buf, 0, stream.gcount());
            }

            out.append(buf, 0, stream.gcount());
            stream.close();

            return Parser(out);
        }

        bool isOK() {
            return isOk;
        }

        File parse() {
            File file;
            std::string currentSection = "";

            while (!isEOF()) {
                switch (currChar) {
                case ';':
                case '#':
                    skipComment();
                    continue;
                case '\n':
                case ' ':
                case '\t':
                    if (currChar == '\n') {
                        ++currLine;
                    }

                    advance();
                    continue;
                case '[':
                    currentSection = parseSectionName();
                    advance();
                    continue;
                default:
                    if (std::isalpha(currChar)) {
                        std::pair<std::string, std::string> variable = parseVariable();
                        file.setValue(currentSection, variable.first, variable.second);
                        continue;
                    }

                    std::cout << "[ERROR] At line " << currLine << ": unknown character " << currChar << "\n";
                    isOk = false;

                    break;
                }
            }

            return file;
        }
    };
}

#endif