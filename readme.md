vmx_editor
----
虚拟机*.vmx文件编辑接口

- 支持 *.vmx 文件内容解析
- 支持 *.vmx 文件创建及修改
- 只有一个头文件,包含一下就能用了

## 创建vmx文件

- 所vmx文件内容:

```
.encoding = "GBK"
config.version = "8"
displayName = "Windows 10 x64"
nvme0.present = "8"
sharedFolder0.hostPath = "E:\vmware\共享"
usb_xhci:4.deviceType = "hid"
usb_xhci:4.present = "TRUE"
vmotion.svga.maxTextureSize = "16384"
```

- 使用方法:

```
vmware::vmx_editor vmx;
vmx[L""][L"encoding"] = L"GBK";
vmx[L"displayName"] = L"Windows 10 x64";
vmx[L"config"][L"version"] = 8;
vmx[L"nvme0"][L"present"] = 8;
vmx[L"usb_xhci"][4][L"present"] = true;
vmx[L"usb_xhci"][4][L"deviceType"] = L"hid";
vmx[L"vmotion"][L"svga"][L"maxTextureSize"] = 16384;
vmx[L"sharedFolder0"][L"hostPath"] = L"E:\\vmware\\共享";

// 1. 输出到字符串
std::wstring buffer = vmx.to_wstring();

// 2. 输出到指定文件路径
vmx.to_file(L"F:\\2.txt");

// 3. 输出到字符流
std::wostringstream oss;
oss << vmx;

// 4. 输出到文件流
std::wofstream ofs;
ofs.open("F:\\1.txt");
ofs.imbue(std::locale("chs")); // 支持中文输出
ofs << vmx;
ofs.close();
```

支持2种设置键值的方式:

- 通过`[]`操作符
- 通过`set_value`方法

```
vmware::vmx_editor vmx;

// 以下的键值设置效果是相同的
// 输出内容: usb_xhci:4.present = "TRUE"

// 方式1: 通过"[]"操作符
vmx[L"usb_xhci"][4][L"present"] = true;

// 方式2: 通过"set_value"方法
vmx.set_value(L"usb_xhci:4.present", L"TRUE");

```

## 读取vmx文件

```
vmware::vmx_editor vmx;
vmx.from_file(L"E:\\vm_os\\win10\\Windows 10 x64.vmx");

std::wstring x = vmx[L"vmotion"][L"svga"][L"maxTextureSize"];
std::wstring y = vmx[L"usb_xhci"][4][L"present"];
std::wstring z = vmx[L"usb_xhci"][4][L"present"][L"helloworld"];
std::wstring q = vmx[L"displayName"];
```
