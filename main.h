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

//zpracov�n� XML token� a ulo�en� do map
void processTokens(pugi::xml_document &doc,
    std::unordered_map<std::string, int> &hashMapWordLemma,
    std::unordered_map<std::string, int> &hashMapPunctuation,
    std::unordered_map <std::string, int> &hashMapWordClass);

//zpracov�n� JSON odpov�di
bool processJsonResponse(std::string &buffer, std::unordered_map<std::string, int> &hashMapWordLemma,
    std::unordered_map<std::string, int> &hashMapPunctuation,
    std::unordered_map<std::string, int> &hashMapWordClass);

//export zanalyzovan�ch dat do CSV soubor� 
void exportDataToCSV(std::unordered_map<std::string, int> &hashMapWordLemma,
    std::unordered_map<std::string, int> &hashMapPunctuation,
    std::unordered_map<std::string, int> &hashMapWordClass);

//utf 8 k�dov�n� p�i z�pisu do csv soubor�
void writeUtf8BOM(std::ofstream &file); 