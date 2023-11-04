echo $1 | openssl pkeyutl -encrypt -inkey pub.pem -pubin | openssl pkeyutl -decrypt -inkey priv.pem
