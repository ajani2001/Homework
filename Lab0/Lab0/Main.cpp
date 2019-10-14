#include <iostream>
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
	size_t capacity;
	size_t length;
	unsigned int* storage;
	void resize(size_t newSize) {
		unsigned int* newStorage = new unsigned int[newSize];
		copyArray<unsigned int>(storage, newStorage, capacity < newSize ? capacity : newSize);
		delete[] storage;
		capacity = newSize;
		storage = newStorage;
	}

	class StorageAccessorBase {
	protected:
		size_t nucleotideIndex;
		size_t bitPackIndex;
		size_t bitPairIndex;
	public:
		void setNewIndex(size_t newIndex) {
			nucleotideIndex = newIndex;
			bitPackIndex = nucleotideIndex / (4 * sizeof(unsigned int));
			bitPairIndex = nucleotideIndex % (4 * sizeof(unsigned int));
		}
		virtual operator Nucleotide () const = 0; // not good
	};
	class StorageAccessor: public StorageAccessorBase {
	private:
		RNA* proprietor;
	public:
		StorageAccessor(RNA* proprietor, size_t nucleotideIndex) : proprietor(proprietor) {
			setNewIndex(nucleotideIndex);
		}
		operator Nucleotide () const {
			unsigned int bitPack = proprietor->storage[bitPackIndex];
			return Nucleotide((bitPack >> bitPairIndex)&mask);
		}
		Nucleotide operator= (Nucleotide newValue) const {
			unsigned int bitPack = proprietor->storage[bitPackIndex];
			proprietor->storage[bitPackIndex] = (bitPack&(~(mask << bitPairIndex))) | ((unsigned int(newValue)) << bitPairIndex);
			return newValue;
		}
	};
	class StorageAccessorConst : public StorageAccessorBase {
	private:
		const RNA* proprietor;
	public:
		StorageAccessorConst(const RNA* proprietor, size_t nucleotideIndex) : proprietor(proprietor) {
			setNewIndex(nucleotideIndex);
		}
		operator Nucleotide () const {
			unsigned int bitPack = proprietor->storage[bitPackIndex];
			return Nucleotide((bitPack >> bitPairIndex)&mask);
		}
	};

public:
	RNA() : capacity(1000), length(0), storage(new unsigned int[1000]) {}
	RNA(const RNA& rna): capacity(rna.capacity), length(rna.length) {
		storage = new unsigned int[rna.capacity];
		copyArray<unsigned int>(rna.storage, storage, capacity);
	}
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
	StorageAccessorConst operator[] (size_t nucleotideIndex) const {
		return StorageAccessorConst(this, nucleotideIndex);
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
	RNA& operator~() const { // dont understand
		RNA result(*this);
		for (int i = 0; i < capacity; ++i) {
			result.storage[i] = ~(result.storage[i]);
		}
		return result; // is this legal?
	}
	RNA& operator=(const RNA& rvalue) {
		delete[] storage;
		capacity = rvalue.capacity;
		length = rvalue.length;
		storage = new unsigned int[capacity];
		copyArray<unsigned int>(rvalue.storage, storage, capacity);
		return *this;
	}
	RNA& operator+(const RNA& rvalue);
	RNA& operator+=(const RNA& rvalue);
	void trim(size_t length);

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

int main() {
	return 0;
}