#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <json/json.h>

int main()
{
    Json::Value root;
    Json::StreamWriterBuilder wb;
    std::ostringstream os;

    root["name"] = "qiyu";
    root["age"] = 18;
    root["Lang"] = "Chinese";
    std::unique_ptr<Json::StreamWriter> sw(wb.newStreamWriter());
    sw->write(root, &os);
    std::string str = os.str();
    std::cout << str << std::endl;
    return 0;
}