#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>

using namespace std;

#define HASHING_VAL_A 0xFEDCBA1234567890
#define HASHING_VAL_B 0x10A10B10C10D10E1

#define BLOCK_SIZE 16

vector<uint8_t> PadInput(const string &input) {
    size_t OriginalSize = input.size();
    size_t PaddingLen = BLOCK_SIZE - (OriginalSize % BLOCK_SIZE);

    vector<uint8_t> PaddedInput(OriginalSize + PaddingLen);
    memcpy(PaddedInput.data(), input.data(), OriginalSize);

    for(size_t i = OriginalSize; i < PaddedInput.size(); ++i) {
        PaddedInput[i] = static_cast<uint8_t>(PaddingLen);
    }

    return PaddedInput;
}


vector<uint8_t> Hashing(const vector<uint8_t> &input) {
    vector<uint8_t> Hashed;
    uint8_t A[8], B[8];

    size_t InputSize = input.size();

    for (size_t i = 0; i < InputSize; i += 16) {
        if (i + 16 <= InputSize) {
            memcpy(A, &input[i], 8);
            memcpy(B, &input[i + 8], 8);
            
            cout << "A: ";
            for (int j = 0; j < 8; ++j) {
                cout << (int)A[j] << " ";
            }

            cout << "\nB: ";
            for (int j = 0; j < 8; ++j) {
                cout << (int)B[j] << " ";
            }

            cout << endl << "------------------" << endl;
        }
    }

    return Hashed;
}

int main() {
    string text;
    vector<uint8_t> Padded;
    cout << "Enter text to hash: ";
    getline(cin, text);
   
    Padded = PadInput(text);
    Hashing(Padded);
    return 0;
}
