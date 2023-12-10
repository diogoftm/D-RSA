package main

import (
	"crypto/sha256"
	"encoding/binary"
	"errors"
	"fmt"
	"os"
	"strconv"

	"golang.org/x/crypto/argon2"
	"golang.org/x/crypto/chacha20"
)

const (
	ZEROS_ARRAY_SIZE = 4096
	UsageStr         = `Usage: %s <password> <confusion string> <iteration count> [--limit]\n
	--limit: number of bytes to be generated (default: no limit).\n
	--patternBytes: number of pattern bytes (default: 2).\n`
)

var PatternBytes int
var zerosArray []byte

type GeneratorArgs struct {
	PW string
	CS string
	IC uint16
}

type Generator struct {
	args      GeneratorArgs
	setupDone bool
	cipher    *chacha20.Cipher
}

type Seed [32]byte
type SHA256Result [32]byte
type LeadingPatternBytes [32]byte

func NewGenerator(args GeneratorArgs) *Generator {
	return &Generator{
		args:      args,
		setupDone: false,
		cipher:    nil,
	}
}

func (g *Generator) Setup() {
	var bootstrapSeed, iterationSeed Seed
	var pattern = make([]byte, PatternBytes)

	g.findBootstrapSeed(&bootstrapSeed)
	g.generatePattern(&pattern, g.args.CS)
	g.initializeGenerator(&bootstrapSeed)

	for i := uint16(0); i < g.args.IC; i++ {
		g.findNextSeedByPattern(pattern, &iterationSeed)
		g.initializeGenerator(&iterationSeed)
	}

	g.setupDone = true
}

func (g *Generator) NextBlock(block []byte) error {
	if !g.setupDone {
		return errors.New("Could not call Generator::nextBlock without calling Generator::setup()")
	}

	g.seekNextBytesFromGenerator(block)

	return nil
}

func (g *Generator) seekNextBytesFromGenerator(out []byte) {
	if zerosArray == nil || len(zerosArray) != len(out) {
		zerosArray = make([]byte, len(out))
	}
	g.cipher.XORKeyStream(out, zerosArray)
}

func (g *Generator) generatePattern(pattern *[]byte, confusionString string) {
	h := sha256.New()

	h.Write([]byte(confusionString))
	hashed := h.Sum(nil)

	*pattern = hashed[:PatternBytes]
}

func (g *Generator) findBootstrapSeed(seed *Seed) {
	iterations := 3

	salt := make([]byte, 16)
	g.setArgon2Salt(salt, g.args.CS, int(g.args.IC))

	hashedPW := argon2.Key([]byte(g.args.PW), salt, uint32(iterations), 1024*1024, 1, 32)

	*seed = Seed(hashedPW)
}

func (g *Generator) setArgon2Salt(salt []byte, CS string, IC int) {
	hash := sha256.New()

	hash.Write([]byte(CS))
	byteSliceIC := make([]byte, 4)
	binary.LittleEndian.PutUint32(byteSliceIC, uint32(IC))
	hash.Write(byteSliceIC)

	copy(salt[:16], hash.Sum(nil))
}

func (g *Generator) initializeGenerator(seed *Seed) {
	var err error
	nonce := make([]byte, 12)
	g.cipher, err = chacha20.NewUnauthenticatedCipher(seed[:], nonce)
	if err != nil {
		fmt.Fprintln(os.Stderr, "Error: unable to init chacha20")
		os.Exit(1)
	}
}

func (g *Generator) calculateSeed(outSeed *Seed, hashResult *SHA256Result, leadingBytes *LeadingPatternBytes) {
	h := sha256.New()

	h.Write(hashResult[:])
	h.Write(leadingBytes[:])

	copy(outSeed[:], h.Sum(nil))
}

func (g *Generator) findNextSeedByPattern(pattern []byte, seed *Seed) {
	h := sha256.New()
	B := make([]byte, 1)
	previousBytes := make([]byte, PatternBytes)
	leadingBytes := make([]byte, 32)

	g.seekNextBytesFromGenerator(previousBytes)

	var result SHA256Result

	equalCheck := true

	for {
		equalCheck = true
		for i := range pattern {
			if pattern[i] != previousBytes[i] {
				equalCheck = false
				break
			}
		}

		if equalCheck {
			copy(result[:], h.Sum(nil))
			g.seekNextBytesFromGenerator(leadingBytes)
			g.calculateSeed(seed, &result, (*LeadingPatternBytes)(leadingBytes))
			break
		} else {
			a := make([]byte, 1)
			a[0] = previousBytes[0]
			h.Write(a)
		}
		g.seekNextBytesFromGenerator(B)
		previousBytes = append(previousBytes[1:], B[0])
	}
}

func main() {
	var confusionStr string
	var password string
	var iterationCount uint16
	var err error
	var limit int

	PatternBytes = 2

	if len(os.Args) < 4 {
		fmt.Printf(UsageStr, os.Args[0])
		os.Exit(1)
	} else {
		password = os.Args[1]
		confusionStr = os.Args[2]
		var val uint64
		val, err = strconv.ParseUint(os.Args[3], 10, 16)
		if err != nil {
			fmt.Fprintln(os.Stderr, "Error parsing iteration count, double check its value.")
			os.Exit(1)
		}
		iterationCount = uint16(val)

		for i := 4; i < len(os.Args); i += 2 {
			if os.Args[i] == "--limit" {
				limit, err = strconv.Atoi(os.Args[i+1])
				if err != nil || limit <= 0 {
					fmt.Fprintln(os.Stderr, "Error: Invalid exponent value.")
					os.Exit(1)
				}
			} else if os.Args[i] == "--patternBytes" {
				PatternBytes, err = strconv.Atoi(os.Args[i+1])
				if err != nil || PatternBytes <= 0 {
					fmt.Fprintln(os.Stderr, "Error: Invalid number of pattern bytes.")
					os.Exit(1)
				}
			} else {
				fmt.Fprintln(os.Stderr, "Inavlid argument: ", os.Args[i])
				os.Exit(1)
			}
		}
	}

	args := GeneratorArgs{
		PW: password,
		CS: confusionStr,
		IC: iterationCount,
	}

	gen := NewGenerator(args)

	gen.Setup()

	nBytesGenerated := 0
	blockSize := 1024

	for {
		if nBytesGenerated == limit && limit != 0 {
			break
		} else if limit-nBytesGenerated < blockSize && limit != 0 {
			blockSize = limit - nBytesGenerated
		}
		block := make([]byte, blockSize)
		_ = gen.NextBlock(block)
		binary.Write(os.Stdout, binary.LittleEndian, block[:])
		nBytesGenerated += blockSize
	}

}
