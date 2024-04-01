
#include <Arduino.h>
#include "Barcode.h"
#include <cstring>
// #include <string>

Barcode::Barcode() {
  //
}

using namespace std;

const String values_table[][2] = {
  { " ", "212222" },
  { "!", "222122" },
  { "\"", "222221" },
  { "#", "121223" },
  { "$", "121322" },
  { "%", "131222" },
  { "&", "122213" },
  { "'", "122312" },
  { "(", "132212" },
  { ")", "221213" },
  { "*", "221312" },
  { "=+", "231212" },
  { ",", "112232" },
  { "-", "122132" },
  { ".", "122231" },
  { "/", "113222" },
  { "0", "123122" },
  { "1", "123221" },
  { "2", "223211" },
  { "3", "221132" },
  { "4", "221231" },
  { "5", "213212" },
  { "6", "223112" },
  { "7", "312131" },
  { "8", "311222" },
  { "9", "321122" },
  { ":", "321221" },
  { ";", "312212" },
  { "<", "322112" },
  { "=", "322211" },
  { ">", "212123" },
  { "?", "212321" },
  { "@", "232121" },
  { "A", "111323" },
  { "B", "131123" },
  { "C", "131321" },
  { "D", "112313" },
  { "E", "132113" },
  { "F", "132311" },
  { "G", "211313" },
  { "H", "231113" },
  { "I", "231311" },
  { "J", "112133" },
  { "K", "112331" },
  { "L", "132131" },
  { "M", "113123" },
  { "N", "113321" },
  { "O", "133121" },
  { "P", "313121" },
  { "Q", "211331" },
  { "R", "231131" },
  { "S", "213113" },
  { "T", "213311" },
  { "U", "213131" },
  { "V", "311123" },
  { "W", "311321" },
  { "X", "331121" },
  { "Y", "312113" },
  { "Z", "312311" },
  { "[", "332111" },
  { "\\", "314111" },
  { "]", "221411" },
  { "^", "431111" },
  { "_", "111224" },
  { "NUL", "111422" },
  { "SOH", "121124" },
  { "STX", "121421" },
  { "ETX", "141122" },
  { "EOT", "141221" },
  { "ENQ", "112214" },
  { "ACK", "112412" },
  { "BEL", "122114" },
  { "BS", "122411" },
  { "HT", "142112" },
  { "LF", "142211" },
  { "VT", "241211" },
  { "FF", "221114" },
  { "CR", "413111" },
  { "SO", "241112" },
  { "SI", "134111" },
  { "DLE", "111242" },
  { "DC1", "121142" },
  { "DC2", "121241" },
  { "DC3", "114212" },
  { "DC4", "124112" },
  { "NAK", "124211" },
  { "SYN", "411212" },
  { "ETB", "421112" },
  { "CAN", "421211" },
  { "EM", "212141" },
  { "SUB", "214121" },
  { "ESC", "412121" },
  { "FS", "111143" },
  { "GS", "111341" },
  { "RS", "131141" },
  { "US", "114113" },
  { "FNC 3", "114311" },
  { "FNC 2", "411113" },
  { "Shift B", "411311" },
  { "Code C", "113141" },
  { "Code B", "114131" },
  { "FNC 4", "311141" },
  { "FNC 1", "411131" },
  { "Start Code A", "211412" },
  { "Start Code B", "211214" },
  { "Start Code C", "211232" },
  { "Stop", "233111" },
  { "Reverse Stop", "211133" },
  { "Stop pattern (7 bars/spaces)", "2331112" },
};

String Barcode::get_bars(char character, int& checkSum, int position) {

  printf("Checking %c\n", character);
  for (int i = 0; i < (sizeof(values_table) / sizeof(*values_table)); i++) {
    // printf("%d\n", i);
    // cout << values_table[i][0];
    if (values_table[i][0].charAt(0) == character) {
      checkSum += (i)*position;

      return values_table[i][1];
    };
  };
  return "";
}


String Barcode::make(String code) {
  String barcode = "";
  int checkSum = 0;
  int position = 0;

  int n = code.length();
  // declaring character array
  char code_array[n + 1];
  // copying the contents of the
  // string to char array
  strcpy(code_array, code.c_str());

  for (int i = 0; i < n; i++) {
    position = i + 1;
    barcode += get_bars(code_array[i], checkSum, position);
  }

  //retour chariot LF \n 0x0A
  position = n + 1;
  barcode += get_bars(0x0A, checkSum, position);

  checkSum += 103;
  checkSum = checkSum % 103;

  return values_table[103][1] + barcode + values_table[checkSum][1] + values_table[108][1];
}


//int main()
//{
//    string code = "VIC";
//    string barcode = "";
//
//    barcode = make_barcode(code);
//
//    cout << barcode;
//}
