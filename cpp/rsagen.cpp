#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <iostream>
#include <fstream>
#include <cstring>

struct keyInfo
{
    BIGNUM *d;
    BIGNUM *p;
    BIGNUM *q;
    BIGNUM *yn;
    BIGNUM *e;
    BIGNUM *n;
    BIGNUM *dmp1;
    BIGNUM *dmq1;
    BIGNUM *iqmp1;
};

void rsaKeyGen(keyInfo *key, unsigned long exponent, int keySize)
{
    // choose two large prime numbers
    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *p = BN_new();
    BN_set_bit(p, 1024);
    BIGNUM *q = BN_new();
    BN_set_bit(q, 1024);
    BN_generate_prime_ex(p, keySize / 2, 1, NULL, NULL, NULL);
    BN_generate_prime_ex(q, keySize / 2, 1, NULL, NULL, NULL);

    // n = pq
    BIGNUM *n = BN_new();
    BN_mul(n, p, q, ctx);

    // compute Carmichael's totient function
    BIGNUM *gcd = BN_new();
    BIGNUM *yn = BN_new();
    BIGNUM *pm = BN_new();
    BIGNUM *qm = BN_new();
    BN_sub(pm, p, BN_value_one());
    BN_sub(qm, q, BN_value_one());
    BN_gcd(gcd, pm, qm, ctx);
    BN_mul(yn, pm, qm, ctx);
    BN_div(yn, NULL, yn, gcd, ctx);

    // exponent
    BIGNUM *e = BN_new();
    BN_set_word(e, exponent);

    // check gcd(e, λ(n)) = 1
    BN_gcd(gcd, e, yn, ctx);
    if(BN_cmp(gcd, BN_value_one())!=0){
        std::cerr << "Error: Invalid condition gcd(e, λ(n)) = 1" << std::endl;
        exit(1);
    }

    // modular inverse
    BIGNUM *d = BN_new();
    BN_mod_inverse(d, e, yn, ctx);

    // Chinese remainder theorem related constants
    BIGNUM *dmp1 = BN_new();
    BIGNUM *dmq1 = BN_new();
    BIGNUM *iqmp = BN_new();
    BIGNUM *dm = BN_new();
    BIGNUM *bn_p = BN_new();
    BIGNUM *bn_q = BN_new();

    BN_sub(dm, d, BN_value_one());
    BN_sub(bn_p, bn_p, BN_value_one());
    BN_sub(bn_q, bn_q, BN_value_one());
    BN_mod(dmp1, d, qm, ctx);
    BN_mod(dmq1, d, qm, ctx);
    BN_mod_inverse(iqmp, q, p, ctx);

    // results
    key->d = d;
    key->e = e;
    key->p = p;
    key->q = q;
    key->yn = yn;
    key->n = n;
    key->dmp1 = dmp1;
    key->dmq1 = dmq1;
    key->iqmp1 = iqmp;

    // clean
    BN_free(gcd);
    BN_free(bn_p);
    BN_free(bn_q);
}

bool isPrime(unsigned long n)
{
    if (n == 2 || n == 3)
        return true;

    if (n <= 1 || n % 2 == 0 || n % 3 == 0)
        return false;

    for (int i = 5; i * i <= n; i += 6)
    {
        if (n % i == 0 || n % (i + 2) == 0)
            return false;
    }

    return true;
}

int main(int argc, char const *argv[])
{
    unsigned long exponent = 65537; // 2^16+1
    int keySize = 2048;

    // command line arguments
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <private_key_file> <public_key_file> [-s | -e]" << std::endl
                  << "-e: set exponent value (65537 default)\n-s: key size (2048 default)" << std::endl;
        return 1;
    }
    else if (argc > 3)
    {
        for (int i = 3; i < argc; i++)
        {
            if (strcmp(argv[i], "-e") == 0)
            {
                exponent = std::stoul(argv[i + 1]);
                if (exponent < 3 || !isPrime(exponent))
                {
                    std::cerr << "Error: Invalid exponent value" << std::endl;
                }
            }
            else if (strcmp(argv[i], "-s") == 0)
            {
                if (std::atoi(argv[i + 1]) % 8 != 0)
                {
                    std::cerr << "Error: Make sure that the key size is a multiple of 8" << std::endl;
                    return 1;
                }
                keySize = std::atoi(argv[i + 1]);
            }
        }
    }

    const char *privFile = argv[1];
    const char *pubFile = argv[2];

    // generate key-pair
    keyInfo keyPair;
    rsaKeyGen(&keyPair, exponent, keySize);

    // Save keys in PEM format
    RSA *rsa = RSA_new();
    RSA_set0_key(rsa, keyPair.n, keyPair.e, keyPair.d);
    RSA_set0_factors(rsa, keyPair.p, keyPair.q);
    RSA_set0_crt_params(rsa, keyPair.dmp1, keyPair.dmq1, keyPair.iqmp1);

    FILE *pubFp = fopen(pubFile, "w");
    PEM_write_RSAPublicKey(pubFp, rsa);
    fclose(pubFp);

    FILE *privFp = fopen(privFile, "w");
    PEM_write_RSAPrivateKey(privFp, rsa, 0, 0, 0, 0, 0);

    RSA_free(rsa);

    return 0;
}
