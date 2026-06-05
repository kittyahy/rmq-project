#include <algorithm>
#include <chrono>
#include <complex>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

// RMQ interface (duck-typed via templates):
//
//   static std::string name();
//   static size_t max_n();               // optional, defaults to SIZE_MAX
//   static RMQ build(const std::vector<uint64_t>& data);
//   size_t space() const;
//   uint64_t query(size_t l, size_t r) const;

// Trivial implementation that computes each query on the fly.
struct Naive {
	static std::string name() { return "QuadraticQuery"; }
	// NOTE: Improved implementations should simply return SIZE_MAXX.
	static size_t max_n() { return 10'000; }

	const std::vector<uint64_t>* data;

	static Naive build(const std::vector<uint64_t>& data) { return {&data}; }

	size_t space() const { return sizeof(*this); }

	uint64_t query(size_t l, size_t r) const {
		uint64_t min = (*data)[l];
		for(size_t i = l + 1; i <= r; ++i) min = std::min(min, (*data)[i]);
		return min;
	}
};

// implementation that precompute all queries,
struct Precompute {
	~Precompute() {
		delete dataContainer;
	}
	static std::string name() {return "Precomputed";}
	static size_t max_n() {return 10'000;}
	const std::vector<std::vector<uint64_t>>* dataContainer;
	static Precompute build(const std::vector<uint64_t>& data) {
		auto* dataContainer = new std::vector<std::vector<uint64_t>>(data.size());
		for (int r = 0; r < data.size(); r++) {
			(*dataContainer)[r].resize(r+1);
			(*dataContainer)[r][r] = data[r];
			for (int l = r-1; 0 <= l; l--) {
				(*dataContainer)[r][l] = std::min((*dataContainer)[r][l+1],data[l]);
			}
		}
	return {dataContainer};
	}

	size_t space() const {
		size_t size = 0;
		for ( size_t i = 0; i < dataContainer->size(); i++) {
			size += dataContainer->at(i).size();
		}
		return size*sizeof(uint64_t);
	}
	uint64_t query(size_t l, size_t r) const {
 	return (*dataContainer)[r][l];
	}


};

// implementation using a sparse array
struct SparseArray {
	~SparseArray() {
		delete dataContainer;
	}
	static std::string name() {return "Sparse";}
	static size_t max_n() {return 10'000;}

	const std::vector<std::vector<uint64_t>>* dataContainer;

	static SparseArray build(const std::vector<uint64_t>& data) {
		auto k = static_cast<int>(std::log2(data.size()));
		auto* dataContainer = new std::vector<std::vector<uint64_t>>(k+1);
		for (size_t l = 0; l <= k; l++) {
			auto current_size = data.size()-((1<<l)-1);
			(*dataContainer)[l].resize(current_size);
			for (size_t i = 0; i < current_size; i++) {
				uint64_t minimum = data[i];
				for(size_t j = i + 1; j < (1 << l)+i; j++) minimum = std::min(minimum, data[j]); //DP möglich
				(*dataContainer)[l][i] = minimum;
			}
		}
		return {dataContainer};
	}

	size_t space() const {
		size_t size = 0;
		for (size_t i = 0; i < dataContainer->size(); i++) {
			size += dataContainer->at(i).size();
		}
		return size*sizeof(uint64_t);
	};

	uint64_t query(size_t l, size_t r) const {
		r++; //to use the algorithm given in the slides
		auto largest_power = static_cast<int>(std::log2(r - l));
		auto left = (*dataContainer)[largest_power][l];
		auto right = (*dataContainer)[largest_power][r-(1<<largest_power)];
		return std::min(left,right);
	}
};

// implementation using a segment tree,
struct SegmentTree {
	~SegmentTree() {
		delete dataContainer;
	}
	static std::string name() {return "SegmentTree";}
	static size_t max_n() {return 10'000;}

	const std::vector<std::vector<uint64_t>>* dataContainer;

	static SegmentTree build(const std::vector<uint64_t>& data) {
		int k = static_cast<int>(std::log2(data.size()));
		auto* dataContainer = new std::vector<std::vector<uint64_t>>(k+1);
		for (size_t l = 0; l <= k; l++) {
			auto power = (1<<l);
			auto size_of_l = static_cast<int>(data.size()/power);
			(*dataContainer)[l].resize(size_of_l);
			for (size_t i = 0; i < size_of_l; i++) {
				uint64_t minimum = data[i*power];
				for(size_t j = i*power+1; j < (i+1)*power; j++) minimum = std::min(minimum, data[j]); //DP möglich
				(*dataContainer)[l][i] = minimum;
			}
		}
		return {dataContainer};
	}

	size_t space() const {
		size_t size = 0;
		for (size_t i = 0; i < dataContainer->size(); i++) {
			size += dataContainer->at(i).size();
		}
		return size*sizeof(uint64_t);
	};

	uint64_t query(size_t l, size_t r) const {
		r++;  // to use the algorithm from the slides
		auto exponent = 0;
		uint64_t minimum = SIZE_MAX;
		while (r != l ) {
			if ((l % (1<<(exponent+1))) == (1<<(exponent))){
				minimum = std::min(minimum, (*dataContainer)[exponent][static_cast<int>(l/(1<<exponent))]);
				l = l + (1<<exponent);
			}
			if ((r % (1<<(exponent+1))) == (1<<(exponent))){
				minimum = std::min(minimum, (*dataContainer)[exponent][static_cast<int>(r/(1<<exponent))-1] );
				r = r - (1<<exponent);
			}
			exponent++;
		}
		return minimum;
	}
};

// implementation using a block based approach with suffix/prefix minima on the fly
//TODO
struct BlockBasedOnTheFly {
	static std::string name() {return "BlockBasedOnTheFly";}
	static size_t max_n() {return SIZE_MAX;}

	static BlockBasedOnTheFly build(const std::vector<uint64_t>& data);//TODO

	size_t space() const; //TODO

	uint64_t query(size_t l, size_t r) const;//TODO
};

// implementation using a block based approach with suffix/prefix minima precomputed
//TODO
struct BlockBasedPrecomputed {
	static std::string name() {return "BlockBasedPrecomputed";}
	static size_t max_n() {return SIZE_MAX;}

	static BlockBasedPrecomputed build(const std::vector<uint64_t>& data);//TODO

	size_t space() const;//TODO

	uint64_t query(size_t l, size_t r) const;//TODO
};

// implementation with Cartesian trees (with varying block size).
//TODO
struct CartesianTree {
	static std::string name() {return "CartesianTree";}
	static size_t max_n() {return SIZE_MAX;}

	static CartesianTree build(const std::vector<uint64_t>& data); //TODO


	size_t space() const; //TODO


	uint64_t query(size_t l, size_t r) const; //TODO

};

struct Input {
	std::vector<uint64_t> data;
	std::vector<std::pair<size_t, size_t>> queries;
};

// Read the given input file.
Input read_input(const std::filesystem::path& file) {
	std::ifstream f(file);
	size_t n, q;
	f >> n >> q;
	Input input;
	input.data.resize(n);
	for(auto& v : input.data) f >> v;
	input.queries.resize(q);
	for(auto& [l, r] : input.queries) f >> l >> r;
	return input;
}

// Bench the given RMQ implementation on the given input, and print results in CSV format.
template <typename RMQ>
void bench(const Input& input) {
	std::cerr << std::setw(10) << input.data.size() << "\t" << std::setw(20) << RMQ::name() << "\t";

	size_t max_n = RMQ::max_n();

	if(input.data.size() > max_n) {
		std::cerr << "skipped\n";
		return;
	}

	auto rmq = RMQ::build(input.data);
	std::cerr << std::setw(10) << rmq.space() << "\t";

	auto start   = std::chrono::high_resolution_clock::now();
	uint64_t sum = 0;
	for(auto& [l, r] : input.queries) sum += rmq.query(l, r);
	auto end = std::chrono::high_resolution_clock::now();

	double elapsed =
	    static_cast<double>(
	        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()) /
	    static_cast<double>(input.queries.size());

	std::cout << input.data.size() << "," << input.queries.size() << "," << RMQ::name() << ","
	          << rmq.space() << "," << sum << "," << elapsed << "\n";
	std::cerr << std::setw(3) << (sum % 1000) << "\t" << std::fixed << std::setprecision(2)
	          << elapsed << "ns/q\n";
}

int main(int argc, char* argv[]) {
	if(argc < 2) {
		std::cerr << "Usage: rmq-cpp <input_dir>\n";
		return 1;
	}

	std::cout << "n,q,name,space,sum,time\n";

	std::filesystem::path file_or_dir(argv[1]);
	std::cerr << "Reading input from " << file_or_dir << " ..\n";

	std::vector<Input> inputs;
	if(std::filesystem::is_regular_file(file_or_dir)) {
		inputs.push_back(read_input(file_or_dir));
	} else {
		for(auto& entry : std::filesystem::directory_iterator(file_or_dir)) {
			if(entry.path().extension() == ".in") inputs.push_back(read_input(entry.path()));
		}
		std::sort(inputs.begin(), inputs.end(),
		          [](const Input& a, const Input& b) { return a.data.size() < b.data.size(); });
	}

	for(const auto& input : inputs) {
		bench<Naive>(input);
		bench<Precompute>(input);
		bench<SparseArray>(input);
		bench<SegmentTree>(input);
		// TODO: Add other implementations here.
	}

	return 0;
}
