# D-RSA
Applied Cryptography 2nd project


# RBG

```
Input: password (PW) / confusion string (CS) / iteration count (IC)
Block size := 32 bytes

1. Bootstrap seed := PBKDF2(PW,CS,SHA256,IC, 32)
2. Confusion Pattern := SHA256(confusion string) % (2^16)
3. ...
4. New Seed := 16 Bytes after Confusion Pattern + (Computed Hash % 2^16)
   Computed Hash := SHA256(BFinal,...(...(SHA256(B0,B1))...))

6. ...
```