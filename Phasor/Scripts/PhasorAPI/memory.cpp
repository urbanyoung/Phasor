#include "memory.h"
#include "../phasor-lua.hpp"
#include <cstdint>
#include <array>
#include <Windows.h>

#undef min
#undef max

void readData(lua_State* L, void* dest, void* src, size_t nbytes) {
    DWORD oldProtect = 0;
    if (VirtualProtect(src, nbytes, PAGE_EXECUTE_READ, &oldProtect)) {
        memcpy(dest, src, nbytes);
        VirtualProtect(src, nbytes, oldProtect, &oldProtect);
    } else {
        auto f = boost::format("invalid read: %u bytes from 0x%08X") % nbytes % src;
        luaL_error(L, "%s", f.str().c_str());
    }
}

void writeData(lua_State* L, void* dest, const void* src, size_t nbytes) {
    DWORD oldProtect = 0;
    if (VirtualProtect(dest, nbytes, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy(dest, src, nbytes);
        VirtualProtect(dest, nbytes, oldProtect, &oldProtect);
    } else {
        auto f = boost::format("invalid write: %u bytes from 0x%08X") % nbytes % src;
        luaL_error(L, "%s", f.str().c_str());
    }
}

template <typename T>
bool checkLimits(double value, T& out) {
    static const T max_value = std::numeric_limits<T>::max();
    static const T min_value = std::numeric_limits<T>::lowest();
    if (value <= max_value && value >= min_value) {
        out = static_cast<T>(value);
        return true;
    } else {
        return false;
    }
}

size_t addBitOffset(size_t x, int& offset) {
    x += offset / 8;
    offset %= 8;
    return x;
}

// --------------------------------------------------------

int l_readbit(lua_State* L) {
    size_t address;
    int bitOffset;

    if (lua_gettop(L) == 3) {
        // legacy: three args mean address, address_offset, bit_offset
        int addrOffset;
        std::tie(address, addrOffset, bitOffset) = phlua::callback::getArguments<size_t, int, int>(L, __FUNCTION__);
        address += addrOffset;
    } else {
        std::tie(address, bitOffset) = phlua::callback::getArguments<size_t, int>(L, __FUNCTION__);
    }

    address = addBitOffset(address, bitOffset);

    std::uint8_t b;
    readData(L, &b, (void*)address, sizeof(b));
    bool bit = (b & (1 << bitOffset)) != 0;

    return phlua::callback::pushReturns(L, std::make_tuple(bit));
}

int l_writebit(lua_State* L) {
    size_t address;
    int bitOffset;
    bool value;

    if (lua_gettop(L) == 4) {
        int addrOffset;
        // legacy: four args mean address, address_offset, bit_offset, value
        std::tie(address, addrOffset, bitOffset, value) = phlua::callback::getArguments<size_t, int, int, bool>(L, __FUNCTION__);
        address += addrOffset;
    } else {
        std::tie(address, bitOffset, value) = phlua::callback::getArguments<size_t, int, bool>(L, __FUNCTION__);
    }
    
    address = addBitOffset(address, bitOffset);

    std::uint8_t b;
    readData(L, &b, (void*)address, sizeof(b));
    if (value) b |= 1 << bitOffset;
    else b &= ~(1 << bitOffset);
    writeData(L, (void*)address, &b, sizeof(b));

    return 0;
}

// --------------------------------------------------------

template <typename T>
int readNumber(lua_State* L, const char* f) {
    size_t address;
    boost::optional<size_t> offset; // backwards compatibility

    std::tie(address, offset) = phlua::callback::getArguments<size_t, decltype(offset)>(L, f);

    if (offset) address += *offset;

    T x;
    readData(L, &x, (void*)address, sizeof(T));

    return phlua::callback::pushReturns(L, std::make_tuple(x));
}

int l_readbyte(lua_State* L) {
    return readNumber<std::uint8_t>(L, __FUNCTION__);
}

int l_readchar(lua_State* L) {
    return readNumber<std::int8_t>(L, __FUNCTION__);
}

int l_readword(lua_State* L) {
    return readNumber<std::uint16_t>(L, __FUNCTION__);
}

int l_readshort(lua_State* L) {
    return readNumber<std::int16_t>(L, __FUNCTION__);
}

int l_readdword(lua_State* L) {
    return readNumber<std::uint32_t>(L, __FUNCTION__);
}

int l_readint(lua_State* L) {
    return readNumber<std::int32_t>(L, __FUNCTION__);
}

int l_readfloat(lua_State* L) {
    return readNumber<float>(L, __FUNCTION__);
}

int l_readdouble(lua_State* L) {
    return readNumber<double>(L, __FUNCTION__);
}

// --------------------------------------------------------

template <typename T>
int writeNumber(lua_State* L, const char* f) {
    size_t address;
    lua_Number value;

    if (lua_gettop(L) == 3) { // backwards compatibility
        // address, offset, value
        int offset;
        std::tie(address, offset, value) = phlua::callback::getArguments<size_t, int, lua_Number>(L, f);
        address += offset;
    } else {
        std::tie(address, value) = phlua::callback::getArguments<size_t, lua_Number>(L, f);
    }

    T x;
    if (!checkLimits<T>(value, x)) {
        auto f = boost::format("invalid write: value (%.0f) outside of type's range") % value;
        luaL_error(L, "%s", f.str().c_str());
    }

    writeData(L, (void*)address, &x, sizeof(T));
    return 0;
}

int l_writebyte(lua_State* L) {
    return writeNumber<std::uint8_t>(L, __FUNCTION__);
}

int l_writechar(lua_State* L) {
    return writeNumber<std::int8_t>(L, __FUNCTION__);
}

int l_writeword(lua_State* L) {
    return writeNumber<std::uint16_t>(L, __FUNCTION__);
}

int l_writeshort(lua_State* L) {
    return writeNumber<std::int16_t>(L, __FUNCTION__);
}

int l_writedword(lua_State* L) {
    return writeNumber<std::uint32_t>(L, __FUNCTION__);
}

int l_writeint(lua_State* L) {
    return writeNumber<std::int32_t>(L, __FUNCTION__);
}

int l_writefloat(lua_State* L) {
    return writeNumber<float>(L, __FUNCTION__);
}

int l_writedouble(lua_State* L) {
    return writeNumber<double>(L, __FUNCTION__);
}

// --------------------------------------------------------

static const size_t kMaxStringElements = 160;

template <typename T>
int readstring(lua_State* L, const char* f) {
    size_t address;
    boost::optional<unsigned int> length;
    std::array<T, kMaxStringElements+1> dest;

    std::tie(address, length) = phlua::callback::getArguments<size_t, decltype(length)>(L, f);

    size_t readLength = kMaxStringElements;
    if (length) {
        if (length > kMaxStringElements) {
            auto f = boost::format("can only read strings to a maximum size of %u characters") % kMaxStringElements;
            luaL_argerror(L, 2, f.str().c_str());
        }
        readLength = *length;
    }

    readData(L, dest.data(), (void*)address, readLength*sizeof(T));
    dest[readLength] = 0; // ensure it's null terminated

    return phlua::callback::pushReturns(L, std::make_tuple(dest.data()));
}

int l_readstring(lua_State* L) {
    return readstring<char>(L, __FUNCTION__);
}

int l_readwidestring(lua_State* L) {
    return readstring<wchar_t>(L, __FUNCTION__);
}

// --------------------------------------------------------

template <typename StrType>
int writestring(lua_State* L, const char* f) {
    typedef typename StrType::value_type char_type;
    size_t address;
    boost::optional<size_t> offset;
    StrType s;

    if (lua_gettop(L) == 3) { // backwards compatibility
        std::tie(address, offset, s) = phlua::callback::getArguments<size_t, decltype(offset), StrType>(L, f);
    } else {
        std::tie(address, s) = phlua::callback::getArguments<size_t, StrType>(L, f);
    }

    if (offset) address += *offset;

    size_t byteLength = s.size() * sizeof(char_type);
    writeData(L, (void*)address, s.c_str(), byteLength);
    char_type nullChar = 0;
    writeData(L, (void*)(address + byteLength), &nullChar, sizeof(nullChar));
    return 0;
}

int l_writestring(lua_State* L) {
    return writestring<std::string>(L, __FUNCTION__);
}

int l_writewidestring(lua_State* L) {
    return writestring<std::wstring>(L, __FUNCTION__);
}