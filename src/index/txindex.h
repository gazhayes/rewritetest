// Copyright (c) 2017-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_INDEX_TXINDEX_H
#define BITCOIN_INDEX_TXINDEX_H

#include <index/base.h>

/**
 * TxIndex is used to look up transactions included in the blockchain by hash.
 * The index is written to a LevelDB database and records the filesystem
 * location of each transaction by transaction hash.
 */
class TxIndex final : public BaseIndex
{
private:
    const std::unique_ptr<TxIndexDB> m_db;

protected:
    /// Override base class init to migrate from old database.
    bool Init() override;

    bool WriteBlock(const CBlock& block, const CBlockIndex* pindex) override;

    BaseIndexDB& GetDB() const override;

    const char* GetName() const override { return "txindex"; }

public:
    /// Constructs the index, which becomes available to be queried.
    explicit TxIndex(std::unique_ptr<TxIndexDB> db);

    /// Look up a transaction by hash.
    ///
    /// @param[in]   tx_hash  The hash of the transaction to be returned.
    /// @param[out]  block_hash  The hash of the block the transaction is found in.
    /// @param[out]  tx  The transaction itself.
    /// @return  true if transaction is found, false otherwise
    bool FindTx(const uint256& tx_hash, uint256& block_hash, CTransactionRef& tx) const;
};

/// The global transaction index, used in GetTransaction. May be null.
extern std::unique_ptr<TxIndex> g_txindex;

#endif // BITCOIN_INDEX_TXINDEX_H
