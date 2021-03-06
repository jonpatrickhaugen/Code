//
// The MIT License(MIT)
//
// Copyright(c) 2014 Demonsaw LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <cassert>
#include <cryptopp/nbtheory.h>
#include <cryptopp/osrng.h>

#include "security/algorithm/diffie_hellman.h"

// NOTE: Ideally, we always want to validate the base/prime
// Unfortunately, I ran into some compatibility problems when using C# & C++ versions.
// Most likely this was caused by a shitty crypto implementation. To maintain backwards
// compatibility cross-language I had to disable this check in demonsaw v2.0. Boo.
#define VALIDATE (1)

namespace eja
{
	// Operator
	diffie_hellman& diffie_hellman::operator=(const diffie_hellman& dh)
	{
		if (this != &dh)
		{
			m_dh = dh.m_dh;
			m_private_key = dh.m_private_key;
			m_public_key = dh.m_public_key;
			m_shared_key = dh.m_shared_key;
		}

		return *this;
	}

	// Interface
	void diffie_hellman::clear()
	{
		// NOTE: Doesn't clear p, q, or g
		m_private_key.resize(0);
		m_public_key.resize(0);
		m_shared_key.resize(0);
	}

	// Init
	void diffie_hellman::init(const size_t pbits /*= get_default_size()*/)
	{
		assert(pbits >= 128);

		// Safe primes are of the form p = 2q + 1, p and q prime.
		// These parameters do not state a maximum security level based
		// on the prime subgroup order. In essence, we get the maximum
		// security level. There is no free lunch: it means more modular
		// multiplications are performed, which affects performance.

		// For a compare/contrast of meeting a security level, see dh-init.zip.
		// Also see http://www.cryptopp.com/wiki/Diffie-Hellman and
		// http://www.cryptopp.com/wiki/Security_level .
		//
		CryptoPP::AutoSeededX917RNG<CryptoPP::AES> rng;
		m_dh.AccessGroupParameters().Initialize(rng, static_cast<int>(pbits));

#if VALIDATE
		if (!m_dh.GetGroupParameters().ValidateGroup(rng, 3))
			throw std::runtime_error("Failed to validate base and prime");

		// http://groups.google.com/group/sci.crypt/browse_thread/thread/7dc7eeb04a09f0ce
		const auto v = ModularExponentiation(get_g(), get_q(), get_p());
		if (v != CryptoPP::Integer::One())
			throw std::runtime_error("Failed to verify order of the subgroup");
#endif
		m_private_key.resize(m_dh.PrivateKeyLength());
		m_public_key.resize(m_dh.PublicKeyLength());
		m_shared_key.resize(m_dh.AgreedValueLength());

		// Generate a pair of integers for Alice. The public integer is forwarded to Bob.
		m_dh.GenerateKeyPair(rng, m_private_key, m_public_key);
	}
	
	void diffie_hellman::init(const CryptoPP::Integer& p, const CryptoPP::Integer& g)
	{
		assert(p.NotZero() && g.NotZero());

		CryptoPP::AutoSeededX917RNG<CryptoPP::AES> rng;
		m_dh.AccessGroupParameters().Initialize(p, g);

#if VALIDATE
		if (!m_dh.GetGroupParameters().ValidateGroup(rng, 3))
			throw std::runtime_error("Failed to validate base and prime");

		// http://groups.google.com/group/sci.crypt/browse_thread/thread/7dc7eeb04a09f0ce
		const auto v = ModularExponentiation(get_g(), get_q(), get_p());
		if (v != CryptoPP::Integer::One())
			throw std::runtime_error("Failed to verify order of the subgroup");
#endif
		m_private_key.resize(m_dh.PrivateKeyLength());
		m_public_key.resize(m_dh.PublicKeyLength());
		m_shared_key.resize(m_dh.AgreedValueLength());

		// Generate a pair of integers
		m_dh.GenerateKeyPair(rng, m_private_key, m_public_key);
	}
	
	void diffie_hellman::init(const CryptoPP::Integer& p, const CryptoPP::Integer& q, const CryptoPP::Integer& g)
	{
		assert(p.NotZero() && q.NotZero() && g.NotZero());

		CryptoPP::AutoSeededX917RNG<CryptoPP::AES> rng;
		m_dh.AccessGroupParameters().Initialize(p, q, g);

#if VALIDATE
		if (!m_dh.GetGroupParameters().ValidateGroup(rng, 3))
			throw std::runtime_error("Failed to validate base and prime");

		// http://groups.google.com/group/sci.crypt/browse_thread/thread/7dc7eeb04a09f0ce
		const auto v = ModularExponentiation(get_g(), get_q(), get_p());
		if (v != CryptoPP::Integer::One())
			throw std::runtime_error("Failed to verify order of the subgroup");
#endif
		m_private_key.resize(m_dh.PrivateKeyLength());
		m_public_key.resize(m_dh.PublicKeyLength());
		m_shared_key.resize(m_dh.AgreedValueLength());

		// Generate a pair of integers for Alice. The public integer is forwarded to Bob.
		m_dh.GenerateKeyPair(rng, m_private_key, m_public_key);
	}

	// Compute
	void diffie_hellman::compute(const void* input, const size_t input_size)
	{
		assert(input && input_size);

		const auto status = m_dh.Agree(m_shared_key, m_private_key, reinterpret_cast<const byte*>(input));
		if (!status)
			throw std::runtime_error("Invalid key agreement");
	}
}
