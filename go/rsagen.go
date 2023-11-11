package main

import (
	"crypto/rsa"
	"crypto/x509"
	"encoding/pem"
	"errors"
	"fmt"
	"math/big"
	"os"
	"strconv"
)

type keyInfo struct {
	d     *big.Int
	p     *big.Int
	q     *big.Int
	yn    *big.Int
	e     *big.Int
	n     *big.Int
	dmp1  *big.Int
	dmq1  *big.Int
	iqmp1 *big.Int
}

func genPrime(valSize int) (*big.Int, error) {
	data := make([]byte, valSize/8)

	_, err := os.Stdin.Read(data)

	if err != nil {
		return nil, err
	}

	two := new(big.Int).SetInt64(2)

	data[valSize/8-1] |= 0x01
	value := new(big.Int).SetBytes(data[:valSize/8])

	isPrime := false

	for !isPrime {
		value.Add(value, two)
		isPrime = value.ProbablyPrime(128)
	}

	return value, nil
}

func rsaKeyGen(keySize int, exponent uint64) (*keyInfo, error) {
	// Const
	one := new(big.Int).SetInt64(1)

	// generate two primes, p and q
	p, _ := genPrime(keySize / 2)
	q, _ := genPrime(keySize / 2)

	// n = pq
	n := new(big.Int)
	n.Mul(p, q)

	// compute Carmichael's totient function
	gcd := new(big.Int)
	yn := new(big.Int)
	pm := new(big.Int)
	qm := new(big.Int)
	pm.Sub(p, one)
	qm.Sub(q, one)
	gcd.GCD(nil, nil, pm, qm)
	yn.Mul(pm, qm)
	yn.Div(yn, gcd)

	// exponent
	e := new(big.Int)
	e.SetUint64(exponent)

	// check gcd(e, λ(n)) = 1
	gcd.GCD(nil, nil, e, yn)
	if gcd.Cmp(one) != 0 {
		return nil, errors.New("check gcd(e, λ(n)) != 1")
	}

	// modular inverse
	d := new(big.Int)
	d.ModInverse(e, yn)

	// Chinese remainder theorem related constants
	dmp1 := new(big.Int)
	dmq1 := new(big.Int)
	iqmp := new(big.Int)
	dm := new(big.Int)

	dm.Sub(d, one)
	dmp1.Sub(d, pm)
	dmq1.Sub(d, qm)
	iqmp.ModInverse(q, p)

	key := keyInfo{d, p, q, yn, e, n, dmp1, dmq1, iqmp}

	return &key, nil
}

// simple test for small numbers
// (not used for RSA key-pair generation)
func isPrime(n uint64) bool {
	if n <= 1 {
		return false
	}
	for i := uint64(2); i*i <= n; i++ {
		if n%i == 0 {
			return false
		}
	}
	return true
}

func main() {
	keySize := 2048
	exponent := 65537
	var err error

	if len(os.Args) < 3 {
		fmt.Printf("Usage: %s <private_key_file> <public_key_file> [-s | -e]\n", os.Args[0])
		fmt.Println("-e: set exponent value (default:", exponent, ")\n-s: key size (default:", keySize, ")")
		os.Exit(1)
	} else if len(os.Args) > 3 {
		for i := 3; i < len(os.Args); i++ {
			if os.Args[i] == "-e" {
				exponentArg := os.Args[i+1]
				if exponent, err = strconv.Atoi(exponentArg); err != nil || exponent < 3 || !isPrime(uint64(exponent)) {
					fmt.Println("Error: Invalid exponent value")
					os.Exit(1)
				}
			} else if os.Args[i] == "-s" {
				keySizeArg := os.Args[i+1]

				if keySize, err = strconv.Atoi(keySizeArg); err != nil || keySize%8 != 0 {
					fmt.Println("Error: Invalid key size")
					os.Exit(1)
				}
			}
		}
	}

	var keyVals *keyInfo

	for true {
		keyVals, err = rsaKeyGen(keySize, uint64(exponent))
		if err == nil {
			break
		}
	}

	keyPair := &rsa.PrivateKey{
		PublicKey: rsa.PublicKey{
			N: keyVals.n,
			E: exponent,
		},
		D:      keyVals.d,
		Primes: []*big.Int{keyVals.p, keyVals.q},
	}

	// Private key to PEM
	privateKeyPEM := &pem.Block{
		Type:  "RSA PRIVATE KEY",
		Bytes: x509.MarshalPKCS1PrivateKey(keyPair),
	}

	privateKeyFile, err := os.Create(os.Args[1])
	if err != nil {
		fmt.Println("Error creating private key file:", err)
		return
	}
	defer privateKeyFile.Close()

	err = pem.Encode(privateKeyFile, privateKeyPEM)
	if err != nil {
		fmt.Println("Error encoding private key to PEM:", err)
		return
	}

	// Public key to PEM
	publicKeyPEM, err := x509.MarshalPKIXPublicKey(&keyPair.PublicKey)
	if err != nil {
		fmt.Println("Error marshaling public key:", err)
		return
	}

	publicKeyFile, err := os.Create(os.Args[2])
	if err != nil {
		fmt.Println("Error creating public key file:", err)
		return
	}
	defer publicKeyFile.Close()

	err = pem.Encode(publicKeyFile, &pem.Block{
		Type:  "PUBLIC KEY",
		Bytes: publicKeyPEM,
	})
	if err != nil {
		fmt.Println("Error encoding public key to PEM:", err)
		return
	}
}
