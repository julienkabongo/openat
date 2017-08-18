/* Copyright 2017 Paolo Galeone <nessuno@nerdz.eu>. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.*/

#ifndef AT_TYPE_H_
#define AT_TYPE_H_

#include <exception>
#include <json.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace at {

static inline void toupper(std::string& str)
{
    transform(str.begin(), str.end(), str.begin(), ::toupper);
}

// Remember:
// Functions included in multiple source files must be inline
// http://en.cppreference.com/w/cpp/language/inline
//
// Handle user defined types & json:
// https://github.com/nlohmann/json#basic-usage

using json = nlohmann::json;

// Returns a std::string if field is a string
// Otherwise returns the string "0.0"
// Throws runtime_error otherwise
inline std::string numeric_string(const json& field)
{
    if (field.is_string()) {
        return field.get<std::string>();
    }
    if (field.is_null()) {
        return std::string("0.0");
    }
    std::ostringstream stream;
    stream << "field " << field << " is not string or null";
    throw std::runtime_error(stream.str());
}

class currency_pair_t {
private:
    std::pair<std::string, std::string> _pair;

public:
    std::string first, second;
    currency_pair_t() {}
    currency_pair_t(const std::string& first, const std::string& second)
    {
        std::string ucFirst = first;
        std::string ucSecond = second;
        toupper(ucFirst);
        toupper(ucSecond);
        _pair = std::pair(ucFirst, ucSecond);
        this->first = _pair.first;
        this->second = _pair.second;
    }

    std::string str() const { return first + "_" + second; }
};

// overload of << between ostream and currency_pair_t
inline std::ostream& operator<<(std::ostream& o, const currency_pair_t& pair)
{
    return o << pair.str();
}

inline void to_json(json& j, const currency_pair_t c)
{
    j = json{c.first, c.second};
}
inline void from_json(const json& j, currency_pair_t& c)
{
    c.first = j.at(0).get<std::string>();
    c.second = j.at(1).get<std::string>();
}

typedef std::string hash_t;
inline void to_json(json& j, const hash_t& a) { j = json::parse(a); }
inline void from_json(const json& j, hash_t& a) { a = j.get<std::string>(); }

typedef struct {
    std::string name, symbol, status;
} coin_t;
inline void to_json(json& j, const coin_t& c)
{
    j = json{{"name", c.name}, {"symbol", c.symbol}, {"status", c.status}};
}
inline void from_json(const json& j, coin_t& c)
{
    c.name = j.at("name").get<std::string>();
    c.symbol = j.at("symbol").get<std::string>();
    c.status = j.at("status").get<std::string>();
}

typedef struct {
    double min;
    double max;
} deposit_limit_t;

inline void to_json(json& j, const deposit_limit_t& d)
{
    j = json{{"min", d.min}, {"max", d.max}};
}
inline void from_json(const json& j, deposit_limit_t& d)
{
    d.min = j.at("min").get<double>();
    d.max = j.at("max").get<double>();
}

typedef struct {
    deposit_limit_t limit;
    double fee;
    std::string currency;
    std::string method;
} deposit_info_t;

inline void to_json(json& j, const deposit_info_t& d)
{
    j = json{{"limit", d.limit},
             {"fee", d.fee},
             {"currency", d.currency},
             {"method", d.method}};
}
inline void from_json(const json& j, deposit_info_t& d)
{
    d.limit.max = j["limit"]["max"].get<double>();
    d.limit.min = j["limit"]["min"].get<double>();
    d.fee = j.at("fee").get<double>();
    d.currency = j.at("currency").get<std::string>();
    d.method = j.at("method").get<std::string>();
}

typedef struct {
    currency_pair_t pair;
    deposit_limit_t limit;
    double rate;
    double miner_fee;
} exchange_info_t;
inline void to_json(json& j, const exchange_info_t& m)
{
    j = json{{"pair", m.pair.str()},
             {"limit", m.limit},
             {"rate", m.rate},
             {"miner_fee", m.miner_fee}};
}
inline void from_json(const json& j, exchange_info_t& m)
{
    m.limit = j.at("limit").get<deposit_limit_t>();
    m.rate = j.at("rate").get<double>();
    m.miner_fee = j.at("miner_fee").get<double>();
    auto pair_str = j.at("pair").get<std::string>();
    std::size_t delimiter_it = pair_str.find_last_of("_");
    if (delimiter_it != std::string::npos) {
        m.pair = currency_pair_t(pair_str.substr(0, delimiter_it),
                                 pair_str.substr(delimiter_it + 1));
    }
}

typedef struct {
    currency_pair_t pair;
    deposit_limit_t limit;
    double maker_fee, taker_fee;
} market_info_t;
inline void to_json(json& j, const market_info_t& m)
{
    j = json{{"pair", m.pair.str()},
             {"limit", m.limit},
             {"taker_fee", m.taker_fee},
             {"maker_fee", m.maker_fee}};
}
inline void from_json(const json& j, market_info_t& m)
{
    m.limit = j.at("limit").get<deposit_limit_t>();
    m.maker_fee = j.at("maker_fee").get<double>();
    m.taker_fee = j.at("taker_fee").get<double>();
    auto pair_str = j.at("pair").get<std::string>();
    std::size_t delimiter_it = pair_str.find_last_of("_");
    if (delimiter_it != std::string::npos) {
        m.pair = currency_pair_t(pair_str.substr(0, delimiter_it),
                                 pair_str.substr(delimiter_it + 1));
    }
}

enum class status_t : char {
    no_deposists = 'a',
    initial,   // initial
    received,  // received
    complete,  // = success
    settled,   // settled
    pending,   // pending
    failed,    // failure
    partial,   // partial
    expired,   // experied
};

inline std::ostream& operator<<(std::ostream& o, const status_t& s)
{
    switch (s) {
        case status_t::no_deposists:
            return o << "no_deposists";
        case status_t::initial:
            return o << "initial";
        case status_t::received:
            return o << "received";
        case status_t::complete:
            return o << "complete";
        case status_t::settled:
            return o << "settled";
        case status_t::pending:
            return o << "pending";
        case status_t::failed:
            return o << "failed";
        case status_t::partial:
            return o << "partial";
        default:
            return o << "expired";
    }
}

typedef struct {
    double price;
    double amount;
    time_t time;
} quotation_t;

typedef struct {
    quotation_t bid;
    quotation_t ask;
} ticker_t;

typedef struct {
    std::string status;     // open, closed, cancelled
    std::string ordertype;  // limit & co
    std::string type;       // buy/sell
    currency_pair_t pair;
    time_t open;
    time_t close;
    double volume;
    double cost;
    double fee;
    double price;
} order_t;

}  // end namespace at

#endif  // AT_TYPE_H
