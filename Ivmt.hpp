#ifndef _IVMT__HPP_
#define _IVMT__HPP_

#include <iostream>
#include <cstdio>
#include <sstream>
#include <string>
#include <unordered_map>
#include <map>
#include <memory>
#include <json/json.h>
#include "base/http.h"
#include "speech.h"

#define RF_PATH "./temp_file/asr.wav"
#define CMD_ETC "./command.etc"
#define KEY_ETC "./key.etc"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

std::unordered_map<string, string> key_val;

class Util
{
public:
    static bool Exec(string cmd, bool is_print)
    {
        if(!is_print)
            cmd += " >/dev/null 2>&1";
        FILE *fp = popen(cmd.c_str(), "r");
        if(nullptr == fp)
        {
            cerr << "Exec error" << endl;
            return false;
        }
        char c;
        while(fread(&c, 1, 1, fp) > 0) ////is_print为true时，打印输出信息
        {
            cout << c;
        }
        pclose(fp);
        return true;
    }

    static void Load(string path, std::unordered_map<string, string>& map)
    {
        char buf[256];
        std::ifstream in(path.c_str());
        if(!in.is_open())
        {
            cerr << "Open file failed." << endl;
            exit(1);
        }
        string seq = ":";
        while(in.getline(buf, sizeof(buf)))
        {
            string str = buf;
            size_t index = str.find(seq);
            if(index == string::npos)
            {
                cerr << "Seq not found." << endl;
                continue;
            }
            string msg = str.substr(0, index);
            string cmd = str.substr(index+seq.size());
            map[msg] = cmd;
        }
    }
};


class Robot
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
            cerr << "parse error: " << errs << endl;
            return "";
        }
        int code = root["intent"]["code"].asInt();
        string err_msg;
        if(!IsCodeLegal(code, err_msg))
        {
            cerr << "Response Code Error: " << err_msg << endl;
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
            cerr << "post error" << endl;
            return "";
        }
        return response;
    }
public:
    Robot() = default;
    void Init(string id = "1")
    {
        this->Url = "http://openapi.tuling123.com/openapi/api/v2";
        this->Api_Key = key_val["Tul_Api_Key"];
        this->User_Id = id;
        this->Client = new aip::HttpClient;
    }

    ~Robot()
    {}
    
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

class Speech //语音识别与合成
{
private:
    string app_id;
    string api_key;
    string secret_key;
    aip::Speech *client;
private:

public:
    Speech() = default;
    void Init()
    {
        app_id = key_val["Baidu_App_Id"];
        api_key = key_val["Baidu_Api_Key"];
        secret_key = key_val["Baidu_Secret_Key"];
        client = new aip::Speech(app_id, api_key, secret_key);
    }

    ~Speech()
    {}

    bool ASR(string& msg) //语音识别
    {
        std::map<string, string> options;
        options["dev_pid"] = "1536";
        string file_content;
        aip::get_file_content(RF_PATH, &file_content);
        Json::Value results = client->recognize(file_content, "wav", 16000, options);
        if(results["err_no"].asInt() != 0)
        {
            cerr << "识别失败：" << results["err_no"].asInt() << " " << results["err_msg"].asString() << endl;
            return false;
        }
        //识别成功
        msg = results["result"][0].asString();
        return true;
    }
};

class Ivmt
{
private:
    Speech sh;
    Robot tl;
    std::unordered_map<string, string> msg_cmd;
public:
    Ivmt()
    {
        Util::Load(CMD_ETC, msg_cmd);
        Util::Load(KEY_ETC, key_val);
        sh.Init();
        tl.Init();
    }
    
    bool Record()
    {
        string cmd = "arecord -t wav -c 1 -r 16000 -d 5 -f S16_LE ";
        cmd += RF_PATH;
        if(!Util::Exec(cmd, false))
        {
            cerr << "Record error." << endl;
            return false;
        }
        return true;
    }

    bool Is_cmd(const string& recog_msg, string& cmd)
    {
        auto it = msg_cmd.find(recog_msg);
        if(it == msg_cmd.end())
            return false;
        else
        {
            cmd = it->second;
            return true;
        }
    }

    int Run()
    {
        cout << "录音中..." << endl;
        if(!Record())
            return 1;
        cout << "正在识别 ..." << endl;
        string recog_msg;
        if(!sh.ASR(recog_msg))
            return 2;
        string cmd;
        if(Is_cmd(recog_msg, cmd)) //是命令
        {
            if(!Util::Exec(cmd, true))
                return 3;
        }
        else //不是命令，交给图灵机器人
        {
            cout << "我: " << recog_msg << endl;
            string msg = tl.Talk(recog_msg);
            cout << "图灵机器人: " << msg << endl;
            //语音
        }
        return 0;
    }
    ~Ivmt()
    {}
    
};

#endif //_IVMT__HPP_