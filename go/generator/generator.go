package main

import (
	"crypto/sha256"
	"encoding/binary"
	"errors"
	"fmt"
	"math"
	"os"
	"strconv"

	"golang.org/x/crypto/argon2"
	"golang.org/x/crypto/chacha20"
)

const (
	PatternBytes     = 2
	ZEROS_ARRAY_SIZE = 4096
)

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
type Pattern [PatternBytes]byte
type SHA256Result [32]byte
type LeadingPatternBytes [32]byte

func NewGenerator(args GeneratorArgs) *Generator {
	return &Generator{
		args:      args,
		setupDone: false,
		cipher:    nil,
	}
}

var zerosArray [ZEROS_ARRAY_SIZE]byte

func (g *Generator) Setup() {
	var bootstrapSeed, iterationSeed Seed
	var pattern Pattern

	g.findBootstrapSeed(&bootstrapSeed)
	g.generatePattern(&pattern, g.args.CS)
	g.initializeGenerator(&bootstrapSeed)

	for i := uint16(0); i < g.args.IC; i++ {
		g.findNextSeedByPattern(&pattern, &iterationSeed)
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
	zerosArray := make([]byte, len(out))
	g.cipher.XORKeyStream(out, zerosArray)
}

func (g *Generator) generatePattern(pattern *Pattern, confusionString string) {
	h := sha256.New()

	h.Write([]byte(confusionString))
	hashed := h.Sum(nil)

	copy(pattern[:], hashed[:PatternBytes])
}

func (g *Generator) findBootstrapSeed(seed *Seed) {
	iterations := g.getArgon2IterationsByIC(int(g.args.IC))

	salt := make([]byte, 16)
	g.setArgon2Salt(salt, g.args.CS, int(g.args.IC))

	hashedPW := argon2.Key([]byte(g.args.PW), salt, uint32(iterations), 64*1024, 1, 32)

	copy(seed[:], hashedPW)
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

func (g *Generator) findNextSeedByPattern(pattern *Pattern, seed *Seed) {
	h := sha256.New()
	B0 := make([]byte, 1)
	B1 := make([]byte, 1)
	leadingBytes := make([]byte, 32)

	g.seekNextBytesFromGenerator(B0)

	var result SHA256Result

	for {
		g.seekNextBytesFromGenerator(B1)

		if pattern[0] == B0[0] && pattern[1] == B1[0] {
			copy(result[:], h.Sum(nil))
			g.seekNextBytesFromGenerator(leadingBytes)
			g.calculateSeed(seed, &result, (*LeadingPatternBytes)(leadingBytes))
			break
		} else {
			h.Write(B0)
			B0[0] = B1[0]
		}
	}
}

func (g *Generator) getArgon2IterationsByIC(IC int) int {
	minIterations := 3
	maxIterations := 6

	usedIterations := minIterations + int((float64(maxIterations-minIterations))*(math.Log10(float64(IC))/4.0))

	if usedIterations < minIterations {
		return minIterations
	} else if usedIterations > maxIterations {
		return maxIterations
	}

	return usedIterations
}

func main() {
	var confusionStr string
	var password string
	var iterationCount uint16
	var err error
	var limit int

	if len(os.Args) < 4 {
		fmt.Printf("Usage: %s <password> <confusion string> <iteration count> [--limit]\n", os.Args[0])
		fmt.Println("--limit: number of bytes to be generated.")
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

		if len(os.Args) > 6 {
			fmt.Fprintln(os.Stderr, "Error: too many arguments.")
			os.Exit(1)
		} else if len(os.Args) == 6 {
			if os.Args[4] == "--limit" {
				limit, err = strconv.Atoi(os.Args[5])
				if err != nil || limit <= 0 {
					fmt.Println("")
					fmt.Fprintln(os.Stderr, "Error: Invalid exponent value.")
					os.Exit(1)
				}
			} else {
				fmt.Println()
				fmt.Fprintln(os.Stderr, "Inavlid flag: ", os.Args[4])
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
		binary.Write(os.Stdout, binary.NativeEndian, block[:])
		nBytesGenerated += blockSize
	}

}
