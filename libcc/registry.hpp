// LibCC ~ Carl Corcoran, https://github.com/thenfour/LibCC

#pragma once

#ifdef WIN32

#include "Winapi.hpp"
#include <vector>

namespace LibCC
{
  /*
      Takes a string reg key name and attempts to convert it to a real HKEY.
      This is only for the registry hive names HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, etc...
      s can have stuff after it too (e.g. "HKEY_LOCAL_MACHINE\\Software")
      Shortened versions also recognized (e.g "HKLM" instead of "HKEY_LOCAL_MACHINE")

      returns 0 for failure (unrecognized key name)
  */
  template<typename Char>
  HKEY InterpretRegHiveName(const std::basic_string<Char>& s, std::basic_string<Char>& therest)
  {
    typedef std::basic_string<Char> String;
    HKEY r = 0;
    String sKey;
    String::size_type nSep = StringFindFirstOf(s, "\\/");
    if(nSep == String::npos)
    {
        // no separator... maybe the whole thing is the hive name
        sKey = s;
        therest.clear();
    }
    else
    {
        sKey = s.substr(0, nSep);
        therest = s.substr(nSep + 1);// BUG: could potentially go past the end of the str.
    }

    NaiveStringToLower(sKey);

    if(StringEquals(sKey, "hklm") || StringEquals(sKey, "hkey_local_machine"))
    {
        r = HKEY_LOCAL_MACHINE;
    }
    else if(StringEquals(sKey, "hkcu") || StringEquals(sKey, "hkey_current_user"))
    {
        r = HKEY_CURRENT_USER;
    }
    else if(StringEquals(sKey, "hkcr") || StringEquals(sKey, "hkey_classes_root"))
    {
        r = HKEY_CLASSES_ROOT;
    }
    else if(StringEquals(sKey, "hku") || StringEquals(sKey, "hkey_users"))
    {
        r = HKEY_CLASSES_ROOT;
    }

    return r;
  }

  /*
    The idea is that a RegistryKey can be either existing or not, recursively.
    a registrykey is really just a string that describes the location.
    here are use-cases:

    RegistryKey key("HKLM\\Software\\Mashboxx");
    if(key.Exists())
    {
      RegistryValueIterator it = key.EnumKeys();
      for(it.Begin(); !it.End(); it.Next())
      {
        if(it->Exists())
        {
          RegistryValue& v(*it);
          v[] = "";
          string x = v.GetString();
          v.GetStringValue(x);
          v.GetName();
          v.Delete();      
        }
      }
    }
    else
    {
      key.Create();
      key.CreateChild()
      key.Delete();
    }

    template<typename Char, typename Traits, typename Allocator>
    class RegistryKeyX
    {
    public:
      RegistryKeyX(HKEY key, const _String& subkey, bool bWriteAccess = false);
      RegistryKeyX(const _String& key, bool bWriteAccess = false);
      RegistryKeyX(const _RegistryKey& rhs);
      _RegistryKey& operator =(const _RegistryKey& rhs);
      _RegistryKey& Assign(const _RegistryKey& rhs);

      ~RegistryKeyX();

      bool Delete();
      bool Exists();
      bool Create();

      _RegistryKey SubKey(const _String& subkey);

      bool SetValue(const _String& name, int value);
      bool SetValue(const _String& name, DWORD value);
      bool SetValue(const _String& name, const void* Buffer, DWORD Length);
      bool GetValueEx(const _String& name, void* buf, IN OUT DWORD& size);
      bool GetValue(const _String& name, void* buf, IN DWORD size);
      bool GetValue(const _String& name, _String& value);
      bool SetValue(const _String& name, const _String& value);
      template<typename el, bool l, typename tr>
      bool GetValue(const _String& name, Blob<el, l, tr>& buf, DWORD& size) const;

      SubKeyIterator EnumSubKeysBegin();
      SubKeyIterator EnumSubKeysEnd();

    private:
      void __Close();
      bool __IsSufficientlyOpen() const;
      bool __Open();
    };

  */
  template<typename Char>
  class RegistryKeyX
  {
  public:
    typedef Char _Char;
    typedef std::basic_string<_Char> _String;
    typedef RegistryKeyX<_Char> _RegistryKey;

    // x = RegistryKey(HKEY_LOCAL_MACHINE, "software\\crap")
    RegistryKeyX(HKEY key, const _String& subkey, bool bWriteAccess, REGSAM additionalAccessFlags) :
	    m_hKey(0),
      m_bOpenedForWrite(false),
      m_bNeedWriteAccess(bWriteAccess),
			m_additionalAccessFlags(additionalAccessFlags)
    {
      m_hRoot = key;
      m_subKey = subkey;
    }

    // x = RegistryKey(HKEY_LOCAL_MACHINE, "software\\crap")
    RegistryKeyX(HKEY key, const _String& subkey, bool bWriteAccess = false) :
	    m_hKey(0),
      m_bOpenedForWrite(false),
      m_bNeedWriteAccess(bWriteAccess),
			m_additionalAccessFlags(0)
    {
      m_hRoot = key;
      m_subKey = subkey;
    }

    // x = RegistryKey("hklm\\software\\crap");
    RegistryKeyX(const _String& key, bool bWriteAccess = false) :
	    m_hKey(0),
      m_bOpenedForWrite(false),
      m_bNeedWriteAccess(bWriteAccess),
			m_additionalAccessFlags(0)
    {
	    m_hRoot = InterpretRegHiveName(key, m_subKey);
    }

    // x = RegistryKey(Y);
    RegistryKeyX(const _RegistryKey& rhs) :
	    m_hKey(0),
      m_bNeedWriteAccess(false),
      m_bOpenedForWrite(false),
      m_hRoot(0),
			m_additionalAccessFlags(0)
    {
      Assign(rhs);
    }

    _RegistryKey& operator =(const _RegistryKey& rhs)
    {
      return Assign(rhs);
    }

    _RegistryKey& Assign(const _RegistryKey& rhs)
    {
	    __Close();
      m_bNeedWriteAccess = rhs.m_bNeedWriteAccess;
			m_additionalAccessFlags = rhs.m_additionalAccessFlags;
      m_hRoot = rhs.m_hRoot;
      m_subKey = rhs.m_subKey;
      return *this;
    }

    ~RegistryKeyX()
    {
      __Close();
    }

    bool Delete(int& gle)
    {
      // delete recursively
      for(SubKeyIterator it = EnumSubKeysBegin(); it != EnumSubKeysEnd(); ++ it)
      {
        if(!it->Delete(gle))
					return false;
      }
      __Close();
      bool ret = ERROR_SUCCESS == RegDeleteKeyX(m_hRoot, m_subKey.c_str());
			gle = GetLastError();
			return ret;
    }

    bool Delete()
    {
			int gle = 0;
			return Delete(gle);
    }

    bool Exists()
    {
      return __Open();
    }

    bool Create()
    {
      if(m_hKey) return false;
	    REGSAM access = m_bNeedWriteAccess ? KEY_WRITE : 0;
      access |= KEY_QUERY_VALUE | KEY_READ;
			access |= m_additionalAccessFlags;
      if(ERROR_SUCCESS != RegCreateKeyExX(m_hRoot, m_subKey.c_str(), 0, access, &m_hKey, 0))
      {
        return false;
      }
      if((access & KEY_WRITE) == KEY_WRITE)
      {
        m_bOpenedForWrite = true;
      }
      return true;
    }

    _RegistryKey SubKey(const _String& subkey)
    {
      _String subkeyCopy(m_subKey);
      subkeyCopy = PathAppendX(subkeyCopy, subkey);
      return _RegistryKey(m_hRoot, subkeyCopy, m_bNeedWriteAccess, m_additionalAccessFlags);
    }

    class Value
    {
    public:
      Value(_RegistryKey& parent, const _String& name) :
        m_key(parent),
        m_name(name)
      {
      }

      Value(const Value& rhs) :
        m_key(rhs.m_key),
        m_name(rhs.m_name)
      {
      }

      Value& operator =(const Value& rhs)
      {
        m_key = rhs.m_key;
        m_name = rhs.m_name;
        return *this;
      }

      bool Exists() { return m_key.ValueExists(m_name); }
      bool Delete() { return m_key.DeleteValue(m_name); }

      bool SetValue(bool value) { return m_key.SetValue(m_name, value); }
      bool SetValue(int value) { return m_key.SetValue(m_name, value); }
      bool SetValue(DWORD value) { return m_key.SetValue(m_name, value); }
      bool SetValue(const _String& value) { return m_key.SetValue(m_name, value); }
      bool operator =(int value) { return SetValue(value); }
      bool operator =(bool value) { return SetValue(value); }
      bool operator =(DWORD value) { return SetValue(value); }
      bool operator =(unsigned int value) { return SetValue(static_cast<DWORD>(value)); }
      bool operator =(const _String& value) { return SetValue(value); }

      bool GetValue(bool& value) { return m_key.GetValue(m_name, value); }
      bool GetValue(int& value) { return m_key.GetValue(m_name, value); }
      bool GetValue(DWORD& value) { return m_key.GetValue(m_name, value); }
      bool GetValue(_String& value) { return m_key.GetValue(m_name, value); }
      int GetInt() { int ret; GetValue(ret); return ret; }
      DWORD GetDWORD(){ DWORD ret; GetValue(ret); return ret; }
      _String GetString() { _String ret; GetValue(ret); return ret; }

      bool SetValue(const void* Buffer, DWORD Length) { return m_key.SetValue(m_name, Buffer, Length); }
      bool GetValueEx(void* buf, IN OUT DWORD& size) { return m_key.GetValueEx(m_name, buf, size); }
      bool GetValue(void* buf, IN DWORD size) { return m_key.GetValue(m_name, buf, size); }
	    template<typename BlobEl>
	    bool GetValue(std::vector<BlobEl>& buf, DWORD& size) { return m_key.GetValue(m_name, buf, size); }
	    template<typename BlobEl>
	    bool GetValue(std::vector<BlobEl>& buf) { return m_key.GetValue(m_name, buf); }

      _String m_name;
      _RegistryKey& m_key;
    };

    bool ValueExists(const _String& name)
    {
      if(!__Open()) return false;
      DWORD type;// just temp
      return ERROR_SUCCESS == RegQueryValueExX(m_hKey, name.length() ? name.c_str() : 0, &type, 0, 0);
    }

    bool DeleteValue(const _String& name, int& gle)
    {
      m_bNeedWriteAccess = true;
      if(!__Open(gle)) return false;
      bool ret = ERROR_SUCCESS == RegDeleteValueX(m_hKey, name.length() ? name.c_str() : 0);
			gle = GetLastError();
			return ret;
    }

    bool DeleteValue(const _String& name)
    {
			int gle;
			return DeleteValue(name, gle);
    }

    Value GetValue(const _String& name)
    {
      return Value(*this, name);
    }

    Value operator [] (const _String& name)
    {
      return Value(*this, name);
    }

    // set string / dword / int
    //bool SetValue(const _String& name, bool value)// having this function is prone to faulty implicit casts to bool.
    //{
    //  DWORD temp = value ? 1 : 0;
    //  return SetValue(name, temp);
    //}
    bool SetValue(const _String& name, int value)
    {
      m_bNeedWriteAccess = true;
      if(!__Open()) return false;
      return (ERROR_SUCCESS == RegSetValueExX(m_hKey, name.length() ? name.c_str() : 0, REG_DWORD, reinterpret_cast<BYTE*>(&value), sizeof(DWORD)));
    }
    bool SetValue(const _String& name, DWORD value)
    {
      m_bNeedWriteAccess = true;
      if(!__Open()) return false;
      return (ERROR_SUCCESS == RegSetValueExX(m_hKey, name.length() ? name.c_str() : 0, REG_DWORD, reinterpret_cast<BYTE*>(&value), sizeof(DWORD)));
    }
    bool SetValueGle(const _String& name, const _String& value, int& gle)
    {
      m_bNeedWriteAccess = true;
      if(!__Open(gle))
				return false;
	    bool ret = (ERROR_SUCCESS == RegSetValueExStringX(m_hKey, name.length() ? name.c_str() : 0, 0, REG_SZ, value.c_str()));
			gle = GetLastError();
			return ret;
    }
		bool SetValue(const _String& name, const _String& value)
		{
			int gle;
			return SetValueGle(name, value, gle);
		}

    // get string / dword / int
    bool GetValue(const _String& name, bool& val)
    {
      DWORD temp;
      if(!GetValue(name, temp)) return false;
      val = (temp == 1);
      return true;
    }
    bool GetValue(const _String& name, int& val)
    {
      if(!__Open()) return false;
      DWORD size = sizeof(val);
	    return (ERROR_SUCCESS == RegQueryValueExX(m_hKey, name.length() ? name.c_str() : 0, 0, reinterpret_cast<BYTE*>(&val), &size));
    }
    bool GetValue(const _String& name, DWORD& val, int& gle)
    {
			gle = ERROR_SUCCESS;
      if(!__Open(gle)) return false;
      DWORD size = sizeof(val);
	    bool ret = (ERROR_SUCCESS == RegQueryValueExX(m_hKey, name.length() ? name.c_str() : 0, 0, reinterpret_cast<BYTE*>(&val), &size));
			gle = GetLastError();
			return ret;
    }
    bool GetValue(const _String& name, DWORD& val)
    {
			int gle;
			return GetValue(name, val, gle);
    }
    bool GetValue(const _String& name, _String& value)
    {
      if(!__Open()) return false;
	    return (ERROR_SUCCESS == RegQueryValueExStringX(m_hKey, name.length() ? name.c_str() : 0, 0, value));
    }

    // set binary
    bool SetValue(const _String& name, const void* Buffer, DWORD Length)
    {
      m_bNeedWriteAccess = true;
      if(!__Open()) return false;
      return (ERROR_SUCCESS == RegSetValueExX(m_hKey, name.length() ? name.c_str() : 0, REG_BINARY, reinterpret_cast<const BYTE*>(Buffer), Length));
    }

    // get binary
    bool GetValueEx(const _String& name, void* buf, IN OUT DWORD& size)
    {
      if(!__Open()) return false;
	    return (ERROR_SUCCESS == RegQueryValueExX(m_hKey, name.length() ? name.c_str() : 0, 0, reinterpret_cast<BYTE*>(buf), &size));
    }
    bool GetValue(const _String& name, void* buf, IN DWORD size)
    {
	    return GetValueEx(name, buf, size);
    }
	  template<typename BlobEl>
	  bool GetValue(const _String& name, std::vector<BlobEl>& buf, DWORD& size)
	  {
		  bool r = false;
      if(!__Open()) return false;
		  // get the size
		  if(ERROR_SUCCESS == RegQueryValueExX(m_hKey, name.length() ? name.c_str() : 0, 0, 0, &size))
		  {
			  buf.resize(size + 1);
			  if(ERROR_SUCCESS == RegQueryValueExX(m_hKey, name.length() ? name.c_str() : 0, 0, reinterpret_cast<BYTE*>(buf.data()), &size))
			  {
				  r = true;
			  }
		  }
		  return r;
	  }
	  template<typename BlobEl>
	  bool GetValue(const _String& name, std::vector<BlobEl>& buf)
	  {
      DWORD size;
	    return GetValue(name, buf, size);
    }

    // SUB-KEY ENUMERATION
    typedef typename std::vector<_RegistryKey>::iterator SubKeyIterator;

    SubKeyIterator EnumSubKeysBegin()
    {
      if(!__Open())
      {
        return m_subKeys.begin();
      }

      // only populate subkeys if it hasnt been done yet.
      if(0 == m_subKeys.size())
      {
        _String subkey;
        _String justKey;
        DWORD maxNameSize = 0;
        DWORD i = 0;
        while(ERROR_NO_MORE_ITEMS != RegEnumKeyExX(m_hKey, i, justKey, maxNameSize))
        {
          subkey = m_subKey;
          subkey = PathAppendX(subkey, justKey);
          m_subKeys.push_back(_RegistryKey(m_hRoot, subkey, m_bNeedWriteAccess, m_additionalAccessFlags));
          i ++;
        }
      }
      return m_subKeys.begin();
    }
    SubKeyIterator EnumSubKeysEnd()
    {
      return m_subKeys.end();
    }

  private:

    void __Close()
    {
      m_subKeys.clear();

      if(m_hKey)
      {
        RegCloseKey(m_hKey);
        m_hKey = 0;
        m_bOpenedForWrite = false;
      }
    }

    bool __IsSufficientlyOpen() const
    {
      if(!m_hKey) return false;
      if(m_bNeedWriteAccess && !m_bOpenedForWrite) return false;
      return true;
    }

    bool __Open(int& gle)
    {
      if(__IsSufficientlyOpen())
      {
				gle = ERROR_SUCCESS;
        return true;
      }
      __Close();
	    REGSAM access = m_bNeedWriteAccess ? KEY_WRITE : 0;
      access |= KEY_QUERY_VALUE | KEY_READ;
			access |= m_additionalAccessFlags;
		  if(ERROR_SUCCESS != RegOpenKeyExX(m_hRoot, m_subKey.c_str(), 0, access, &m_hKey))
      {
				gle = GetLastError();
        return false;
      }
      if((access & KEY_WRITE) == KEY_WRITE)
      {
        m_bOpenedForWrite = true;
      }
			gle = ERROR_SUCCESS;
      return true;
    }

    bool __Open()
		{
			int gle;
      return __Open(gle);
    }

	  HKEY m_hKey;// Null when the key's not opened.
    bool m_bOpenedForWrite;

    // these are from the constructor
    _String m_subKey;
    bool m_bNeedWriteAccess;
		REGSAM m_additionalAccessFlags;
    HKEY m_hRoot;

    std::vector<_RegistryKey> m_subKeys;
  };
  typedef RegistryKeyX<wchar_t> RegistryKeyW;
  typedef RegistryKeyX<char> RegistryKeyA;
  typedef RegistryKeyX<TCHAR> RegistryKey;
}


#endif
