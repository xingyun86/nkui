// EnumHostAddr.h : Include file for standard system include files,
// or project specific include files.
#pragma once

#include <iostream>
// TODO: Reference additional headers your program requires here.
#include <string>
#include <thread>
#include <mutex>
#include <string>
#include <fstream>
#include <streambuf>
#include <unordered_map>
#define file_reader(F) std::string((std::istreambuf_iterator<char>(std::ifstream(F, std::ios::binary | std::ios::in).rdbuf())), std::istreambuf_iterator<char>())
#include <ppsyqm/json.hpp>
using namespace ppsyqm;
#ifdef _MSC_VER
#define  _WINSOCK_DEPRECATED_NO_WARNINGS 
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <limits.h>
#endif
#pragma pack(1)
typedef struct __xxx__ { uint8_t v; }__xxx__;
#pragma pack()
#include <vector>

class SockUtil {
public:
#ifdef _MSC_VER
    WSADATA wsadata = { 0 };
#endif
    SockUtil()
    {
#ifdef _MSC_VER
        //初始化套接字库
        WORD w_req = MAKEWORD(2, 2);//版本号
        int err;
        err = WSAStartup(w_req, &wsadata);
        if (err != 0)
        {
            std::cout << "Initialize winsock library failed！" << std::endl;
        }
        else
        {
            std::cout << "Initialize winsock library ok！" << std::endl;
        }
        //检测版本号
        if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2)
        {
            std::cout << "Winsock library version failed！" << std::endl;
            WSACleanup();
        }
        else
        {
            std::cout << "Winsock library version ok！" << std::endl;
        }
#endif
    }
    ~SockUtil()
    {
#ifdef _MSC_VER
        WSACleanup();
#endif
    }
private:
    int enum_host_addr(std::vector<std::string>& sv, int af/*= AF_INET or AF_INET6*/)
    {
        int ret = 0;
        char ip[65] = { 0 };
        struct sockaddr_in* addr = nullptr;
#ifdef _MSC_VER
        char host_name[33] = { 0 };
        struct addrinfo hints = { 0 };
        struct addrinfo* res = nullptr;
        struct addrinfo* cur = nullptr;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = af; /* Allow IPv4 */
        hints.ai_flags = AI_PASSIVE; /* For wildcard IP address */
        hints.ai_protocol = 0; /* Any protocol */
        hints.ai_socktype = SOCK_STREAM;
        ret = gethostname(host_name, sizeof(host_name) / sizeof(*host_name));
        if (ret == 0)
        {
            ret = getaddrinfo(host_name, nullptr, &hints, &res);
            if (ret == 0) {
                for (cur = res; cur != nullptr; cur = cur->ai_next) {
                    addr = (struct sockaddr_in*)cur->ai_addr;
                    inet_ntop(af, &addr->sin_addr, ip, sizeof(ip) / sizeof(*ip));
                    //std::cout << ip << std::endl;
                    sv.emplace_back(ip);
                }
                freeaddrinfo(res);
            }
        }
#else
        struct ifaddrs* ifa = nullptr;
        struct ifaddrs* oifa = nullptr;
        ret = getifaddrs(&ifa);
        if (ret == 0)
        {
            oifa = ifa;
            while (ifa != nullptr)
            {
                // IPv4 排除localhost
                if (ifa->ifa_addr != nullptr
                    && ifa->ifa_addr->sa_family == af
                    && strncmp(ifa->ifa_name, "lo", 2) != 0)
                {
                    addr = (struct sockaddr_in*)ifa->ifa_addr;
                    inet_ntop(af, &addr->sin_addr, ip, sizeof(ip) / sizeof(*ip));
                    //std::cout << ip << std::endl;
                    sv.emplace_back(ip);
                }
                ifa = ifa->ifa_next;
            }
            freeifaddrs(oifa);
        }
#endif
        return ret;
    }
public:
    int enum_host_addr_ipv4(std::vector<std::string>& sv)
    {
        return enum_host_addr(sv, AF_INET);
    }
    int enum_host_addr_ipv6(std::vector<std::string>& sv)
    {
        return enum_host_addr(sv, AF_INET6);
    }
public:
    static SockUtil* Inst()
    {
        static SockUtil SockUtilInstance;
        return &SockUtilInstance;
    }
};
class UtilTool {
#define GCUT UtilTool::Inst()
    enum STATE_TYPE {
        ST_CLOSE = 0,
        ST_STARTING,
        ST_WAITING,
        ST_CALIBRATION,//定位模式 
        ST_WORKING,
        ST_ERROR,
    };
private:
    json config;
    bool state = false;
    std::mutex locker;
    std::string app_dir = APP_DIR();
    std::thread task_thread;
    std::vector<std::string> ip_list = {};
    std::string conf_json_file = APP_DIR() + "/conf.json";
    const std::unordered_map<uint32_t, std::string> state_info_list =
    {
        {ST_CLOSE,u8"已关闭"},
        {ST_STARTING,u8"正在启动中..."},
        {ST_WAITING,u8"正在等待..."},
        {ST_CALIBRATION,u8"定位校准中..."},
        {ST_WORKING,u8"正常工作中..."},
        {ST_ERROR,u8"异常"},
    };
    
public:
    std::string font_file = APP_DIR() + "/res/fonts/msyh.ttf";
    std::string home_file = APP_DIR() + "/res/icons/home.png";
public:
    typedef int(*PFN_CALLBACK)(void*, void*);
    void ShowAPUState(PFN_CALLBACK fnCallBack, void* p)
    {
        this->locker.lock();
        std::string u8State = u8"服务主机";
        if (config["state"].is_number_integer())
        {
            uint32_t nState = config["state"].get<uint32_t>();
            if (state_info_list.find(nState) != state_info_list.end())
            {
                u8State.append(u8" - [状态: ").append(state_info_list.at(nState)).append(u8"]");
            }
        }
        if (fnCallBack != nullptr)
        {
            fnCallBack((void*)u8State.c_str(), p);
        }
        this->locker.unlock();
    }
    void ShowIPList(PFN_CALLBACK fnCallBack, void * p)
    {
        this->locker.lock();
        if (fnCallBack != nullptr)
        {
            fnCallBack(&ip_list, p);
        }
        this->locker.unlock();
    }
private:
    std::string APP_DIR()
    {
#ifdef _MSC_VER
        char workpath[MAX_PATH];
        GetModuleFileNameA(NULL, workpath, MAX_PATH - 1);
        for (int i = 0; i < MAX_PATH && workpath[i]; i++) {
            if (workpath[i] == '\\')
            {
                workpath[i] = '/';
            }
        }
#else
        char workpath[PATH_MAX];
        char procpath[64];
        sprintf(procpath, "/proc/%d/exe", getpid());
        if (readlink(procpath, workpath, sizeof(workpath) - 2) <= 0) {
            assert(0);
        }
#endif

        char* p = strrchr(workpath, '/');
        if (p)
        {
            *p = 0x00;
        }
        return workpath;
    }
    void Start()
    {
        this->state = true;
        task_thread = std::thread([](void* p) {
            UtilTool* thiz = (UtilTool*)p;
            while (thiz->state == true)
            {
                std::vector<std::string> _ip_list;
                SockUtil::Inst()->enum_host_addr_ipv4(_ip_list);
                if (_ip_list.size() != thiz->ip_list.size())
                {
                    thiz->locker.lock();
                    thiz->ip_list.clear();
                    thiz->ip_list.assign(_ip_list.begin(), _ip_list.end());
                    thiz->locker.unlock();
                }
                else
                {
                    bool bSame = false;
                    for (auto _it : _ip_list)
                    {
                        bSame = false;
                        for (auto it : thiz->ip_list)
                        {
                            if (_it.compare(it) == 0)
                            {
                                bSame = true;
                                break;
                            }
                        }
                        if (bSame == false)
                        {
                            break;
                        }
                    }
                    if (bSame == false)
                    {
                        thiz->locker.lock();
                        thiz->ip_list.clear();
                        thiz->ip_list.assign(_ip_list.begin(), thiz->ip_list.end());
                        thiz->locker.unlock();
                    }
                }
                auto data = file_reader(thiz->conf_json_file);
                if (data.length() > 0)
                {
                    try
                    {
                        thiz->locker.lock();
                        thiz->config = json::parse(data);
                        thiz->locker.unlock();
                    }
                    catch (const std::exception&)
                    {
                        ;//pass
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            }, this);
    }
    void Stop()
    {
        state = false;
        if (task_thread.joinable()) {
            task_thread.join();
        }
    }
public:
    UtilTool(){
        Start();
    }
    ~UtilTool() {
        Stop();
    }
   
public:
    static UtilTool* Inst() {
        static UtilTool UtilToolInstance;
        return &UtilToolInstance;
    }
};
