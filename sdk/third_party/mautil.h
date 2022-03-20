/**********************************************************************
*
* Copyright (C)2015-2022 Maxwell Analytica. All rights reserved.
*
* @file mautil.h
* @brief
* @details
* @author Maxwell
* @version 1.0.0
* @date 2018-10-09 22:41:27.492
*
**********************************************************************/
#ifndef MAUTIL_H
#define MAUTIL_H

#include <stdint.h>
#include <string>
#include <chrono>
#include <random>
#include <intsafe.h>
#include <algorithm>
#include <ObjBase.h>

namespace ma
{
    class MaUtil
    {
    public:
        static std::string lower(std::string& s)
        {
            transform(s.begin(), s.end(), s.begin(), ::tolower);
            return s;
        }

        static std::string upper(std::string& s)
        {
            transform(s.begin(), s.end(), s.begin(), ::toupper);
            return s;
        }

        static void ltrim(std::string& s)
        {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
                return !std::isspace(ch & 0xff);
                }));
        }

        static void rtrim(std::string& s)
        {
            s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned int ch) {
                return !std::isspace(ch & 0xff);
                }).base(), s.end());
        }

        static void trim(std::string& s)
        {
            ltrim(s);
            rtrim(s);
        }

        static std::string ws2sUtf8(std::wstring ws, DWORD encoding = CP_UTF8)
        {
            if (ws.empty()) return "";

            int utf16len = 0;
            if (SizeTToInt(ws.length(), &utf16len) >= 0)
            {
                int utf8len = WideCharToMultiByte(encoding, 0, ws.c_str(), utf16len, NULL, 0, NULL, NULL);

                std::string ret(utf8len, 0);
                WideCharToMultiByte(encoding, 0, ws.c_str(), utf16len, &ret[0], utf8len, 0, 0);
                return ret;
            }

            return "";
        }

        static std::wstring tohex(char* data, int32_t size)
        {
            constexpr wchar_t hexmap[] = { L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9', L'a', L'b', L'c', L'd', L'e', L'f' };
            std::wstring s(size * 2, L' ');
            for (DWORD i = 0; i < size; ++i)
            {
                s[2 * i] = hexmap[((((PBYTE)data)[i]) & 0xF0) >> 4];
                s[2 * i + 1] = hexmap[((PBYTE)data)[i] & 0x0F];
            }
            //
            return s;
        }

        static std::string ws2s(const std::wstring& wstr)
        {
            std::string result;
             
            int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
            char* buffer = new char[len + 1];
             
            WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);
            buffer[len] = '\0';
             
            result.append(buffer);
            delete[] buffer;
            return result;
        }

        static std::wstring s2ws(const std::string& str)
        {
            std::wstring result;
             
            int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
            TCHAR* buffer = new TCHAR[len + 1];
            
            MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
            buffer[len] = '\0';         
                                            
            result.append(buffer);
            delete[] buffer;
            return result;
        }

        static uint16_t swap16(uint16_t x)
        {
            return ((x & 0x00ff) << 8) | ((x & 0xff00) >> 8);
        }

        static uint32_t swap32(uint32_t x)
        {
            return ((x & 0x000000ff) << 24) |
                ((x & 0x0000ff00) << 8) |
                ((x & 0x00ff0000) >> 8) |
                ((x & 0xff000000) >> 24);
        }

        static uint64_t swap64(uint64_t x)
        {
            return ((x & 0x00000000000000ff) << 56) |
                ((x & 0x000000000000ff00) << 40) |
                ((x & 0x0000000000ff0000) << 24) |
                ((x & 0x00000000ff000000) << 8) |
                ((x & 0x000000ff00000000) >> 8) |
                ((x & 0x0000ff0000000000) >> 24) |
                ((x & 0x00ff000000000000) >> 40) |
                ((x & 0xff00000000000000) >> 56);
        }

        static std::string getTimeStamp()
        {
            auto tps = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
            auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tps.time_since_epoch());
            time_t timestamp = tmp.count();
            
            time_t milli = timestamp + (time_t)8 * 60 * 60 * 1000;
            auto mTime = std::chrono::milliseconds(milli);
            auto tp = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>(mTime);
            auto tt = std::chrono::system_clock::to_time_t(tp);
            std::tm* now = gmtime(&tt);
            char szBuf[1024] = { 0x00 };
            long millseconds = milli % 1000;

            sprintf_s(szBuf, "%4d-%02d-%02d %02d:%02d:%02d.%03d", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, millseconds);

            return std::string(szBuf);
        }

        static std::vector<std::string> split_string(const std::string& str, char tag)
        {
            std::vector<std::string> strlist;
            std::string subStr;
            for (size_t i = 0; i < str.length(); i++)
            {
                if (tag == str[i])
                {
                    if (!subStr.empty())
                    {
                        strlist.push_back(subStr);
                        subStr.clear();
                    }
                }
                else
                {
                    subStr.push_back(str[i]);
                }
            }

            if (!subStr.empty())
            {
                strlist.push_back(subStr);
            }

            return strlist;
        }

        static std::string& replace_str(std::string& str, const std::string& to_replaced, const std::string& newchars)
        {
            for (std::string::size_type pos(0); pos != std::string::npos; pos += newchars.length())
            {
                pos = str.find(to_replaced, pos);
                if (pos != std::string::npos)
                    str.replace(pos, to_replaced.length(), newchars);
                else
                    break;
            }
            
            return str;
        }

        static std::string GenerateGuid(int32_t bit = 16)
        {
            GUID guid;
            CoCreateGuid(&guid);
            char cBuffer[64] = { 0 };
            sprintf_s(cBuffer, sizeof(cBuffer),
                "%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
                guid.Data1, guid.Data2,
                guid.Data3, guid.Data4[0],
                guid.Data4[1], guid.Data4[2],
                guid.Data4[3], guid.Data4[4],
                guid.Data4[5], guid.Data4[6],
                guid.Data4[7]);
            
            std::string res(cBuffer);
            std::transform(res.begin(), res.end(), res.begin(), ::tolower);

            return res;
        }

        static int32_t GoodSuffix(int32_t j, char* pat, int32_t size)
        {
            int32_t terminal = size - 1;
            int32_t index = -1;
            j--;
            while (j >= 0)
            {
                if (pat[j] == pat[terminal]) {
                    index = j;
                    break;
                }
                else j--;
            }
            
            return (terminal - index);
        }
       
        static int32_t BadChar(int32_t j, char temp, char* str)
        {

            int32_t index = -1;
            for (int32_t i = j - 1; i >= 0; --i)
            {
                if (str[i] == temp)
                {
                    index = i;
                    break;
                }
            }
            
            return (j - index);
        }

        static int32_t BoyerMoore(char* source, int32_t source_size, char* target, int32_t target_size)
        {
            int32_t i = 0, j = 0, soulen = source_size, tarlen = target_size;
            int32_t badValue = 0, distance = 0;
            if (soulen < tarlen)
            {
                return -1;
            }
            
            i = tarlen - 1;
            j = tarlen - 1;
            
            while (i < soulen)
            {
                if (source[i] == target[j])
                {
                    if (j == 0)
                    {
                        return i;
                    }
                    i--;
                    j--;
                }
                else
                {
                    if (j == tarlen - 1)
                    {
                        badValue = BadChar(j, source[i], target);
                        i = i + tarlen - 1 - j + badValue;
                        j = tarlen - 1;
                    }
                    else
                    {
                        badValue = BadChar(j, source[i], target);
                        if (badValue == -1)
                            badValue = target_size;
                        
                        int32_t goodValue = GoodSuffix(j, target, target_size);
                        distance = badValue > goodValue ? badValue : goodValue;
                        
                        i = i + tarlen - 1 - j + distance;
                        j = tarlen - 1;
                    }
                }
            }
            
            return -1;
        }

    };
}

#endif // MAUTIL_H