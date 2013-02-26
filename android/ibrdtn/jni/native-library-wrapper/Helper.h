#ifndef HELPER_H_
#define HELPER_H_

#include <string>

std::string jstringToStdString(JNIEnv*, jstring);

jstring stdStringToJstring(JNIEnv*, std::string);

#endif
