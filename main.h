#pragma once
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <unordered_map>
#include <pugixml.hpp>
#include <nlohmann/json.hpp>
#include <windows.h>

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
std::string getDataWithCurl(const std::string &url);

//zpracování XML tokenù a uložení do map
void processTokens(pugi::xml_document &doc,
    std::unordered_map<std::string, int> &hashMapWordLemma,
    std::unordered_map<std::string, int> &hashMapPunctuation,
    std::unordered_map <std::string, int> &hashMapWordClass);

//zpracování JSON odpovìdi
bool processJsonResponse(std::string &buffer, std::unordered_map<std::string, int> &hashMapWordLemma,
    std::unordered_map<std::string, int> &hashMapPunctuation,
    std::unordered_map<std::string, int> &hashMapWordClass);

//výpis slov, znamének a jejich výskytù v textu
void listTextElements(std::unordered_map<std::string, int> &hashMapWordLemma, std::unordered_map<std::string, int> &hashMapPunctuation);

//výpis èetnosti slovních druhù
void wordClassCount(std::unordered_map<std::string, int> &hashMapWordClass);

//výpoèet poètu slov
int wordCount(std::unordered_map<std::string, int> &hashMapWordLemma);

//nalezení nejpoužívanìjšího slova
void mostUsedWord(std::unordered_map<std::string, int> &hashMapWordLemma);