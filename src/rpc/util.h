// Copyright (c) 2017-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_RPC_UTIL_H
#define BITCOIN_RPC_UTIL_H

#include <node/transaction.h>
#include <pubkey.h>
#include <rpc/protocol.h>
#include <script/standard.h>
#include <univalue.h>

#include <string>
#include <vector>

#include <boost/variant.hpp>

class CKeyStore;
class CPubKey;
class CScript;
struct InitInterfaces;

//! Pointers to interfaces that need to be accessible from RPC methods. Due to
//! limitations of the RPC framework, there's currently no direct way to pass in
//! state to RPC method implementations.
extern InitInterfaces* g_rpc_interfaces;

CPubKey HexToPubKey(const std::string& hex_in);
CPubKey AddrToPubKey(CKeyStore* const keystore, const std::string& addr_in);
CScript CreateMultisigRedeemscript(const int required, const std::vector<CPubKey>& pubkeys);

UniValue DescribeAddress(const CTxDestination& dest);

//! Parse a confirm target option and raise an RPC error if it is invalid.
unsigned int ParseConfirmTarget(const UniValue& value);

RPCErrorCode RPCErrorFromTransactionError(TransactionError terr);
UniValue JSONRPCTransactionError(TransactionError terr, const std::string& err_string = "");

struct RPCArg {
    enum class Type {
        OBJ,
        ARR,
        STR,
        NUM,
        BOOL,
        OBJ_USER_KEYS, //!< Special type where the user must set the keys e.g. to define multiple addresses; as opposed to e.g. an options object where the keys are predefined
        AMOUNT,        //!< Special type representing a floating point amount (can be either NUM or STR)
        STR_HEX,       //!< Special type that is a STR with only hex chars
    };

    enum class Optional {
        /** Required arg */
        NO,
        /**
         * Optinal arg that is a named argument and has a default value of
         * `null`. When possible, the default value should be specified.
         */
        OMITTED_NAMED_ARG,
        /**
         * Optional argument with default value omitted because they are
         * implicitly clear. That is, elements in an array or object may not
         * exist by default.
         * When possible, the default value should be specified.
         */
        OMITTED,
    };
    using Fallback = boost::variant<Optional, /* default value for optional args */ std::string>;
    const std::string m_name; //!< The name of the arg (can be empty for inner args)
    const Type m_type;
    const std::vector<RPCArg> m_inner; //!< Only used for arrays or dicts
    const Fallback m_fallback;
    const std::string m_description;
    const std::string m_oneline_description; //!< Should be empty unless it is supposed to override the auto-generated summary line
    const std::vector<std::string> m_type_str; //!< Should be empty unless it is supposed to override the auto-generated type strings. Vector length is either 0 or 2, m_type_str.at(0) will override the type of the value in a key-value pair, m_type_str.at(1) will override the type in the argument description.

    RPCArg(
        const std::string& name,
        const Type& type,
        const Fallback& fallback,
        const std::string& description,
        const std::string& oneline_description = "",
        const std::vector<std::string>& type_str = {})
        : m_name{name},
          m_type{type},
          m_fallback{fallback},
          m_description{description},
          m_oneline_description{oneline_description},
          m_type_str{type_str}
    {
        assert(type != Type::ARR && type != Type::OBJ);
    }

    RPCArg(
        const std::string& name,
        const Type& type,
        const Fallback& fallback,
        const std::string& description,
        const std::vector<RPCArg>& inner,
        const std::string& oneline_description = "",
        const std::vector<std::string>& type_str = {})
        : m_name{name},
          m_type{type},
          m_inner{inner},
          m_fallback{fallback},
          m_description{description},
          m_oneline_description{oneline_description},
          m_type_str{type_str}
    {
        assert(type == Type::ARR || type == Type::OBJ);
    }

    /**
     * Return the type string of the argument.
     * Set oneline to allow it to be overridden by a custom oneline type string (m_oneline_description).
     */
    std::string ToString(bool oneline) const;
    /**
     * Return the type string of the argument when it is in an object (dict).
     * Set oneline to get the oneline representation (less whitespace)
     */
    std::string ToStringObj(bool oneline) const;
    /**
     * Return the description string, including the argument type and whether
     * the argument is required.
     */
    std::string ToDescriptionString() const;
};

struct RPCResult {
    const std::string m_cond;
    const std::string m_result;

    explicit RPCResult(std::string result)
        : m_cond{}, m_result{std::move(result)}
    {
        assert(!m_result.empty());
    }

    RPCResult(std::string cond, std::string result)
        : m_cond{std::move(cond)}, m_result{std::move(result)}
    {
        assert(!m_cond.empty());
        assert(!m_result.empty());
    }
};

struct RPCResults {
    const std::vector<RPCResult> m_results;

    RPCResults()
        : m_results{}
    {
    }

    RPCResults(RPCResult result)
        : m_results{{result}}
    {
    }

    RPCResults(std::initializer_list<RPCResult> results)
        : m_results{results}
    {
    }

    /**
     * Return the description string.
     */
    std::string ToDescriptionString() const;
};

struct RPCExamples {
    const std::string m_examples;
    RPCExamples(
        std::string examples)
        : m_examples(std::move(examples))
    {
    }
    std::string ToDescriptionString() const;
};

class RPCHelpMan
{
public:
    RPCHelpMan(std::string name, std::string description, std::vector<RPCArg> args, RPCResults results, RPCExamples examples);

    std::string ToString() const;

private:
    const std::string m_name;
    const std::string m_description;
    const std::vector<RPCArg> m_args;
    const RPCResults m_results;
    const RPCExamples m_examples;
};

#endif // BITCOIN_RPC_UTIL_H
