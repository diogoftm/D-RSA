# C++ Implementation

## Files
- generator.h - pseudo-random generator header;
- generator.cpp - pseudo-random generator implementation;
- rsagen.cpp - generate a RSA key-pair and save it in two PEM formated files (private and public);
- test_keys.sh - test RSA key by encrypting a message with the public key and then decrypting with the private.
- RBG.cpp - Random byte generator (behaves like /dev/urandom)

## Dependecies

```
sudo apt-get install libsodium-dev openssl libargon2-0-dev
```

# Compile
```
make all
```

## RBG
Usage ./RBG password confusionString iterationCount [--limit nbytes] [--patternBytes nbytes]

The default value for patternBytes argument is two.

Use limit argument to stop the generator after producing nbytes.

## rsagen

### Run
Usage: 
./rsagen <private_key_file> <public_key_file> [-s | -e]
-e: set exponent value (65535 default)
-s: key size (2048 default)

Example with an exponent equal to 3, key size equal to 4096 and 
entropy from `RBG`:
```
cat ./RBG pw cs 5 | ./rsagen priv.pem pub.pem -e 3 -s 4096
```
