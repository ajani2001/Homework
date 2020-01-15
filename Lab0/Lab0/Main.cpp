#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

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

bool isEqualHeapStates(const _CrtMemState& memstate1, const _CrtMemState& memstate2){
	return(memstate1.lCounts[1] == memstate2.lCounts[1] && memstate1.lSizes[1] == memstate2.lSizes[1]);
}

class RNA {
private:
	size_t length;
	size_t storageSize;
	unsigned int* storage;

	void resize(size_t newSize) {
		if (newSize == storageSize){
			return;
		}
		unsigned int* newStorage = new unsigned int[newSize];
		copyArray<unsigned int>(storage, newStorage, storageSize < newSize ? storageSize : newSize);
		delete[] storage;
		storageSize = newSize;
		storage = newStorage;
	}

	void fitSize() {
		size_t bytesRequired = (length + 3) / 4;
		size_t bytesAllocated = storageSize * sizeof(unsigned int);
		if (bytesRequired + 100 >= bytesAllocated) {
			resize((bytesRequired + 100) + 1000 - (bytesRequired + 100) % 1000);
			return;
		}
		if (bytesRequired + 1900 <= bytesAllocated) {
			resize(bytesRequired + 1000 - bytesRequired % 1000);
			return;
		}
	}

	Nucleotide getNucleotide(size_t nucleotideIndex) const {
		if (nucleotideIndex >= length) {
			throw 1;
		}
		size_t bitPackIndex = nucleotideIndex / (4 * sizeof(unsigned int));
		size_t bitPairIndex = nucleotideIndex % (4 * sizeof(unsigned int));
		unsigned int bitPack = storage[bitPackIndex];
		return Nucleotide((bitPack >> (2*bitPairIndex))&mask);
	}

	void setNucleotide(size_t nucleotideIndex, Nucleotide newValue) {
		if (nucleotideIndex >= length) {
			throw 1;
		}
		size_t bitPackIndex = nucleotideIndex / (4 * sizeof(unsigned int));
		size_t bitPairIndex = nucleotideIndex % (4 * sizeof(unsigned int));
		unsigned int bitPack = storage[bitPackIndex];
		storage[bitPackIndex] = (bitPack&(~(mask << (2*bitPairIndex)))) | ((unsigned int(newValue)) << (2*bitPairIndex));
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
	size_t getLength() const {
		return length;
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
	RNA operator+(const RNA& rvalue) const {
		RNA result(*this);
		result += rvalue;
		return result;
	}
	RNA& operator+=(const RNA& rvalue) {
		for (int i = 0; i < rvalue.length; ++i) {
			(*this) += rvalue[i];
		}
		return *this;
	}
	RNA operator+(Nucleotide rvalue) const {
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
	for (int i = 0; i < dna.rna1.getLength(); ++i) {
		switch (dna.rna1[i]) {
		case A:
			os << "A-<>-U" << endl;
			break;
		case G:
			os << "G-<>-C" << endl;
			break;
		case C:
			os << "C-<>-G" << endl;
			break;
		case T:
			os << "U-<>-A" << endl;
			break;
		}
	}
	return os;
}
ostream& operator<<(ostream& os, const RNA& rna) {
	for (int i = 0; i < rna.getLength(); ++i) {
		char currentChar;
		switch (rna[i]) {
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

namespace testingNamespace {
	class RNATestEnvironment :public ::testing::Test {
	protected:
		RNA dummy;
		RNATestEnvironment() : dummy(C, 1000) {}
		~RNATestEnvironment() {}
	};
	class DNATestEnvironment :public ::testing::Test {
	protected:
		RNA rna1;
		RNA rna2;
		DNATestEnvironment(): rna1(A, 1000), rna2(~rna1) {}
		~DNATestEnvironment() {}
	};
	
	TEST_F(DNATestEnvironment, succeedIfComplementary) {
		ASSERT_EQ(rna1, ~rna2);
		ASSERT_NO_THROW(DNA(rna1, rna2));
	}
	TEST_F(DNATestEnvironment, exceptionIfNotComplementary) {
		rna2[0] = Nucleotide((int(rna2[0]) + 1) % 4);
		ASSERT_NE(rna1, ~rna2);
		ASSERT_THROW(DNA(rna1, rna2), int);
	}

	TEST_F(RNATestEnvironment, defaultConstructorTest){
		RNA defaultRNA;
		ASSERT_EQ(defaultRNA.getLength(), 0);
	}
	TEST_F(RNATestEnvironment, fillConstructorTest){
		int fillNumber = 1234;
		Nucleotide filler = A;
		RNA filledRNA(filler, fillNumber);
		ASSERT_EQ(filledRNA.getLength(), fillNumber);
		for (int i = 0; i < fillNumber; ++i){
			ASSERT_EQ(filledRNA[i], filler);
		}
	}
	TEST_F(RNATestEnvironment, copyConstructorIsCorrect){
		RNA newRna = RNA(dummy);
		ASSERT_EQ(newRna, dummy);
		for (int i = 0; i < dummy.getLength(); ++i){
			ASSERT_EQ(newRna[i], dummy[i]);
		}
		ASSERT_EQ(dummy.getLength(), newRna.getLength());
		ASSERT_THROW(newRna[newRna.getLength()].operator Nucleotide(), int);
		ASSERT_THROW(dummy[dummy.getLength()].operator Nucleotide(), int);
	}
	TEST_F(RNATestEnvironment, assignmentOperatorIsCorrect){
		RNA newRna = dummy;
		ASSERT_EQ(newRna, dummy);
		for (int i = 0; i < dummy.getLength(); ++i){
			ASSERT_EQ(newRna[i], dummy[i]);
		}
		ASSERT_EQ(dummy.getLength(), newRna.getLength());
		ASSERT_THROW(newRna[newRna.getLength()].operator Nucleotide(), int);
		ASSERT_THROW(dummy[dummy.getLength()].operator Nucleotide(), int);
	}
	TEST_F(RNATestEnvironment, arrayBehaviourTest){
		RNA ARna(A, 1000);
		ASSERT_THROW(ARna[1000].operator Nucleotide(), int);
		ASSERT_EQ(ARna[999], A);
		ARna[999] = G;
		ASSERT_EQ(ARna[999], G);
	}
	TEST_F(RNATestEnvironment, comparisonOperatorTest){
		RNA rna(dummy);
		ASSERT_EQ(rna, dummy);
		rna[rna.getLength() - 1] = Nucleotide((int(rna[rna.getLength() - 1]) + 1) % 4);
		ASSERT_NE(rna, dummy);
	}
	TEST_F(RNATestEnvironment, complementaryOperatorTest){
		RNA complementaryRNA(~dummy);
		for (int i = 0; i < complementaryRNA.getLength(); ++i){
			switch (complementaryRNA[i]){
			case A:
				ASSERT_EQ(dummy[i], T);
				break;
			case G:
				ASSERT_EQ(dummy[i], C);
				break;
			case C:
				ASSERT_EQ(dummy[i], G);
				break;
			case T:
				ASSERT_EQ(dummy[i], A);
				break;
			}
		}
	}
	TEST_F(RNATestEnvironment, sumOperatorsTest){
		RNA newRna(A, 1000);
		const RNA newRna2(newRna);
		newRna += dummy;
		ASSERT_EQ(newRna.getLength(), dummy.getLength() + 1000);
		ASSERT_EQ(newRna, newRna2 + dummy);
		for (int i = 0; i < 1000; ++i){
			ASSERT_EQ(newRna[i], A);
		}
		for (int i = 1000; i < newRna.getLength(); ++i){
			ASSERT_EQ(newRna[i], dummy[i - 1000]);
		}
	}
	TEST_F(RNATestEnvironment, trimTest){
		RNA newRna(A, 10000);
		ASSERT_GT(newRna.getLength(), 5);
		newRna.trim(5);
		ASSERT_EQ(newRna.getLength(), 5);
	}
	TEST_F(RNATestEnvironment, memoryLeaksTest){
		RNA* rna1;
		_CrtMemState memState1, memState2;
		_CrtMemCheckpoint(&memState1);
		rna1 = new RNA(dummy);
		(*rna1) += dummy;
		(*rna1) += dummy;
		(*rna1).trim(10);
		(*rna1)[9] = T;
		for (int i = 0; i < 1000000; ++i){
			(*rna1) += Nucleotide(i % 4);
		}
		_CrtMemCheckpoint(&memState2);
		ASSERT_FALSE(isEqualHeapStates(memState1, memState2));
		delete rna1;
		_CrtMemCheckpoint(&memState2);
		ASSERT_TRUE(isEqualHeapStates(memState1, memState2));
	}
}


int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();
	RNA dummy2;
	for (int i = 0; i < 100; ++i){
		dummy2 += Nucleotide(i % 4);
	}
	DNA dummy3(dummy2, ~dummy2);
	cout << "Length: " << dummy2.getLength() << endl;
	cout << "Sequence: " << dummy2 << endl;
	cout << "DNA:" << endl << dummy3;
	int dummy;
	cin >> dummy;
	return result;
}