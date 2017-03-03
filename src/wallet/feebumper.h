// Copyright (c) 2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_WALLET_FEEBUMPER_H
#define BITCOIN_WALLET_FEEBUMPER_H

#include <primitives/transaction.h>

class CWallet;
class uint256;

enum class BumpFeeResult
{
    OK,
    INVALID_ADDRESS_OR_KEY,
    INVALID_REQUEST,
    INVALID_PARAMETER,
    WALLET_ERROR,
    MISC_ERROR,
};

class CFeeBumper
{
public:
    CFeeBumper(const CWallet *pWalletIn, const uint256 txidIn, int newConfirmTarget, bool specifiedConfirmTarget, CAmount totalFee, bool newTxReplaceable);
    BumpFeeResult getResult() const { return currentResult; }
    const std::vector<std::string>& getErrors() const { return vErrors; }
    CAmount getOldFee() const { return nOldFee; }
    CAmount getNewFee() const { return nNewFee; }
    CMutableTransaction* getBumpedTxRef() { return &mtx; }
    uint256 getBumpedTxId() const { return bumpedTxid; }

    bool commit(CWallet *pWalletNonConst);

private:
    const uint256 txid;
    uint256 bumpedTxid;
    CMutableTransaction mtx;
    std::vector<std::string> vErrors;
    BumpFeeResult currentResult;
    CAmount nOldFee;
    CAmount nNewFee;
};

#endif // BITCOIN_WALLET_FEEBUMPER_H
