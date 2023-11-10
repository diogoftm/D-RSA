if [ "$#" -eq 3 ]; then
    echo $1 | openssl pkeyutl -encrypt -inkey $2 -pubin | openssl pkeyutl -decrypt -inkey $3
else
    echo "Usage: $0 <message> <public pem> <private pem>"
    echo "If the message displayed is the same one inputed by the user, the test has passed"
fi