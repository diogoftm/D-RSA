# Go Implementation
[Install golang here]("https://go.dev/doc/install")

## Download dependecies
```
go mod tidy
```

## rsagen

Example run:
```
cat /dev/urandom | go run . priv.pem pub.pem -e 3 -s 4096
```
