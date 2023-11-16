# Go Implementation
[Install golang here]("https://go.dev/doc/install")

## Download dependecies
```
go mod tidy
```

## rsagen

First go to the right directory:
```
cd rsagen/
```

Example run (interpreted):
```
cat /dev/urandom | go run . priv.pem pub.pem -e 3 -s 4096
```

Example run (compiled):
```
go build rsagen.go
cat /dev/urandom | ./rsagen priv.pem pub.pem -e 3 -s 4096
```

## Random generator
First go to the right directory:
```
cd generator/
```

Example run (interpreted):
```
go run . password confusionString 10 --limit 4
```

Example run (compiled):
```
go build generator.go
./generator password confusionString 10 --limit 4
```