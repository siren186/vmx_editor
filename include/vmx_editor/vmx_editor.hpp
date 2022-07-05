#pragma once
#include <string>
#include <ostream>
#include <sstream>
#include <fstream>
#include <exception>
#include <map>


namespace vmware
{

class object_t;
class vmx_editor;


class object_t
{
    friend class vmx_editor;
    friend std::wostream& operator<<(std::wostream& out, const object_t& rhs);

    enum type_t
    {
        OBJECT_TYPE_STRING,
        OBJECT_TYPE_ARRAY
    };

    enum
    {
        BUFFER_LEN_MAX = 512
    };

public:
    object_t(const std::wstring& name, object_t* parent) : m_type(OBJECT_TYPE_STRING), m_parent(parent), m_name(name)
    {}

    object_t(int index, object_t* parent) : m_type(OBJECT_TYPE_ARRAY), m_parent(parent)
    {
        m_name = std::to_wstring(index);
    }

    ~object_t()
    {
        m_parent = nullptr;

        for (auto& kv : m_objects)
        {
            delete kv.second;
        }
        m_objects.clear();

        for (auto& kv : m_array_objects)
        {
            delete kv.second;
        }
        m_array_objects.clear();
    }

    object_t& operator[](const wchar_t* obj_name)
    {
        return get_obj(obj_name ? obj_name : L"");
    }

    object_t& operator[](const std::wstring& obj_name)
    {
        return get_obj(obj_name);
    }

    object_t& operator[](int index)
    {
        return get_obj(index);
    }

    template<class T>
    object_t& operator=(const T& t)
    {
        set_value(std::to_wstring(t).c_str());
        return *this;
    }

    object_t& operator=(const wchar_t* val)
    {
        set_value(val);
        return *this;
    }

    object_t& operator=(const std::wstring& val)
    {
        set_value(val.c_str());
        return *this;
    }

    object_t& operator=(const bool& b)
    {
        set_value(b ? L"TRUE" : L"FALSE");
        return *this;
    }

    operator std::wstring() const
    {
        return m_value;
    }

private:
    object_t(const object_t&) = delete;
    object_t& operator=(const object_t&) = delete;

    object_t& get_obj(const std::wstring& obj_name)
    {
        object_t* obj = nullptr;
        auto it = m_objects.find(obj_name);

        if (it != m_objects.end())
        {
            obj = it->second;
        }
        else
        {
            obj = new object_t(obj_name, this);
            if (!obj)
            {
                throw std::bad_alloc();
            }

            m_objects[obj_name] = obj;
        }

        return *obj;
    }

    object_t& get_obj(int index)
    {
        object_t* obj = nullptr;
        auto it = m_array_objects.find(index);
        if (it != m_array_objects.end())
        {
            obj = it->second;
        }
        else
        {
            obj = new object_t(index, this);
            if (!obj)
            {
                throw std::bad_alloc();
            }

            m_array_objects[index] = obj;
        }

        return *obj;
    }

    void set_value(const wchar_t* val)
    {
        m_value = val ? val : L"";
    }

    std::wstring obj_path() const
    {
        if (!m_parent)
        {
            return m_name;
        }

        auto path = m_parent->obj_path();
        if (m_type == OBJECT_TYPE_ARRAY)
        {
            path.push_back(L':');
        }
        else
        {
            path.push_back(L'.');
        }

        if (!m_name.empty())
        {
            path += m_name;
        }
        return path;
    }

    void to_wstream(std::wostream& os) const
    {
        if (!m_value.empty())
        {
            os << obj_path() << L" = \"" << m_value << L"\"\n";
        }

        for (auto& kv : m_objects)
        {
            kv.second->to_wstream(os);
        }

        for (auto& kv : m_array_objects)
        {
            kv.second->to_wstream(os);
        }
    }

private:
    static std::wstring trim_as_wstring(const wchar_t* buffer, size_t len)
    {
        std::wstring ret;
        if (buffer && len > 0)
        {
            if (len > BUFFER_LEN_MAX)
            {
                len = BUFFER_LEN_MAX;
            }

            // 找到第一个非空白字符
            size_t pos1 = 0;
            for (size_t i = 0; i < len; i++)
            {
                auto ch = buffer[i];
                if (ch != L' ' && ch != L'\n' && ch != L'\r')
                {
                    pos1 = i;
                    break;
                }
            }

            // 找到最后一个非空白字符
            size_t pos2 = 0;
            for (int i = (int)(len - 1); i >= 0; i--)
            {
                auto ch = buffer[i];
                if (ch != L' ' && ch != L'\n' && ch != L'\r')
                {
                    pos2 = i;
                    break;
                }
            }

            if (pos2 >= pos1)
            {
                ret.assign(&buffer[pos1], pos2 - pos1 + 1);
            }
        }

        return ret;
    }

    static bool trim_as_int(const wchar_t* buffer, size_t len, int& index)
    {
        auto name = trim_as_wstring(buffer, len);
        if (name.empty())
        {
            return false;
        }

        if (!iswdigit(name[0]))
        {
            return false;
        }

        index = _wtoi(name.c_str());
        return true;
    }

private:
    type_t m_type;
    object_t* m_parent;
    std::wstring m_name;
    std::wstring m_value;
    std::map<std::wstring, object_t*> m_objects;
    std::map<int, object_t*> m_array_objects;
}; // end of class object_t


class vmx_editor
{
    friend std::wostream& operator<<(std::wostream& out, const vmx_editor& rhs);

public:
    vmx_editor() {}
    ~vmx_editor() { clear(); }

    object_t& operator[](const wchar_t* obj_name)
    {
        return get_obj(obj_name ? obj_name : L"");
    }

    object_t& operator[](const std::wstring& obj_name)
    {
        return get_obj(obj_name);
    }

    bool set_value(const wchar_t* path, const wchar_t* value)
    {
        m_last_error.clear();

        auto obj = get_obj_by_path(path);
        if (obj)
        {
            obj->set_value(value);
            return true;
        }

        return false;
    }

    std::wstring to_wstring() const
    {
        std::wostringstream out;
        out << *this;
        return out.str();
    }

    bool from_wstring(const wchar_t* buffer, size_t len = -1)
    {
        m_last_error.clear();
        clear();

        if (!buffer)
        {
            m_last_error = L"无效的参数";
            return false;
        }

        if (-1 == len)
        {
            len = wcslen(buffer);
        }

        size_t start = 0;
        for (size_t i = 0; i < len; i++)
        {
            auto ch = buffer[i];
            if (ch == '\n')
            {
                if (!from_buffer_line(&buffer[start], i - start))
                {
                    clear();
                    return false;
                }

                start = i + 1;
            }
        }

        if (len > start)
        {
            if (!from_buffer_line(&buffer[start], len - start))
            {
                clear();
                return false;
            }
        }

        return true;
    }

    bool to_file(const wchar_t* filepath)
    {
        m_last_error.clear();

        if (filepath)
        {
            std::wofstream ofs(filepath);
            if (ofs.is_open())
            {
                ofs.imbue(std::locale("chs")); // 支持中文
                ofs << *this;
                ofs.close();
                return true;
            }
            else
            {
                m_last_error = L"文件打开失败";
            }
        }
        else
        {
            m_last_error = L"无效的参数";
        }
        return false;
    }

    bool from_file(const wchar_t* filepath)
    {
        m_last_error.clear();
        clear();

        if (!filepath)
        {
            m_last_error = L"无效的参数";
            return false;
        }

        std::wifstream ifs;
        ifs.open(filepath, std::ios_base::in);
        if (!ifs.is_open())
        {
            m_last_error = L"文件打开失败";
            return false;
        }

        ifs.imbue(std::locale("chs")); // 支持中文
        std::wstring line;
        while (!ifs.eof())
        {
            std::getline(ifs, line);
            if (!from_buffer_line(line.c_str(), line.length()))
            {
                return false;
            }
        }

        return true;
    }

    void clear()
    {
        for (auto& kv : m_objects)
        {
            delete kv.second;
        }
        m_objects.clear();
    }

    std::wstring last_error() const
    {
        return m_last_error;
    }

private:
    vmx_editor(const vmx_editor&) = delete;
    vmx_editor& operator=(const vmx_editor&) = delete;

    object_t& get_obj(const std::wstring& obj_name)
    {
        object_t* obj = nullptr;
        auto it = m_objects.find(obj_name);
        if (it != m_objects.end())
        {
            obj = it->second;
        }
        else
        {
            obj = new object_t(obj_name, nullptr);
            if (!obj)
            {
                throw std::bad_alloc();
            }

            m_objects[obj_name] = obj;
        }

        return *obj;
    }

    object_t* get_obj_by_path(const wchar_t* path)
    {
        if (!path)
        {
            auto& obj = get_obj(L"");
            return &obj;
        }

        auto len = wcslen(path);
        if (len <= 0)
        {
            auto& obj = get_obj(L"");
            return &obj;
        }

        return get_obj_by_path(path, len);
    }

    object_t* get_obj_by_path(const wchar_t* path, const size_t& len)
    {
        object_t* obj = nullptr;
        size_t start = 0;
        object_t::type_t next_type = object_t::OBJECT_TYPE_STRING;

        for (size_t i = 0; i < len; i++)
        {
            if (path[i] == L'.' || path[i] == L':')
            {
                if (object_t::OBJECT_TYPE_STRING == next_type)
                {
                    obj = get_sub_obj(obj, object_t::trim_as_wstring(&path[start], i - start));
                }
                else
                {
                    int obj_index = 0;
                    if (object_t::trim_as_int(&path[start], i - start, obj_index))
                    {
                        obj = get_sub_obj(obj, _wtoi(&path[start]));
                    }
                    else
                    {
                        m_last_error = L"冒号(':')后面跟的必须是数字";
                        return nullptr;
                    }
                }

                start = i + 1;
                if (path[i] == L'.')
                {
                    next_type = object_t::OBJECT_TYPE_STRING;
                }
                else
                {
                    next_type = object_t::OBJECT_TYPE_ARRAY;
                }
            }
            else
            {
                // noting to do
            }

            if (start >= len)
            {
                m_last_error = L"左值不能以'.'或':'结尾";
                return nullptr;
            }
        }

        if (len > start)
        {
            if (object_t::OBJECT_TYPE_STRING == next_type)
            {
                obj = get_sub_obj(obj, object_t::trim_as_wstring(&path[start], len - start));
            }
            else
            {
                obj = get_sub_obj(obj, _wtoi(&path[start]));
            }
        }

        return obj;
    }

    object_t* get_sub_obj(object_t* parent, const std::wstring& obj_name)
    {
        if (parent)
        {
            auto& obj = parent->get_obj(obj_name);
            return &obj;
        }
        else
        {
            auto& obj = get_obj(obj_name);
            return &obj;
        }
    }

    object_t* get_sub_obj(object_t* parent, int index)
    {
        if (parent)
        {
            auto& obj = parent->get_obj(index);
            return &obj;
        }
        else
        {
            // 根节点不支持直接使用数组,转换为字符串object
            auto& obj = get_obj(std::to_wstring(index));
            return &obj;
        }
    }

    bool from_buffer_line(const wchar_t* buffer, const size_t& len)
    {
        if (len <= 1) // empty or '\n'
        {
            return true;
        }

        // 找到等于号
        std::wstring path;
        std::wstring value;
        for (size_t i = 0; i < len; i++)
        {
            if (buffer[i] == L'=')
            {
                path = object_t::trim_as_wstring(buffer, i);
                value = parse_value(&buffer[i], len - i);
                break;
            }
        }

        if (!path.empty() && !value.empty())
        {
            return set_value(path.c_str(), value.c_str());
        }

        if (path.empty())
        {
            m_last_error = L"键值对中的键为空";
            return false;
        }

        return true;
    }

private:
    static std::wstring parse_value(const wchar_t* buffer, const size_t& len)
    {
        size_t p1 = -1;
        size_t p2 = -1;
        for (size_t i = 0; i < len; i++)
        {
            if (buffer[i] == L'\"')
            {
                if (-1 == p1)
                {
                    p1 = i + 1;
                }
                else
                {
                    p2 = i;
                    break;
                }
            }
        }

        std::wstring value;
        if (-1 != p1 && -1 != p2 && p1 < len && p2 < len)
        {
            value.assign(&buffer[p1], p2 - p1);
        }
        return value;
    }

private:
    std::map<std::wstring, object_t*> m_objects;
    std::wstring m_last_error;
}; // end of class vmx_editor


static std::wostream& operator<<(std::wostream& out, const object_t& rhs)
{
    if (!rhs.m_value.empty())
    {
        out << rhs.obj_path() << L" = \"" << rhs.m_value << L"\"\n";
    }

    for (auto& kv : rhs.m_objects)
    {
        if (kv.second)
        {
            out << *kv.second;
        }
    }

    for (auto& kv : rhs.m_array_objects)
    {
        if (kv.second)
        {
            out << *kv.second;
        }
    }

    return out;
}

static std::wostream& operator<<(std::wostream& out, const vmx_editor& rhs)
{
    // out.imbue(std::locale("chs")); // 支持中文
    for (auto& kv : rhs.m_objects)
    {
        if (kv.second)
        {
            out << *kv.second;
        }
    }
    return out;
}

} // end of namespace vmware
