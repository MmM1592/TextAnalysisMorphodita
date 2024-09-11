#include "main.h"
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <unordered_map>
#include <pugixml.hpp>
#include <nlohmann/json.hpp>
#include <regex>
#include <windows.h>

//jednodušší práce s JSON
using json = nlohmann::json;

//uložení dat z cURL
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

//inicializace a získání dat z Morphodity přes cURL
std::string getDataWithCurl(const std::string &url) {
    CURL *curl;
    CURLcode result;
    std::string buffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        result = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (result != CURLE_OK) {
            std::cout << "curl_easy_perform() failed" << std::endl;
            return "";
        }
    } else {
        std::cout << "cURL initialization failed" << std::endl;
        return "";
    }
    std::cout << buffer << std::endl;
    return buffer;
}

//zpracování XML tokenů a uložení do map
void processTokens(pugi::xml_document &doc,
    std::unordered_map<std::string, int> &hashMapWordLemma,
    std::unordered_map<std::string, int> &hashMapPunctuation,
    std::unordered_map <std::string, int> &hashMapWordClass) {
    std::regex punctuationRegex(R"([[:punct:]])");

    for (pugi::xml_node sentence : doc.children("sentence")) {
        for (pugi::xml_node token : sentence.children("token")) {
            std::string form = token.text().get();
            std::string lemma = token.attribute("lemma").value(); //infinitiv slova
            std::string tag = token.attribute("tag").value(); //slovní druh

            //uložení slov a znamének do map
            if (std::regex_match(form, punctuationRegex)) {
                hashMapPunctuation[form] += 1;
            } else {
                hashMapWordLemma[lemma] += 1;
                hashMapWordClass[tag] += 1;
            }
        }
    }
}

//zpracování JSON odpovědi
bool processJsonResponse(std::string &buffer, std::unordered_map<std::string, int> &hashMapWordLemma,
    std::unordered_map<std::string, int> &hashMapPunctuation,
    std::unordered_map<std::string, int> &hashMapWordClass) {
    try {
        json jsonResponse = json::parse(buffer);
        std::string xmlResult = jsonResponse["result"];

        pugi::xml_document doc;
        pugi::xml_parse_result parsedResults = doc.load_string(xmlResult.c_str());

        if (!parsedResults) {
            std::cout << "XML parsing error" << std::endl;
            return false;
        }

        processTokens(doc, hashMapWordLemma, hashMapPunctuation, hashMapWordClass);
    } catch (json::parse_error &e) {
        std::cout << "JSON error" << std::endl;
        return false;
    }
    return true;
}

//výpis slov, znamének a jejich výskytů v textu
void listTextElements(std::unordered_map<std::string, int> &hashMapWordLemma, std::unordered_map<std::string, int> &hashMapPunctuation) {
    for (auto i : hashMapWordLemma) {
        std::cout << u8"Slovo: " << i.first << u8" Počet výskytů: " << i.second << std::endl;
    }

    for (auto i : hashMapPunctuation) {
        std::cout << u8"Znaménko: " << i.first << u8" Počet výskytů: " << i.second << std::endl;
    }
}

//výpis četnosti slovních druhů
void wordClassCount(std::unordered_map<std::string, int> &hashMapWordClass) {
    std::cout << u8"Slovní druhy" << std::endl;
    for (auto i: hashMapWordClass) {
        std::cout << i.first << ": " << i.second << std::endl;
    }
}

//výpočet počtu slov
int wordCount(std::unordered_map<std::string, int> &hashMapWordLemma) {
    int totalWordCount = 0;

    for (auto &pair : hashMapWordLemma) {
        totalWordCount += pair.second;
    }
    std::cout << u8"Celkový počet slov: ";
    return totalWordCount;
}

//nalezení nejpoužívanějšího slova
void mostUsedWord(std::unordered_map<std::string, int> &hashMapWordLemma) {
    if (hashMapWordLemma.empty()) {
        std::cout << u8"Žádná slova k analýze." << std::endl;
        return;
    }

    int maxCount = 0; //nejvyšší počet výskytů
    for (const auto &pair : hashMapWordLemma) {
        if (pair.second > maxCount) {
            maxCount = pair.second;
        }
    }

    std::vector<std::string> mostUsedWords; //seznam všech slov s nejvyšším počtem výskytů
    for (auto &pair : hashMapWordLemma) {
        if (pair.second == maxCount) {
            mostUsedWords.push_back(pair.first);
        }
    }

    std::cout << u8"Nejpoužívanější slova jsou: ";
    for (auto &word : mostUsedWords) {
        std::cout << word << " ";
    }
    std::cout << "(" << maxCount << u8" krát)" << std::endl;
}




int main() {
    SetConsoleOutputCP(CP_UTF8);
    std::string baseURL = "https://lindat.mff.cuni.cz/services/morphodita/api/tag?data=";
    std::string data = u8"!! ale Ahoj, ahoj a pes se Tomáš, člověk.... . ??";

    std::unordered_map<std::string, int> hashMapWordLemma; //hash mapa pro uložení slov
    std::unordered_map<std::string, int> hashMapPunctuation; //hash mapa pro uložení interpunkčních znamének
    std::unordered_map<std::string, int> hashMapWordClass; //hash mapa pro uložení slovních druhů

    std::string url = baseURL + curl_easy_escape(curl_easy_init(), data.c_str(), data.length());

    //získání dat z cURL
    std::string buffer = getDataWithCurl(url);
    if (buffer.empty()) return 1;

    //zpracování JSON odpovědi
    if (!processJsonResponse(buffer, hashMapWordLemma, hashMapPunctuation, hashMapWordClass)) return 1;

    //výpis prvků
    listTextElements(hashMapWordLemma, hashMapPunctuation);

    //vypsání výsledků
    std::cout << wordCount(hashMapWordLemma) << std::endl;
    wordClassCount(hashMapWordClass);
    mostUsedWord(hashMapWordLemma);

    return 0;
}