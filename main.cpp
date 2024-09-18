#include "main.h"
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <unordered_map>
#include <pugixml.hpp>
#include <nlohmann/json.hpp>
#include <regex>
#include <windows.h>
#include <fstream>

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
    } 
    else {
        std::cout << "cURL initialization failed" << std::endl;
        return "";
    }
    //std::cout << buffer << std::endl;
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
            } 
            else {
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

void exportDataToCSV(std::unordered_map<std::string, int> &hashMapWordLemma,
    std::unordered_map<std::string, int> &hashMapPunctuation,
    std::unordered_map<std::string, int> &hashMapWordClass) {

    //výpis obecných dat o textu do csv souboru obecnaData.csv
    std::ofstream csvFile1("obecnaData.csv");

    if (csvFile1.is_open()) {
        //zjištění počtu slov
        int totalWords = 0;
        for (auto &pair : hashMapWordLemma) {
            totalWords += pair.second;
        }

        //nalezení nejpoužívanějšíáho slova
        auto mostFrequentWord = std::max_element(hashMapWordLemma.begin(), hashMapWordLemma.end(),
            [](auto &a, auto &b) {
            return a.second < b.second;
        });

        //zápis počtu slov a nejpoužívanějšího slova
        csvFile1 << u8"POČET SLOV," << totalWords << "\n";
        csvFile1 << u8"NEJPOUŽÍVANĚJŠÍ SLOVO," << mostFrequentWord->first << "," << mostFrequentWord->second << "\n";

        csvFile1.close();

        std::cout << "Data byla exportována do souboru obecnaData.csv\n";
    } else {
        std::cout << "Nepodařilo se exportovat data do CSV souboru" << std::endl;
    }

    //výpis jednotlivých slov, slovních druhů a interpunkčních znamének a jejich četnosti do csv souboru list.csv 
    std::ofstream csvFile2("list.csv");

    if (csvFile2.is_open()) {
        writeUtf8BOM(csvFile2);
        //výpis slov
        csvFile2 << u8"SLOVA,POČET\n";
        for (auto &pair : hashMapWordLemma) {
            csvFile2 << pair.first << "," << pair.second << "\n";
        }

        csvFile2 << "\n"; //oddělení řádků

        //výpis interpunkčních znamének
        csvFile2 << u8"INTERPUNKČNÍ ZNAMÉNKA,POČET\n";
        for (auto &pair : hashMapPunctuation) {
            if (pair.first == ",") {
                csvFile2 << u8"čárka" << "," << pair.second << "\n";
            }
            else {
                csvFile2 << pair.first << "," << pair.second << "\n";
            }
        }

        csvFile2 << "\n"; //oddělení řádků

        //výpis slovních druhů
        csvFile2 << u8"SLOVNÍ DRUHY,POČET\n";
        for (auto &pair : hashMapWordClass) {
            csvFile2 << pair.first << "," << pair.second << "\n";
        }

        csvFile2.close();

        std::cout << "Data byla exportována do souboru list.csv\n";

    } else {
        std::cout << "Nepodařilo se exportovat data do CSV souboru" << std::endl;
    }
}

void writeUtf8BOM(std::ofstream &file) {
    const char bom[] = {static_cast<char>(0xEF), static_cast<char>(0xBB), static_cast<char>(0xBF)};
    file.write(bom, sizeof(bom));
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

    //zápis analyzovaných dat do CSV souborů
    exportDataToCSV(hashMapWordLemma, hashMapPunctuation, hashMapWordClass);

    return 0;
}