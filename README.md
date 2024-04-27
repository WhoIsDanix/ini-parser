# 📝 ini-parser

 ini-parser - C++ header-only library for working with .ini files.

## Usage

### Parsing

```cpp
#include <iostream>
#include <string>

#include "INIParser.h"

int main() {
    INI::Parser parser = INI::Parser::fromFile("userdata.ini");

    if (!parser.isOK()) {
        std::cout << "Failed to create parser from file\n";
        return -1;
    }

    INI::File file = parser.parse();

    if (!parser.isOK()) {
        std::cout << "Failed to parse .ini file\n";
        return -1;
    }

    std::string name, ip, password;

    file.getValue("User", "Name", name);
    file.getValue("User", "IPAddress", ip);
    file.getValue("User", "Password", password);

    std::cout << "Name = " << name << "\n";
    std::cout << "IPAddress = " << ip << "\n";
    std::cout << "Password = " << password << "\n";

    return 0;
}
```

### Generating

```cpp
#include <iostream>
#include "INIParser.h"

int main() {
    INI::File file;

    file.setValue("Graphics", "Resolution", "1920x1080");
    file.setValue("Graphics", "TextureQuality", "high");
    file.setValue("Graphics", "ShadowQuality", "high");
    file.setValue("Graphics", "MultisamplingAntiAliasing", "FXAA");

    file.saveToFile("graphics.ini");
    std::cout << "Saved data to graphics.ini\n";

    return 0;
}
```