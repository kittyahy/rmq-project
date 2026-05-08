package main

import (
	"bufio"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"time"
	"unsafe"
)

// RMQ interface. Implementations also provide package-level functions:
//
//	func <Name>Name() string
//	func <Name>MaxN() int        // optional; omit to default to math.MaxInt
//	func <Name>Build(data []uint64) *<Name>
type Rmq interface {
	Space() int
	Query(l, r int) uint64
}

// Trivial implementation that computes each query on the fly.
type Naive struct {
	data []uint64
}

func NaiveName() string { return "QuadraticQuery" }

// NOTE: Do not use this for the improved implementations!
func NaiveMaxN() int { return 10_000 }

func NaiveBuild(data []uint64) *Naive { return &Naive{data: data} }

func (n *Naive) Space() int { return len(n.data)*8 }

func (n *Naive) Query(l, r int) uint64 {
	min := n.data[l]
	for i := l + 1; i <= r; i++ {
		if n.data[i] < min {
			min = n.data[i]
		}
	}
	return min
}

// -------------------------------------------------------------
// TODO: Implement the Rmq interface for additional data structures.
// -------------------------------------------------------------

type Input struct {
	data   []uint64
	queryL []int
	queryR []int
}

// readInput reads the given input file.
func readInput(file string) (Input, error) {
	f, err := os.Open(file)
	if err != nil {
		return Input{}, err
	}
	defer f.Close()
	r := bufio.NewReader(f)

	var n, q int
	fmt.Fscan(r, &n, &q)

	data := make([]uint64, n)
	for i := range data {
		fmt.Fscan(r, &data[i])
	}
	queryL := make([]int, q)
	queryR := make([]int, q)
	for i := range queryL {
		fmt.Fscan(r, &queryL[i], &queryR[i])
	}
	return Input{data, queryL, queryR}, nil
}

// bench benchs a pre-built RMQ instance and prints results in CSV format.
func bench(input Input, name string, maxN int, rmq Rmq) {
	n := len(input.data)
	fmt.Fprintf(os.Stderr, "%10d\t%20s\t", n, name)
	if n > maxN {
		fmt.Fprintln(os.Stderr, "skipped")
		return
	}
	fmt.Fprintf(os.Stderr, "%10d\t", rmq.Space())

	q := len(input.queryL)
	start := time.Now()
	var sum uint64
	for i := 0; i < q; i++ {
		sum += rmq.Query(input.queryL[i], input.queryR[i])
	}
	nsPerQuery := float64(time.Since(start).Nanoseconds()) / float64(q)

	fmt.Printf("%d,%d,%s,%d,%d,%.6f\n", n, q, name, rmq.Space(), sum, nsPerQuery)
	fmt.Fprintf(os.Stderr, "%3d\t%.2fns/q\n", sum%1000, nsPerQuery)
}

func benchNaive(input Input) {
	bench(input, NaiveName(), NaiveMaxN(), NaiveBuild(input.data))
}

// TODO: Add bench helpers for other implementations here.

func main() {
	if len(os.Args) < 2 {
		fmt.Fprintln(os.Stderr, "Usage: rmq-go <input_dir>")
		os.Exit(1)
	}

	fmt.Println("n,q,name,space,sum,time")

	fileOrDir := os.Args[1]
	fmt.Fprintf(os.Stderr, "Reading input from %q ..\n", fileOrDir)

	var inputs []Input
	info, err := os.Stat(fileOrDir)
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
	if info.Mode().IsRegular() {
		input, err := readInput(fileOrDir)
		if err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
		inputs = append(inputs, input)
	} else {
		entries, _ := filepath.Glob(filepath.Join(fileOrDir, "*.in"))
		for _, e := range entries {
			input, err := readInput(e)
			if err != nil {
				fmt.Fprintln(os.Stderr, err)
				continue
			}
			inputs = append(inputs, input)
		}
		sort.Slice(inputs, func(i, j int) bool {
			return len(inputs[i].data) < len(inputs[j].data)
		})
	}

	for _, input := range inputs {
		benchNaive(input)
		// TODO: Add other implementations here.
	}
}
