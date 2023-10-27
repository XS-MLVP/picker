
#include "data.hpp"
#include "util.hpp"

namespace dut
{
    LogLevel log_level = LogLevel::debug;
    LogLevel get_log_level()
    {
        return log_level;
    }
    void set_log_level(LogLevel val)
    {
        log_level = val;
    }

    // class PinBind
    PinBind::PinBind(svLogicVecVal *p, int index)
    {
        this->pVec = p;
        this->index = index / 32;
        this->mask = 1 << index % 32;
    }
    PinBind::PinBind(svLogic *p)
    {
        this->pLgc = p;
        this->index = -1;
    }

    PinBind &PinBind::operator=(const u_int8_t &v)
    {
        auto val = (u_int8_t)v;
        Assert(val >= 0 && val <= 3, "Set value[%d(to uint8)] must in rang [0,1,2,3] 01ZX", val);
        if (this->pLgc)
        {
            *this->pLgc = val;
        }
        else if (this->pVec)
        {
            switch (val)
            {
            case 0:
                bit32_zro(this->pVec[this->index].aval, this->mask);
                bit32_zro(this->pVec[this->index].bval, this->mask);
                break;
            case 1:
                bit32_one(this->pVec[this->index].aval, this->mask);
                bit32_zro(this->pVec[this->index].bval, this->mask);
                break;
            case 2:
                bit32_zro(this->pVec[this->index].aval, this->mask);
                bit32_one(this->pVec[this->index].bval, this->mask);
                break;
            case 3:
                bit32_one(this->pVec[this->index].aval, this->mask);
                bit32_one(this->pVec[this->index].bval, this->mask);
                break;
            default:
                Assert(false, "Set value[%d] must in rang [0,1,2,3] 01ZX", val);
                break;
            }
        }
        else
        {
            Assert(false, "Pin is not bind to any svLogic data");
        }
        // write to dpi
        if (this->write_fc)
            this->write_fc();
        return *this;
    }

    PinBind::operator u_int8_t()
    {
        if (this->pLgc)
        {
            return static_cast<u_int8_t>(*this->pLgc);
        }
        else if (this->pVec)
        {
            uint8_t aval = bit32_val(this->pVec[this->index].aval, this->mask);
            uint8_t bval = bit32_val(this->pVec[this->index].bval, this->mask);
            return bval * 2 + aval;
        }
        else
        {
            Assert(false, "PinType error");
        }
    }

    // class XData::
    void XData::update_read()
    {
        this->_dpi_read();
        this->_sv_to_local();
        this->_update_shadow();
    }
    void XData::update_write()
    {
        this->_local_to_sv();
        this->_dpi_write();
        this->_update_shadow();
    }
    bool XData::DataValid()
    {
        return this->udata_is_valid;
    }
    void XData::_sv_to_local()
    {
        this->_trunc_sv();
        if (this->mWidth == 0)
        {
            Assert(this->mLogicData >= 0 && this->mLogicData < 4, "Logic data[%d] need in range [0,1,2,3]", this->mLogicData);
            this->udata = this->mLogicData;
            this->xdata = this->udata > 1 ? 1 : 0;
            this->udata_is_valid = this->udata > 1 ? false : true;
        }
        else
        {
            // reset buff
            this->ubuff[0] = 0;
            this->ubuff[1] = 0;
            this->xbuff[0] = 0;
            this->xbuff[1] = 0;
            auto range = std::min(2, (int)this->vecSize); // max xdata.length is 64 (2 vecData)
            for (int i = 0; i < range; i++)
            {
                this->ubuff[i] = this->pVecData[i].aval;
                this->xbuff[i] = this->pVecData[i].bval;
            }
            this->udata = *(uint64_t *)this->ubuff;
            this->xdata = *(uint64_t *)this->xbuff;
            if (this->xdata != 0)
            {
                this->udata_is_valid = false;
            }
            else
            {
                this->udata_is_valid = true;
            }
        }
    }
    void XData::_zero_sv()
    {
        for (int i = 0; i < this->vecSize; i++)
        {
            this->pVecData[i].aval = 0;
            this->pVecData[i].bval = 0;
        }
    }
    void XData::_trunc_sv()
    {
        if (this->mWidth == 0)
            return;
        this->pVecData[this->vecSize - 1].aval &= this->zero_mask;
        this->pVecData[this->vecSize - 1].bval &= this->zero_mask;
    }
    void XData::_local_to_sv()
    {
        if (this->mWidth == 0)
        {
            Assert(this->udata >= 0 && this->udata < 4, "Logic data to write need in range [0,1,2,3]");
            this->mLogicData = this->udata;
        }
        else
        {
            // reset buff
            *(uint64_t *)this->ubuff = this->udata;
            auto range = std::min(2, (int)this->vecSize); // max xdata.length is 64 (2 vecData)
            for (int i = 0; i < this->vecSize; i++)
            {
                // assign lower 64 bits
                if (i < 2)
                {
                    this->pVecData[i].aval = this->ubuff[i];
                    // assign higher bits zero
                }
                else
                {
                    this->pVecData[i].aval = 0;
                }
                this->pVecData[i].bval = 0;
            }
        }
        this->_sv_to_local();
    }
    void XData::_dpi_read()
    {
        this->_dpi_check();
        if (this->bitRead)
            this->bitRead(&this->mLogicData);
        if (this->vecRead)
            this->vecRead(this->pVecData);
    }
    void XData::_dpi_write()
    {
        if (this->mIOType == IOType::Output)
            return;
        this->_dpi_check();

        // if(this->bitWrite || this->vecWrite){
        //     Debug("%s._dpi_write = %ld", this->mName.c_str(), this->udata);
        // }

        if (this->bitWrite)
            this->bitWrite(this->mLogicData);
        if (this->vecWrite)
            this->vecWrite(this->pVecData);
    }
    void XData::_update_shadow()
    {
        if (this->igore_callback)
            return;
        bool need_call = false;
        bool validate = true;
        if (this->mWidth == 0)
        {
            need_call = (this->__mLogicData != this->mLogicData);
            validate = this->mLogicData < 2;
        }
        else
        {
            for (int i = 0; i < this->vecSize; i++)
            {
                if (this->__pVecData[i].aval != this->pVecData[i].aval)
                {
                    need_call = true;
                }
                if (this->__pVecData[i].bval != this->pVecData[i].bval)
                {
                    need_call = true;
                }
                if (this->pVecData->bval)
                {
                    validate = false;
                }
                this->__pVecData[i] = this->pVecData[i];
            }
        }
        this->__mLogicData = this->mLogicData;
        this->igore_callback = true;
        if (need_call)
        {
            for (auto fc : this->call_back_on_change)
            {
                fc.fc(validate, this, this->udata, fc.args);
            }
        }
        this->igore_callback = false;
    }
    void XData::_dpi_check(){
        // TBD
    };

    XData::XData() : XData(0, IOType::InOut){};
    XData::XData(uint32_t width, IOType itype, std::string name) : mWidth(width), mIOType(itype), pinbind_bit(&this->mLogicData), mName(name)
    {
        this->ReInit(width, itype, name);
    }
    void XData::ReInit(uint32_t width, IOType itype, std::string name)
    {
        this->mWidth = width;
        this->mIOType = itype;
        this->mName = name;

        auto write_fc = [this]()
        {this->_dpi_write(); this->_sv_to_local(); this->_update_shadow(); };
        if (width > 1)
        {
            // svDPI Vec is 32 bits
            this->vecSize = width / 32 + ((width % 32 != 0) ? 1 : 0);
            this->pVecData = (svLogicVecVal *)calloc(this->vecSize, sizeof(svLogicVecVal));
            this->__pVecData = (svLogicVecVal *)calloc(this->vecSize, sizeof(svLogicVecVal));
            this->pinbind_vec = (PinBind **)calloc(this->mWidth, sizeof(PinBind *));
            for (int i = 0; i < this->mWidth; i++)
            {
                this->pinbind_vec[i] = new PinBind(this->pVecData, i);
                this->pinbind_vec[i]->write_fc = write_fc;
            }
            this->zero_mask = bit32_msk(width % 32); // eg: [bin] ,|1111,1000,0000,0000| ...
            if (this->zero_mask == 0)
            {
                this->zero_mask = -1; // set: 0xFFFFFFFF
            }
        }
        else
        {
            this->pinbind_bit.write_fc = write_fc;
            this->vecSize = 0;
        }
        this->mLogicData = 0;
        this->udata_is_valid = true;
    }
    XData::XData(XData &t) : mWidth(t.mWidth), mIOType(t.mIOType), pinbind_bit(&this->mLogicData), mLogicData(t.mLogicData)
    {
        auto write_fc = [this]()
        {this->_dpi_write(); this->_sv_to_local(); this->_update_shadow(); };
        t.update_read();
        // members
        this->xdata = t.xdata;
        this->udata = t.udata;
        this->vecSize = t.vecSize;
        this->zero_mask = t.zero_mask;
        // pointers
        if (this->vecSize > 0)
        {
            this->pVecData = (svLogicVecVal *)calloc(this->vecSize, sizeof(svLogicVecVal));
            this->__pVecData = (svLogicVecVal *)calloc(this->vecSize, sizeof(svLogicVecVal));
            this->pinbind_vec = (PinBind **)calloc(this->mWidth, sizeof(PinBind *));
            for (int i = 0; i < this->mWidth; i++)
            {
                this->pinbind_vec[i] = new PinBind(this->pVecData, i);
                this->pinbind_vec[i]->write_fc = write_fc;
            }
        }
        for (int i = 0; i < this->vecSize; i++)
        {
            this->pVecData[i] = t.pVecData[i];
        }
        this->pinbind_bit.write_fc = write_fc;
        this->_sv_to_local();
    };
    XData::~XData()
    {
        if (this->pVecData)
        {
            for (int i = 0; i < this->mWidth; i++)
            {
                delete this->pinbind_vec[i];
            }
            delete this->pinbind_vec;
            delete this->pVecData;
            delete this->__pVecData;
            this->pVecData = nullptr;
        }
    }
    void XData::BindDPIRW(std::function<void(svLogicVecVal *)> read, std::function<void(svLogicVecVal *)> write)
    {
        Assert(this->mWidth > 0, "XData(name=%s).mWidth(%d) need > 0", this->mName.c_str(), this->mWidth);
        this->vecRead = read;
        this->vecWrite = write;
        this->update_read();
    }
    void XData::BindDPIRW(std::function<void(svLogic *)> read, std::function<void(svLogic)> write)
    {
        Assert(this->mWidth == 0, "XData(name=%s).mWidth(%d) need == 0", this->mName.c_str(), this->mWidth);
        this->bitRead = read;
        this->bitWrite = write;
        this->update_read();
    }
    void XData::SetBits(u_int8_t *buffer, int count, u_int8_t *mask, int start)
    {
        Assert(this->mWidth > 0, "Only svVec support SetBits");
        int index = start;
        for (int i = 0; i < count; i++)
        {
            auto vec_idx = index / 4;
            auto vec_off = index % 4;
            if (vec_idx >= this->vecSize)
                break;
            uint8_t byte_val = buffer[i];
            if (mask)
            { // FIXME: need refine
                u_int8_t old = (u_int8_t)((this->pVecData[vec_idx].aval >> vec_off * 4) & 0x000000FF);
                byte_val = (old & (~mask[i])) | (buffer[i] & mask[i]);
            }
            bit32_chr(this->pVecData[vec_idx].aval, vec_off, byte_val);
            bit32_chr(this->pVecData[vec_idx].bval, vec_off, 0);
            index += 1;
        }
        this->_dpi_write();
        this->_sv_to_local();
        this->_update_shadow();
    }
    void XData::SetBits(u_int32_t *buffer, u_int32_t count, u_int32_t *mask, u_int32_t start)
    {
        Assert(this->mWidth > 0, "only svVec support SetBits");
        auto range = std::min(count, this->vecSize - start);
        for (int i = 0; i < range; i++)
        {
            if (mask == nullptr)
            {
                this->pVecData[i + start].aval = buffer[i];
            }
            else
            {
                u_int32_t aval = this->pVecData[i + start].aval & (~mask[i]);
                this->pVecData[i + start].aval = aval | (buffer[i] & mask[i]);
            }
            this->pVecData[i + start].bval = 0;
        }
        this->_dpi_write();
        this->_sv_to_local();
        this->_update_shadow();
    }
    bool XData::GetBits(u_int32_t *buffer, u_int32_t count)
    {
        Assert(this->mWidth > 0, "only svVec support GetBits");
        this->update_read();
        auto range = std::min(count, this->vecSize);
        bool ret = true;
        for (int i = 0; i < range; i++)
        {
            buffer[i] = this->pVecData[i].aval;
            if (this->pVecData[i].bval != 0)
            {
                ret = false;
            }
        }
        return ret;
    }
    void XData::OnChange(std::function<void(bool, XData *, u_int64_t, void *)> func, void *args, std::string desc)
    {
        CallBack cb;
        cb.args = args;
        cb.fc = func;
        cb.desc = desc;
        this->call_back_on_change.push_back(cb);
    }
    uint32_t XData::W() { return this->mWidth; }
    uint64_t XData::U()
    {
        this->update_read();
        return static_cast<uint64_t>(this->udata);
    }
    int64_t XData::S()
    {
        this->update_read();
        return static_cast<int64_t>(this->udata);
    }
    bool XData::B()
    {
        this->update_read();
        return static_cast<bool>(this->udata);
    }
    std::string XData::String()
    {
        this->update_read();
        if (this->mWidth == 0)
        {
            switch (mLogicData)
            {
            case 0:
                return "0";
                break;
            case 1:
                return "1";
                break;
            case 2:
                return "Z";
                break;
            case 3:
                return "X";
                break;
            default:;
            }
            Assert(false, "Logic Data value error(%d), need range (0 -> 3)", this->mLogicData);
        }
        // svLogicVecVal
        std::string ret;
        u_int8_t abuffer[4]; // 32 bit buffer
        u_int8_t bbuffer[4]; // 32 bit buffer
        for (int i = 0; i < this->vecSize; i++)
        {
            *(u_int32_t *)abuffer = this->pVecData[i].aval;
            *(u_int32_t *)bbuffer = this->pVecData[i].bval;
            for (int j = 0; j < 4; j++)
            {
                if (bbuffer[j] == 0)
                {
                    ret += dut::sFmt("%02x", abuffer[j]);
                }
                else
                {
                    ret += "??";
                }
                if (i * 32 + j * 8 > this->mWidth)
                    break;
            }
        }
        return ret;
    }

    bool XData::operator==(u_int64_t data)
    {
        this->update_read();
        if (this->mWidth > 64)
        {
            for (int i = 2; i < this->vecSize; i++)
            {
                if (this->pVecData[i].aval != 0 || this->pVecData[i].bval != 0)
                {
                    return false;
                }
            }
        }
        return this->udata == data && this->xdata == 0;
    }

    XData &XData::operator=(u_int64_t data)
    {
        Assert(this->mIOType != IOType::Output, "Can not assign value to ouput.PIN(name=%s)",
               this->mName.c_str());
        this->udata = data;
        this->xdata = 0;
        this->update_write();
        return *this;
    }

    XData &XData::operator=(XData &data)
    {
        data.update_read();
        Assert(this->mIOType != IOType::Output, "Can not assign value to ouput.PIN");
        Assert(this->mWidth == data.mWidth, "Need left.mWidth(%d) == right.mWidth(%d)", this->mWidth, data.mWidth);
        // raw
        for (int i = 0; i < this->vecSize; i++)
        {
            this->pVecData[i] = data.pVecData[i];
        }
        this->udata_is_valid = data.udata_is_valid;
        this->mLogicData = data.mLogicData;
        // update
        this->_sv_to_local();
        this->_dpi_write();
        this->_update_shadow();
        return *this;
    }
    bool XData::operator==(XData &data)
    {
        // update value
        this->update_read();
        data.update_read();
        if (this->mWidth < 64 && data.mWidth < 64)
        {
            return (this->udata == data.udata) && (this->xdata == data.xdata);
        }
        // logic compare
        auto lgc_cmp_vec = [](XData *lgc, XData *vec) -> bool
        {
            for (int i = 1; i < vec->vecSize; i++)
            {
                if (vec->pVecData[i].aval != 0)
                {
                    return false;
                }
            }
            if (vec->pVecData[0].aval > 1 | vec->pVecData[0].bval > 1)
            {
                return false;
            }
            return (vec->pVecData->aval + 2 * vec->pVecData->bval) == lgc->mLogicData;
        };
        if (this->mWidth == 0)
        {
            if (data.mWidth == 0)
            {
                return this->mLogicData == data.mLogicData;
            }
            return lgc_cmp_vec(this, &data);
        }
        else
        {
            if (data.mWidth == 0)
                return lgc_cmp_vec(&data, this);
            // vec compare
            XData *ldata = (this->vecSize > data.vecSize) ? this : &data;
            uint32_t idx_vec = std::min(this->vecSize, data.vecSize);
            for (int i = idx_vec; i < ldata->vecSize; i++)
            {
                if (ldata->pVecData[i].aval != 0 || ldata->pVecData[i].bval != 0)
                {
                    return false;
                }
            }
            for (int i = 0; i < idx_vec; i++)
            {
                if ((this->pVecData[i].aval != data.pVecData[i].aval) || (this->pVecData[i].bval != data.pVecData[i].bval))
                {
                    return false;
                }
            }
            return true;
        }
    }

    XData &XData::operator=(const char *str)
    {
        std::string raw_str = str;
        return this->operator=(raw_str);
    }
    XData &XData::operator=(std::string &data)
    {
        auto prefix = data.substr(0, 2);
        Assert(this->mIOType != IOType::Output, "Can not assign value to ouput.PIN");
        Assert(data.length() > 2 && contians(std::vector<std::string>{"0b", "0x", "::"},
                                             prefix),
               "Input string needs start with: 0b (binary), 0x (hex) or :: (str)");
        this->_zero_sv();

        auto txt = data.substr(2);
        int tsz = txt.length();
        int index = 0;
        if (prefix == "0b")
        {
            for (int i = 0; i < tsz; i++)
            {
                if (txt[i] == '_')
                    continue;
                Assert(txt[i] == '0' || txt[i] == '1', "find no 0/1 (%c) value: %s(%d) at %d", txt[i], txt.c_str(), tsz, i);
                auto vec_idx = index / 32;
                auto vec_off = index % 32;
                if (vec_idx >= this->vecSize)
                    break;
                if (txt[i] == '1')
                {
                    bit32_set(this->pVecData[vec_idx].aval, vec_off);
                }
                index += 1;
            }
        }
        else if (prefix == "0x")
        {
            txt = sLower(txt);
            for (int i = 0; i < tsz; i++)
            {
                if (txt[i] == '_')
                    continue;
                u_int32_t hex_val = (u_int32_t)txt[i] - 48;
                if (hex_val > 9)
                {
                    hex_val = (u_int32_t)txt[i] - 97 + 10;
                }
                Assert(hex_val >= 0 && hex_val <= 15, "find no hex(%c) value: %s(%d) at %d", txt[i], txt.c_str(), tsz, i);
                auto vec_idx = index / 8;
                auto vec_off = index % 8;
                if (vec_idx >= this->vecSize)
                    break;
                bit32_hex(this->pVecData[vec_idx].aval, vec_off, hex_val);
                index += 1;
            }
        }
        else if (prefix == "::")
        {
            for (int i = 0; i < tsz; i++)
            {
                if (txt[i] == '_')
                    continue;
                u_int32_t chr_val = (u_int32_t)txt[i];
                auto vec_idx = index / 4;
                auto vec_off = index % 4;
                if (vec_idx >= this->vecSize)
                    break;
                bit32_chr(this->pVecData[vec_idx].aval, vec_off, chr_val);
                index += 1;
            }
        }
        this->_dpi_write();
        this->_sv_to_local();
        this->_update_shadow();
        return *this;
    }

    void XData::ReadFresh()
    {
        this->update_read();
    }

    XData &XData::Flip()
    {
        if (this->mIOType == IOType::Input)
        {
            this->mIOType = IOType::Output;
        }
        else if (this->mIOType == IOType::Output)
        {
            this->mIOType = IOType::Input;
        }
        return *this;
    }

    XData &XData::AsBiIO()
    {
        this->mIOType = IOType::InOut;
        return *this;
    }

    XData &XData::Invert()
    {
        if (this->mWidth == 0)
        {
            this->mLogicData = this->mLogicData == 0 ? 1 : 0;
        }
        else
        {
            for (int i = 0; i < this->vecSize; i++)
            {
                this->pVecData[i].aval = ~this->pVecData[i].aval;
                this->pVecData[i].bval = 0;
            }
        }
        this->_dpi_write();
        this->_sv_to_local();
        this->_update_shadow();
        return *this;
    }
    XData::operator u_int64_t()
    {
        this->update_read();
        return this->udata;
    }
    XData::operator std::string()
    {
        return this->mName;
    }
    PinBind &XData::operator[](u_int32_t index)
    {
        this->update_read();
        Assert(index < this->mWidth, "Index[%d] need range 0 t %d", index, this->mWidth);
        if (this->mWidth > 0)
        {
            return *this->pinbind_vec[index];
        }
        return this->pinbind_bit;
    }

    bool XData::Connect(XData &xdata)
    {
        auto a_drive_b = [](XData &a, XData &b) -> bool
        {
            if (a.mIOType == IOType::Input)
            {
                Error("Master IO type cannot be IOType::Input");
                return false;
            }
            if (b.mIOType == IOType::Output)
            {
                Error("Slave IO type cannot be IOType::Output");
                return false;
            }
            std::string desc = "connet_" + a.mName + "_to_" + b.mName;
            a.OnChange([](bool valid, XData *x, u_int32_t val, void *args)
                       {
                            if(valid){
                                auto y = ((XData*)args);
                                 y->mLogicData = x->mLogicData;
                                for (int i = 0; i < x->vecSize; i++){
                                    y->pVecData[i] = x->pVecData[i];
                                }
                                y->_sv_to_local();
                                y->_dpi_write();
                                y->_update_shadow();
                            } },
                       &b, desc);
            return true;
        };
        // same type
        // if(this->mIOType == IOType::InOut || xdata.mIOType == IOType::InOut) Warn("connect InOut PINs, it may cause error!");
        if (this->mIOType == xdata.mIOType)
        {
            if (this->mIOType == IOType::InOut)
            {
                xdata = *this;
                return a_drive_b(*this, xdata);
            }
            Error("Can not connect same IOType[%s] %s <-> %s",
                  this->mIOType == IOType::Input ? "IOType::Input" : "IOType::Output",
                  this->mName.c_str(), xdata.mName.c_str());
            return false;
        }
        // diff type
        switch (this->mIOType)
        {
        case IOType::Input:
            *this = xdata;
            return a_drive_b(xdata, *this);
            break;
        case IOType::Output:
            xdata = *this;
            return a_drive_b(*this, xdata);
        case IOType::InOut:
            if (xdata.mIOType == IOType::Input)
            {
                xdata = *this;
                return a_drive_b(*this, xdata);
            }
            else
            {
                *this = xdata;
                return a_drive_b(xdata, *this);
            };
            break;
        default:
            Assert(false, "Connect %s -> %s fail!", this->mName.c_str(), xdata.mName.c_str());
        }
    }
}