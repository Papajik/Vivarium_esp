// #ifndef _HELPER_H_
// #define _HELPER_H_
// #include <WString.h>
// #include <vector>
// #include <string>

// static bool hasDelimeter(String k)
// {
//     std::string s = k.c_str();
//     return s.find('.') != std::string::npos;
// }

// static std::vector<String> splitString(String keys, std::string token)
// {
//     std::vector<String> result;
//     std::string str = keys.c_str();

//     while (str.size())
//     {
//         int index = str.find(token);
//         if (index != std::string::npos)
//         {
//             result.push_back(String(str.substr(0, index).c_str()));
//             str = str.substr(index + token.size());
//             if (str.size() == 0)
//                 result.push_back(String(str.c_str()));
//         }
//         else
//         {
//             result.push_back(String(str.c_str()));
//             str = "";
//         }
//     }
//     return result;
// }

// #endif