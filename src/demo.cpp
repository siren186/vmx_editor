#include <iostream>
#include <fstream>
#include <sstream>
#include "vmx_editor.hpp"


void test4()
{
    auto filepath = LR"(E:\vm_os\win10\Windows 10 x64.vmx)";
    vmware::vmx_editor vmx;
    vmx.from_file(filepath);
    auto str = vmx.to_wstring();
}

void test3()
{
    vmware::vmx_editor vmx;
    vmx.from_wstring(L"\n\n\n\n\n\nx=\"");
    auto str = vmx.to_wstring();
    vmx.from_wstring(L"usb_xhci:4.deviceType = \"hid\"");
    str = vmx.to_wstring();
    vmx.from_wstring(L"\nusb_xhci:4.deviceType = \"hid\"\nfloppy0.present = \"FALSE\"");
    str = vmx.to_wstring();
    vmx.from_wstring(L"\n\n\n\n\n\ny=\"\"");
    vmx.from_wstring(L"\n\n\n\n\n\nz=\"yao yu laing\"sdfasddfas");
    str = vmx.to_wstring();
}

void test2()
{
    vmware::vmx_editor vmx;
    bool suc = false;
    suc = vmx.set_value(L"     hello   .   world  : 32  .   sheet", L"UTF-X");
    suc = vmx.set_value(L":HELLO.32", L"UTF-X");
    suc = vmx.set_value(L"1.32:4:", L"UTF-9");
    suc = vmx.set_value(L"1.32:5.", L"UTF-9");
    suc = vmx.set_value(L":3.usb_xhci:4.deviceType", L"hid");
    suc = vmx.set_value(L"checkpoint.vmState.readOnly", L"FALSE");
    suc = vmx.set_value(L"usb_xhci:4.deviceType", L"hid");
    suc = vmx.set_value(L".encoding", L"UTF-8");
    suc = vmx.set_value(L".32", L"UTF-32");
    suc = vmx.set_value(L"1.32:4", L"UTF-9");
    auto str = vmx.to_wstring();
}

void test()
{
    vmware::vmx_editor vmx;
    vmx[L""][L"encoding"] = L"GBK";
    vmx[L"displayName"] = L"Windows 10 x64";
    vmx[L"config"][L"version"] = 8;
    vmx[L"nvme0"][L"present"] = 8;
    vmx[L"usb_xhci"][4][L"present"] = true;
    vmx[L"usb_xhci"][4][L"deviceType"] = L"hid";
    vmx[L"vmotion"][L"svga"][L"maxTextureSize"] = 16384;
    vmx[L"sharedFolder0"][L"hostPath"] = L"E:\\vmware\\共享";

    std::wstring x = vmx[L"vmotion"][L"svga"][L"maxTextureSize"];
    std::wstring y = vmx[L"usb_xhci"][4][L"present"];
    std::wstring z = vmx[L"usb_xhci"][4][L"present"][L"yao"];
    std::wstring q = vmx[L"displayName"];

    std::wstring buffer = vmx.to_wstring();
    bool bf = vmx.to_file(L"F:\\2.txt");

    std::wostringstream oss;
    oss << vmx;
    auto yao = oss.str();

    std::wofstream ofs;
    ofs.open("F:\\1.txt");
    ofs.imbue(std::locale("chs")); // 支持中文输出
    ofs << vmx;
    ofs.close();
}

int main()
{
    test4();
    return 0;
}
