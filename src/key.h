// Copyright (c) 2009-2015 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin developers
// Copyright (c) 2015 The FlyCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_KEY_H
#define BITCOIN_KEY_H

#include <boost/variant.hpp>

#include <stdexcept>
#include <vector>

#include "allocators.h"
#include "serialize.h"
#include "uint256.h"
#include "util.h"

#include <openssl/ec.h> // for EC_KEY definition

// secp160k1
// const unsigned int PRIVATE_KEY_SIZE = 192;
// const unsigned int PUBLIC_KEY_SIZE  = 41;
// const unsigned int SIGNATURE_SIZE   = 48;
//
// secp192k1
// const unsigned int PRIVATE_KEY_SIZE = 222;
// const unsigned int PUBLIC_KEY_SIZE  = 49;
// const unsigned int SIGNATURE_SIZE   = 57;
//
// secp224k1
// const unsigned int PRIVATE_KEY_SIZE = 250;
// const unsigned int PUBLIC_KEY_SIZE  = 57;
// const unsigned int SIGNATURE_SIZE   = 66;
//
// secp256k1:
// const unsigned int PRIVATE_KEY_SIZE = 279;
// const unsigned int PUBLIC_KEY_SIZE  = 65;
// const unsigned int SIGNATURE_SIZE   = 72;
//
// see www.keylength.com
// script supports up to 75 for single byte push

class key_error : public std::runtime_error
{
public:
    explicit key_error(const std::string& str) : std::runtime_error(str) {}
};

/** A reference to a CKey: the Hash160 of its serialized public key */
class CKeyID : public uint160
{
public:
    CKeyID() : uint160(0) { }
    CKeyID(const uint160 &in) : uint160(in) { }
};

/** A reference to a CKeyExchange: the Hash160 of its serialized public key */
/** This was the best solution to differentiating an Exchange key in the
 * map address book from a regular address key as well as changing the int type- Griffith */
class CKeyExchangeID : public uint512
{
public:
    CKeyExchangeID() : uint512(0) { }
    CKeyExchangeID(const uint512 &in) : uint512(in) { }
};


/** A reference to a CScript: the Hash160 of its serialization (see script.h) */
class CScriptID : public uint160
{
public:
    CScriptID() : uint160(0) { }
    CScriptID(const uint160 &in) : uint160(in) { }
};

/** An encapsulated public key. */
class CPubKey
{
private:
    std::vector<unsigned char> vchPubKey;
    friend class CKey;

public:
    CPubKey(){}
    CPubKey(const std::vector<unsigned char> &vchPubKeyIn)
    {
        vchPubKey = vchPubKeyIn;
    }
    CKeyID GetID() const
    {
        return CKeyID(Hash160(vchPubKey));
    }
    friend bool operator==(const CPubKey &a, const CPubKey &b) { return a.vchPubKey == b.vchPubKey; }
    friend bool operator!=(const CPubKey &a, const CPubKey &b) { return a.vchPubKey != b.vchPubKey; }
    friend bool operator<(const CPubKey &a, const CPubKey &b) { return a.vchPubKey < b.vchPubKey; }

    IMPLEMENT_SERIALIZE(
        READWRITE(vchPubKey);
    )

    uint256 GetHash() const
    {
        return Hash(vchPubKey.begin(), vchPubKey.end());
    }

    bool IsValid() const
    {
        return vchPubKey.size() == 33 || vchPubKey.size() == 65;
    }

    bool IsCompressed() const
    {
        return vchPubKey.size() == 33;
    }

    std::vector<unsigned char> Raw() const
    {
        return vchPubKey;
    }
};

class CPubKeyExchange
{
private:
    std::vector<unsigned char> vchPubKey;
    friend class CKeyExchange;

public:
    CPubKeyExchange(){}
    CPubKeyExchange(const std::vector<unsigned char> &vchPubKeyIn)
    {
         vchPubKey = vchPubKeyIn;
    }
    CKeyExchangeID GetID() const
    {
        return CKeyExchangeID(Hash512(vchPubKey));
    }
    friend bool operator==(const CPubKeyExchange &a, const CPubKeyExchange &b) { return a.vchPubKey == b.vchPubKey; }
    friend bool operator!=(const CPubKeyExchange &a, const CPubKeyExchange &b) { return a.vchPubKey != b.vchPubKey; }
    friend bool operator<(const CPubKeyExchange &a, const CPubKeyExchange &b) { return a.vchPubKey < b.vchPubKey; }

    IMPLEMENT_SERIALIZE(
        READWRITE(vchPubKey);
    )

    uint256 GetHash() const
    {
        return Hash(vchPubKey.begin(), vchPubKey.end());
    }

    bool IsValid() const
    {
        return vchPubKey.size() == 66;
    }

    bool IsCompressed() const
    {
        return vchPubKey.size() == 66;
    }

    std::vector<unsigned char> Raw() const
    {
        return vchPubKey;
    }
};

// secure_allocator is defined in allocators.h
// CPrivKey is a serialized private key, with all parameters included (279 bytes)
typedef std::vector<unsigned char, secure_allocator<unsigned char> > CPrivKey;
// CSecret is a serialization of just the secret parameter (32 bytes)
typedef std::vector<unsigned char, secure_allocator<unsigned char> > CSecret;


/** An encapsulated OpenSSL Elliptic Curve key (public and/or private) */
class CKey
{
protected:
    const bool exchangekey = false;
    EC_KEY* pkey;
    bool fSet;
    bool fCompressedPubKey;
    void SetCompressedPubKey();

public:

    void Reset();

    CKey();
    CKey(const CKey& b);
    CPubKey GetPubKey() const;

    ~CKey();

    CKey& operator=(const CKey& b);


    bool IsNull() const;
    bool IsCompressed() const;

    void MakeNewKey(bool fCompressed);
    bool SetPrivKey(const CPrivKey& vchPrivKey);
    bool SetSecret(const CSecret& vchSecret, bool fCompressed = false);
    CSecret GetSecret(bool &fCompressed) const;
    CPrivKey GetPrivKey() const;
    bool SetPubKey(const CPubKey& vchPubKey);

    bool Sign(uint256 hash, std::vector<unsigned char>& vchSig);

    // reconstruct public key from a compact signature
    // This is only slightly more CPU intensive than just verifying it.
    // If this function succeeds, the recovered public key is guaranteed to be valid
    // (the signature is a valid signature of the given data for that key)
    bool SetCompactSignature(uint256 hash, const std::vector<unsigned char>& vchSig);

    bool Verify(uint256 hash, const std::vector<unsigned char>& vchSig);

    // create a compact signature (65 bytes), which allows reconstructing the used public key
    // The format is one header byte, followed by two times 32 bytes for the serialized r and s values.
    // The header byte: 0x1B = first key with even y, 0x1C = first key with odd y,
    //                  0x1D = second key with even y, 0x1E = second key with odd y
    bool SignCompact(uint256 hash, std::vector<unsigned char>& vchSig);

    // Verify a compact signature
    bool VerifyCompact(uint256 hash, const std::vector<unsigned char>& vchSig);

    bool IsValid();
};

/** An encapsulated OpenSSL Elliptic Curve key (public and/or private) for a secondary address type (Exchange address) - Griffith
    uses a set of two keys instead of an ordinary one key resulting in dual priv keys*/
class CKeyExchange
{
protected:
    const bool exchangekey = true;
    EC_KEY* pkey1;
    EC_KEY* pkey2;
    bool fSet1;
    bool fSet2;
    bool fCompressedPubKey;
    void SetCompressedPubKey();

public:
    CKeyExchange();
    CKeyExchange(const CKeyExchange& b);
    CPubKeyExchange GetPubKeyExchange() const;

    ~CKeyExchange();

    CKeyExchange& operator=(const CKeyExchange& b);

    void Reset();

    bool IsNull() const;
    bool IsCompressed() const;

    void MakeNewKey(bool fCompressed);
    bool SetPrivKey(const CPrivKey& vchPrivKey1, const CPrivKey &vchPrivKey2);
    bool SetSecret(const CSecret& vchSecret, bool fCompressed = false);
    CSecret GetSecret(bool &fCompressed) const;
    CPrivKey GetPrivKey1() const;
    CPrivKey GetPrivKey2() const;
    bool SetPubKey(const CPubKeyExchange& vchPubKey);

    bool Sign(uint512 hash, std::vector<unsigned char>& vchSig);

    // reconstruct public key from a compact signature
    // This is only slightly more CPU intensive than just verifying it.
    // If this function succeeds, the recovered public key is guaranteed to be valid
    // (the signature is a valid signature of the given data for that key)
    bool SetCompactSignature(uint256 hash, const std::vector<unsigned char>& vchSig);

    bool Verify(uint256 hash, const std::vector<unsigned char>& vchSig);

    // create a compact signature (65 bytes), which allows reconstructing the used public key
    // The format is one header byte, followed by two times 32 bytes for the serialized r and s values.
    // The header byte: 0x1B = first key with even y, 0x1C = first key with odd y,
    //                  0x1D = second key with even y, 0x1E = second key with odd y
    bool SignCompact(uint256 hash, std::vector<unsigned char>& vchSig);

    // Verify a compact signature
    bool VerifyCompact(uint256 hash, const std::vector<unsigned char>& vchSig);

    bool IsValid();
};


typedef boost::variant<CKeyID, CKeyExchangeID> CKeyType;

#endif
