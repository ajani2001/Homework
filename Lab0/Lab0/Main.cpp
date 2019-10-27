#include <iostream>
#include <gtest/gtest.h>
using namespace std;

enum Nucleotide { A, G, C, T };

unsigned int mask = 3;

template <typename T> void copyArray(const T* source, T* dest, size_t arrayLength) {
	for (int i = 0; i < arrayLength; ++i) {
		dest[i] = source[i];
	}
}

class RNA {
private:
	size_t storageSize;
	unsigned int* storage;

	void resize(size_t newSize) {
		unsigned int* newStorage = new unsigned int[newSize];
		copyArray<unsigned int>(storage, newStorage, storageSize < newSize ? storageSize : newSize);
		delete[] storage;
		storageSize = newSize;
		storage = newStorage;
	}

	void fitSize() {/*
		if (length / 4 > sizeof(unsigned int)*storageSize * 99 / 100 || length / 4 < sizeof(unsigned int)*storageSize * 90 / 100) {
			resize(length / 4 / 95 * 100 / sizeof(unsigned int));
		}*/
		size_t bytesRequired = (length + 3) / 4;
		size_t bytesAllocated = storageSize * sizeof(unsigned int);
		if (bytesRequired + 100 >= bytesAllocated) {
			resize(storageSize + 1000);
			return;
		}
		if (bytesRequired + 1900 <= bytesAllocated) {
			resize(storageSize - 1000);
			return;
		}
	}

	Nucleotide getNucleotide(size_t nucleotideIndex) const {
		size_t bitPackIndex = nucleotideIndex / (4 * sizeof(unsigned int));
		size_t bitPairIndex = nucleotideIndex % (4 * sizeof(unsigned int));
		unsigned int bitPack = storage[bitPackIndex];
		return Nucleotide((bitPack >> bitPairIndex)&mask);
	}

	void setNucleotide(size_t nucleotideIndex, Nucleotide newValue) {
		if (nucleotideIndex >= length) {
			throw 1;
		}
		size_t bitPackIndex = nucleotideIndex / (4 * sizeof(unsigned int));
		size_t bitPairIndex = nucleotideIndex % (4 * sizeof(unsigned int));
		unsigned int bitPack = storage[bitPackIndex];
		storage[bitPackIndex] = (bitPack&(~(mask << bitPairIndex))) | ((unsigned int(newValue)) << bitPairIndex);
	}

	class StorageAccessor {
	private:
		RNA* proprietor;
		size_t nucleotideIndex;
	public:
		StorageAccessor(RNA* proprietor, size_t nucleotideIndex) : proprietor(proprietor), nucleotideIndex(nucleotideIndex) {}
		operator Nucleotide () const {
			return proprietor->getNucleotide(nucleotideIndex);
		}
		Nucleotide operator= (Nucleotide newValue) const {
			proprietor->setNucleotide(nucleotideIndex, newValue);
			return newValue;
		}
	};

public:
	size_t length;
	RNA() : storageSize(0), length(0), storage(nullptr) {}
	RNA(const RNA& rna): storageSize(rna.storageSize), length(rna.length) {
		storage = new unsigned int[rna.storageSize];
		copyArray<unsigned int>(rna.storage, storage, storageSize);
	}
	RNA(Nucleotide filler, int fillLength) : length(fillLength) {
		storageSize = 0;
		storage = nullptr;
		fitSize();
		for (int i = 0; i < fillLength; ++i) {
			setNucleotide(i, filler);
		}
	}
	StorageAccessor operator[] (size_t nucleotideIndex) {
		return StorageAccessor(this, nucleotideIndex);
	}
	Nucleotide operator[] (size_t nucleotideIndex) const {
		return getNucleotide(nucleotideIndex);
	}
	bool operator== (const RNA& rvalue) const {
		if (length != rvalue.length) {
			return false;
		}
		for (int i = 0; i < length; ++i) {
			if ((*this)[i] != rvalue[i]){
				return false;
			}
		}
		return true;
	}
	bool operator!= (const RNA& rvalue) const {
		return !operator==(rvalue);
	}
	RNA operator~() const {
		RNA result(*this);
		for (int i = 0; i < storageSize; ++i) {
			result.storage[i] = ~(result.storage[i]);
		}
		return result;
	}
	RNA& operator=(const RNA& rvalue) {
		delete[] storage;
		storageSize = rvalue.storageSize;
		length = rvalue.length;
		storage = new unsigned int[storageSize];
		copyArray<unsigned int>(rvalue.storage, storage, storageSize);
		return *this;
	}
	RNA operator+(const RNA& rvalue) {
		RNA result(*this);
		result += rvalue;
		return result;
	}
	RNA& operator+=(const RNA& rvalue) {
		length += rvalue.length;
		for (int i = 0; i < rvalue.length; ++i) {
			(*this) += rvalue[i];
		}
		return *this;
	}
	RNA operator+(Nucleotide rvalue) {
		RNA result(*this);
		result += rvalue;
		return result;
	}
	RNA& operator+=(Nucleotide rvalue) {
		++length;
		fitSize();
		setNucleotide(length - 1, rvalue);
		return *this;
	}
	void trim(size_t newLength) {
		length = newLength;
		fitSize();
	}
	~RNA() {
		delete[] storage;
		storage = nullptr;
	}
};

class DNA {
public:
	const RNA rna1;
	const RNA rna2;
	DNA(const RNA& rna1, const RNA& rna2): rna1(rna1), rna2(rna2) {
		if (rna1 != ~rna2) {
			throw 1;
		}
	}
};
ostream& operator<<(ostream& os, const DNA& dna) {
	for (int i = 0; i < dna.rna1.length; ++i) {
		Nucleotide currentNucl = dna.rna1[i];
		char currentChar;
		switch (currentNucl) {
		case A:
			currentChar = 'A';
			break;
		case G:
			currentChar = 'G';
			break;
		case C:
			currentChar = 'C';
			break;
		case T:
			currentChar = 'U';
			break;
		}
		os << currentChar;
	}
	return os;
}
ostream& operator<<(ostream& os, const RNA& rna) {
	for (int i = 0; i < rna.length; ++i) {
		Nucleotide currentNucl = rna[i];
		char currentChar;
		switch (currentNucl) {
		case A:
			currentChar = 'A';
			break;
		case G:
			currentChar = 'G';
			break;
		case C:
			currentChar = 'C';
			break;
		case T:
			currentChar = 'T';
			break;
		}
		os << currentChar;
	}
	return os;
}

namespace testInfoNamespace {
	class TestEnvironment :public ::testing::Test {
	protected:
		RNA rna1;
		RNA rna2;
		TestEnvironment(): rna1(A, 1000), rna2(~rna1) {
		}
	};

	TEST_F(TestEnvironment, simpleTest) {
		bool caughtException = false;
		try {
			DNA dna = DNA(rna1, rna2);
		}
		catch (int){
			caughtException = true;
		}
		ASSERT_EQ(caughtException, false);
	}

	TEST_F(TestEnvironment, anotherSimpleTest) {
		bool caughtException = false;
		try {
			DNA dna = DNA(rna1, ~rna2);
		}
		catch (int){
			caughtException = true;
		}
		ASSERT_EQ(caughtException, true);
	}
}

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}