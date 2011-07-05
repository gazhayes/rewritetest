// Copyright (c) 2009-2011 Satoshi Nakamoto & Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.

#include "headers.h"
#include "db.h"
#include "crypter.h"

std::vector<unsigned char> CKeyStore::GenerateNewKey()
{
    RandAddSeedPerfmon();
    CKey key;
    key.MakeNewKey();
    if (!AddKey(key))
        throw std::runtime_error("CKeyStore::GenerateNewKey() : AddKey failed");
    return key.GetPubKey();
}

bool CKeyStore::GetPubKey(const uint160 &hashAddress, std::vector<unsigned char> &vchPubKeyOut) const
{
    CKey key;
    if (!GetKey(hashAddress, key))
        return false;
    vchPubKeyOut = key.GetPubKey();
    return true;
}

bool CBasicKeyStore::AddKey(const CKey& key)
{
    CRITICAL_BLOCK(cs_KeyStore)
        mapKeys[Hash160(key.GetPubKey())] = key.GetSecret();
    return true;
}

std::vector<unsigned char> CCryptoKeyStore::GenerateNewKey()
{
    RandAddSeedPerfmon();
    CKey key;
    key.MakeNewKey();
    if (!AddKey(key))
        throw std::runtime_error("CCryptoKeyStore::GenerateNewKey() : AddKey failed");
    return key.GetPubKey();
}

bool CCryptoKeyStore::Unlock(const CKeyingMaterial& vMasterKeyIn)
{
    CRITICAL_BLOCK(cs_vMasterKey)
    {
        if (!SetCrypted())
            return false;

        CryptedKeyMap::const_iterator mi = mapCryptedKeys.begin();
        for (; mi != mapCryptedKeys.end(); ++mi)
        {
            const std::vector<unsigned char> &vchPubKey = (*mi).second.first;
            const std::vector<unsigned char> &vchCryptedSecret = (*mi).second.second;
            CSecret vchSecret;
            if(!DecryptSecret(vMasterKeyIn, vchCryptedSecret, Hash(vchPubKey.begin(), vchPubKey.end()), vchSecret))
                return false;
            CKey key;
            key.SetSecret(vchSecret);
            if (key.GetPubKey() == vchPubKey)
                break;
            return false;
        }
        vMasterKey = vMasterKeyIn;
    }
    return true;
}

bool CCryptoKeyStore::AddKey(const CKey& key)
{
    CRITICAL_BLOCK(cs_KeyStore)
    CRITICAL_BLOCK(cs_vMasterKey)
    {
        if (!IsCrypted())
            return CBasicKeyStore::AddKey(key);

        if (IsLocked())
            return false;

        std::vector<unsigned char> vchCryptedSecret;
        std::vector<unsigned char> vchPubKey = key.GetPubKey();
        if (!EncryptSecret(vMasterKey, key.GetSecret(), Hash(vchPubKey.begin(), vchPubKey.end()), vchCryptedSecret))
            return false;

        if (!AddCryptedKey(key.GetPubKey(), vchCryptedSecret))
            return false;
    }
    return true;
}


bool CCryptoKeyStore::AddCryptedKey(const std::vector<unsigned char> &vchPubKey, const std::vector<unsigned char> &vchCryptedSecret)
{
    CRITICAL_BLOCK(cs_KeyStore)
    {
        if (!SetCrypted())
            return false;

        mapCryptedKeys[Hash160(vchPubKey)] = make_pair(vchPubKey, vchCryptedSecret);
    }
    return true;
}

bool CCryptoKeyStore::GetKey(const uint160 &hashAddress, CKey& keyOut) const
{
    CRITICAL_BLOCK(cs_vMasterKey)
    {
        if (!IsCrypted())
            return CBasicKeyStore::GetKey(hashAddress, keyOut);

        CryptedKeyMap::const_iterator mi = mapCryptedKeys.find(hashAddress);
        if (mi != mapCryptedKeys.end())
        {
            const std::vector<unsigned char> &vchPubKey = (*mi).second.first;
            const std::vector<unsigned char> &vchCryptedSecret = (*mi).second.second;
            CSecret vchSecret;
            if (!DecryptSecret(vMasterKey, vchCryptedSecret, Hash(vchPubKey.begin(), vchPubKey.end()), vchSecret))
                return false;
            keyOut.SetSecret(vchSecret);
            return true;
        }
    }
    return false;
}

bool CCryptoKeyStore::GetPubKey(const uint160 &hashAddress, std::vector<unsigned char>& vchPubKeyOut) const
{
    CRITICAL_BLOCK(cs_vMasterKey)
    {
        if (!IsCrypted())
            return CKeyStore::GetPubKey(hashAddress, vchPubKeyOut);

        CryptedKeyMap::const_iterator mi = mapCryptedKeys.find(hashAddress);
        if (mi != mapCryptedKeys.end())
        {
            vchPubKeyOut = (*mi).second.first;
            return true;
        }
    }
    return false;
}

bool CCryptoKeyStore::EncryptKeys(CKeyingMaterial& vMasterKeyIn)
{
    CRITICAL_BLOCK(cs_KeyStore)
    CRITICAL_BLOCK(cs_vMasterKey)
    {
        if (!mapCryptedKeys.empty() || IsCrypted())
            return false;

        fUseCrypto = true;
        CKey key;
        BOOST_FOREACH(KeyMap::value_type& mKey, mapKeys)
        {
            if (!key.SetPrivKey(mKey.second))
                return false;
            const std::vector<unsigned char> vchPubKey = key.GetPubKey();
            std::vector<unsigned char> vchCryptedSecret;
            if (!EncryptSecret(vMasterKeyIn, key.GetSecret(), Hash(vchPubKey.begin(), vchPubKey.end()), vchCryptedSecret))
                return false;
            if (!AddCryptedKey(vchPubKey, vchCryptedSecret))
                return false;
        }
        mapKeys.clear();
    }
    return true;
}
