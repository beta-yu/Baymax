#ifndef _IVMT__HPP_
#define _IVMT__HPP_

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <memory>
#include <json/json.h>
#include "base/http.h"

using std::cout;
using std::endl;
using std::string;

class Rebot
{
private:
    string Url;
    string Api_Key;
    string User_Id;
    aip::HttpClient *Client;
private:
    bool IsCodeLegal(int code, string& err_msg)
    {
        std::unordered_map<int, string> err_reason = {
            {5000, "无解析结果"}, 
            {6000, "暂不支持该功能"}
        };
        auto it = err_reason.find(code);
        if(it != err_reason.end())
        {
            err_msg = it->second;
            return false;
        }
        else
            return true;
    }
public:
    Rebot(string id = "1")
    {
        this->Url = "http://openapi.tuling123.com/openapi/api/v2";
        this->Api_Key = "17d188a929314b4d8260b95f6483a470";
        this->User_Id = id;
        this->Client = new aip::HttpClient;
    }
    ~Rebot()
    {

    }
    string MessageToJson(string& msg)
    {
        string jsonstr; 
        Json::Value root;
        Json::StreamWriterBuilder wb;
        std::ostringstream os;

        root["reqType"] = 0;
        Json::Value item_1;
        Json::Value item_1_1;
        item_1_1["text"] = msg;
        item_1["inputText"] = item_1_1;
        root["perception"] = item_1;
        Json::Value item_2;
        item_2["apiKey"] = Api_Key;
        item_2["userId"] = User_Id;
        root["userInfo"] = item_2;

        std::unique_ptr<Json::StreamWriter> sw(wb.newStreamWriter());
        sw->write(root, &os);
        jsonstr = os.str();
        return jsonstr;
    }

    string JsonToMessage(string& jsonstr)
    {
        string ret;
        JSONCPP_STRING errs;
        Json::Value root;
        Json::CharReaderBuilder rb;
        std::unique_ptr<Json::CharReader> const rj(rb.newCharReader());
        bool res = rj->parse(jsonstr.data(), jsonstr.data()+jsonstr.size(), &root, &errs);
        if(!res || !errs.empty())
        {
            cout << "parse error: " << errs << endl;
            return "";
        }
        int code = root["intent"]["code"].asInt();
        string err_msg;
        if(!IsCodeLegal(code, err_msg))
        {
            cout << "Response Code Error: " << err_msg << endl;
            return "";
        }
        ret = root["results"][0]["values"]["text"].asString();
        return ret;
    }

    string RequestTuL(string& jsonstr)
    {
        string response;
        int status_code = Client->post(Url, nullptr, jsonstr, nullptr, &response);
        if (status_code != CURLcode::CURLE_OK) {
            cout << "post error" << endl;
            return "";
        }
        return response;
    }

    string Talk(string& msg)
    {
        string request;
        string response;
        string ret;
        request = MessageToJson(msg);
        response = RequestTuL(request);
        ret = JsonToMessage(response);
        return ret;
    }

};

class Ivmt
{
    
};

#endif //_IVMT__HPP_