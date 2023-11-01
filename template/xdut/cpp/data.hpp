#pragma once
#include <bits/stdc++.h>
#include <svdpi.h>

enum class IOType
{
    Input,
    Output,
    InOut,
};

#define bit32_one(tar, msk) tar = msk | tar
#define bit32_zro(tar, msk) tar = (~msk) & tar
#define bit32_val(tar, msk) (msk & tar) == 0 ? 0 : 1
#define bit32_msk(ones) (1 << ones) - 1
#define bit32_set(tar, idx) tar = (tar) | (1 << idx)
#define bit32_hex(tar, idx, val) tar = ((~(0xF << idx * 4)) & (tar)) | (val << idx * 4)
#define bit32_chr(tar, idx, val) tar = ((~(0xFF << idx * 8)) & (tar)) | (val << idx * 8)

class PinBind
{
private:
    int index = -1;
    uint32_t mask = 0;
    svLogicVecVal *pVec = nullptr;
    svLogic *pLgc = nullptr;

public:
    std::function<void()> write_fc = nullptr;
    PinBind(svLogicVecVal *p, int index);
    PinBind(svLogic *p);
    PinBind &operator=(const u_int8_t &v);
    operator u_int8_t();

    // template functions
    template <typename T>
    bool operator==(const T data)
    {
        return (T) * this == data;
    }
    template <typename T>
    PinBind &operator=(const T &v)
    {
        return this->operator=((u_int8_t)v);
    }
    template <typename T>
    operator T()
    {
        return (T)this->operator u_int8_t();
    }
};

class XData;
struct CallBack
{
    std::string desc;
    std::function<void(bool, XData *, u_int32_t, void *)> fc = nullptr;
    void *args = nullptr;
};
class XData
{
public:
    static const IOType In = IOType::Input;
    static const IOType Out = IOType::Output;
    static const IOType InOut = IOType::InOut;

private:
    std::vector<CallBack> call_back_on_change;
    // if (mWidth == 1) {
    std::function<void(svLogic *)> bitRead = nullptr;
    std::function<void(svLogic)> bitWrite = nullptr;
    // } else {
    std::function<void(svLogicVecVal *)> vecRead = nullptr;
    std::function<void(svLogicVecVal *)> vecWrite = nullptr;
    // }
    PinBind pinbind_bit;
    PinBind **pinbind_vec = nullptr;
    uint32_t ubuff[2];
    uint32_t xbuff[2];
    uint64_t xdata = 0;
    uint64_t udata = 0;
    uint32_t vecSize = 0;
    uint32_t zero_mask = -1;
    svLogicVecVal *__pVecData = nullptr; // 01ZX  -> 00,01,10,11
    svLogic __mLogicData = 0;            // 01ZX  -> 00,01,10,11
    bool igore_callback = false;
    bool udata_is_valid = false;

private:
    void update_read();
    void update_write();
    void _sv_to_local();
    void _zero_sv();
    void _trunc_sv();
    void _local_to_sv();
    void _dpi_read();
    void _dpi_write();
    void _update_shadow();
    void _dpi_check();

public:
    // basic
    std::string mName;
    IOType mIOType;
    uint32_t mWidth;
    // DPI                                         ba
    svLogicVecVal *pVecData = nullptr; // 01ZX  -> 00,01,10,11
    svLogic mLogicData = 0;            // 01ZX  -> 00,01,10,11
public:
    XData();
    void ReInit(uint32_t width, IOType itype, std::string name = "");
    XData(uint32_t width, IOType itype, std::string name = "");
    XData(XData &t);
    ~XData();
    bool DataValid();
    void BindDPIRW(std::function<void(svLogicVecVal *)> read, std::function<void(svLogicVecVal *)> write);
    void BindDPIRW(std::function<void(svLogic *)> read, std::function<void(svLogic)> write);
    void SetBits(u_int8_t *buffer, int count, u_int8_t *mask = nullptr, int start = 0);
    void SetBits(u_int32_t *buffer, u_int32_t count, u_int32_t *mask = nullptr, u_int32_t start = 0);
    bool GetBits(u_int32_t *buffer, u_int32_t count);
    void OnChange(std::function<void(bool, XData *, u_int64_t, void *)> func, void *args = nullptr, std::string desc = "");
    void ReadFresh();
    uint32_t W();
    uint64_t U();
    int64_t S();
    bool B();
    std::string String();
    bool Connect(XData &xdata);
    bool operator==(XData &data);
    bool operator==(u_int64_t data);
    XData &operator=(XData &data);
    XData &operator=(const char *str);
    XData &operator=(std::string &data);
    XData &operator=(u_int64_t data);
    XData &AsBiIO();
    XData &Flip();
    XData &Invert();
    PinBind &operator[](u_int32_t index);
    operator std::string();
    operator u_int64_t();

    // template functions
    template <typename T>
    XData(T value) : XData(sizeof(T) * 8, IOType::Input)
    {
        this->operator=(value);
    }
    template <typename T>
    bool operator==(T data)
    {
        return this->operator==((u_int64_t)data);
    }
    template <typename T>
    XData &operator=(T data)
    {
        return this->operator=((u_int64_t)data);
    }
    template <typename T>
    operator T()
    {
        return (T)(this->operator u_int64_t());
    }
};
