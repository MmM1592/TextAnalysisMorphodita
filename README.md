# TextAnalysis
Tento program je vypracován jako zápočtový program pro předmět Programování v C++ 2 . Jeho cílem je provést jednoduchou analýzu českého textu s využitím nástroje Morphodita. Pro komunikaci s Morphoditou bylo využito API prostřednictvím libcurl přičemž data byla přenášena ve formátu JSON a XML.

POUŽITÉ TECHNOLOGIE:
  1. libcurl - knihovna pro komunikaci s Morphoditou
  2. pugixml - knihovna pro zpracování XML dat
  3. nlohmann/json - knihovna pro práci s JSON daty
  4. Windows.h - použito pro nastavení kódování UTF-8

POUŽITÍ:

Program načítá text z proměnné data a odesílá ho na API Morphodity. Následně jsou získaná XML data zpracována a analyzována za pomocí jednotlivých funkcí. 
Výstup programu pak zahrnuje dva csv soubory:
  1. list.csv - výpis jednotlivých slov, interpunkčních znamének, slovních druhů a jejich četnosti
  2. obecnaData.csv - výpis počtu slov, nejpoužívanější slovo


FUNKCE: 
  1. WriteCallBack: zpracovává odpovědi z libcurl
  2. getDataWithCurl: získává data z API Morphodity
  3. processTokens: ukládá jednotlivé tokeny z XML do hash map
  4. processJsonResponse: zpracovává JSON odpovědi a převádí je na XML
  5. exportDataToCSV - exportování zanalyzovaných dat do csv souborů
  6. writeUtf8BOM - pro zápis v UTF8 kódování do CSV souborů

