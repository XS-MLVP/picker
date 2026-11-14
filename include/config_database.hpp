#pragma once
#include "json.hpp"
#include "xspcomm/detail/config_db.h"

namespace picker::config {
namespace db = xspcomm::detail::cfgdb;
inline db::Value encode(const nlohmann::json &j) {
    db::Value v;
    if (j.is_null()) return v;
    if (j.is_boolean()) v.kind = j.get<bool>() ? db::Value::True : db::Value::False;
    else if (j.is_number_unsigned()) v = db::Value::uint(j.get<uint64_t>());
    else if (j.is_number_integer()) v = db::Value::sint(j.get<int64_t>());
    else if (j.is_number_float()) { v.kind = db::Value::Real; double n = j.get<double>(); std::memcpy(&v.number, &n, 8); }
    else if (j.is_string()) v = db::Value::str(j.get<std::string>());
    else if (j.is_array()) { v.kind = db::Value::Array; for (const auto &x : j) v.array.push_back(encode(x)); }
    else { v.kind = db::Value::Object; for (const auto &[k, x] : j.items()) v.object.emplace(k, encode(x)); }
    return v;
}
inline nlohmann::json decode(const db::Value &v) {
    using V = db::Value;
    switch (v.kind) {
    case V::Null: return nullptr;
    case V::False: return false;
    case V::True: return true;
    case V::Unsigned: return v.number;
    case V::Signed: return int64_t(v.number);
    case V::Real: { double n; std::memcpy(&n, &v.number, 8); return n; }
    case V::String: return v.text;
    case V::Array: { auto j = nlohmann::json::array(); for (const auto &x : v.array) j.push_back(decode(x)); return j; }
    case V::Object: { auto j = nlohmann::json::object(); for (const auto &[k, x] : v.object) j[k] = decode(x); return j; }
    }
    throw std::invalid_argument("Unknown configuration value");
}
}
