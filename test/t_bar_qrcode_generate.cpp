
#include <codecvt>
#include <locale>
#include <string>

#include <iostream>
#include <cstring>
#include <cstdio>

#include <zint.h>
 
#include <codecvt>
#include <string>
 
std::wstring s2ws(const std::string& str)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
 
    return converterX.from_bytes(str);
}
 
std::string ws2s(const std::wstring& wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
 
    return converterX.to_bytes(wstr);
}

int main(int argc, char * argv[])
{
    struct zint_symbol * sym = ZBarcode_Create();
    sym->symbology = BARCODE_QRCODE;
    sym->option_1 = 4;
    sym->option_2 = 7;
    sym->input_mode = DATA_MODE;

    std::string msg = ws2s(s2ws(argv[1]));
    ZBarcode_Encode(sym, (uint8_t *)msg.data(), 0);
    ZBarcode_Print(sym, 0);
    ZBarcode_Delete(sym);

    return 0;
}

