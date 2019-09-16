#include <iostream>
using namespace std;

enum Nucleotide { A, G, C, T };

unsigned int mask = 3;

class RNA {
private:
	size_t capacity;
	size_t length;
	unsigned int* storage;
	void resize(size_t newSize) {
		unsigned int* newStorage = new unsigned int[newSize];
		for (int i = 0; i<capacity && i<newSize; ++i) {
			newStorage[i] = storage[i];
		}
		delete[] storage;
		capacity = newSize;
		storage = newStorage;
	}
	class StorageAccessor {
	private:
		RNA* proprietor;
		size_t nucleotideIndex;
		size_t bitPackIndex;
		size_t bitPairIndex;
	public:
		void setNewIndex(size_t newIndex) {
			nucleotideIndex = newIndex;
			bitPackIndex = nucleotideIndex / (4 * sizeof(unsigned int));
			bitPairIndex = nucleotideIndex % (4 * sizeof(unsigned int));
		}
		StorageAccessor(RNA* proprietor, size_t nucleotideIndex) : proprietor(proprietor) {
			setNewIndex(nucleotideIndex);
		}
		operator Nucleotide () {
			unsigned int bitPack = proprietor->storage[bitPackIndex];
			return Nucleotide((bitPack >> bitPairIndex)&mask);
		}
		Nucleotide operator= (Nucleotide newValue) {
			unsigned int bitPack = proprietor->storage[bitPackIndex];
			proprietor->storage[bitPackIndex] = (bitPack&(~(mask << bitPairIndex))) | ((unsigned int(newValue)) << bitPairIndex);
			return newValue;
		}
	};
public:
	RNA() : capacity(1000), length(0), storage(new unsigned int[1000]) {}
	RNA(Nucleotide filler, int fillLength) : length(fillLength) {
		capacity = fillLength / sizeof(int) + (fillLength % sizeof(int) == 0 ? 0 : 1);
		storage = new unsigned int[capacity];
		StorageAccessor iterator(this, 0);
		for (int i = 0; i < fillLength; iterator.setNewIndex(++i)) {
			iterator = filler;
		}
	}
	StorageAccessor operator[] (size_t nucleotideIndex) {
		return StorageAccessor(this, nucleotideIndex);
	}
	bool operator== (const RNA& rvalue) {
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
	//operator~
	//operator=
	//isComplementary
	//~RNA
};

int main() {
	return 0;
}