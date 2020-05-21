#ifndef PEKO_STRING_H
#define PEKO_STRING_H

#include <memory>


// ==================================== //

int cstring_length(const char* CStr) {
    int Length = 0;
    for (;;) {
        if (CStr[Length] == '\0') {
            ++Length;
            break;
        }
        ++Length;
    }
    return(Length);
}

// ==================================== //

#if defined(PEKO_STRING_C)

struct peko_string {
    char* Data;
    int Length;
    int AllocSize;
    
    // NOTE: To ensure proper use of uninitialized strings for copy_string
    peko_string() : Length(0), AllocSize(0), Data(nullptr) {}
};

// If Source is longer than Destination->AllocSize, resizes Destination
void copy_string(peko_string* Destination, const char* Source, int Length) {
    if (Destination->AllocSize < Length) {
        if (Destination->Data) {
            free(Destination->Data);
        }
        Destination->Data = (char*) malloc(sizeof(char) * Length);
        Destination->AllocSize = Length;
    }
    int i = 0;
    for (i;
         i <= Length;
         ++i) {
        Destination->Data[i] = Source[i];
        if (Source[i] == '\0') {
            ++i;
            break;
        }
    }
    Destination->Length = i;
}

void set_string(peko_string* Destination, const char* Source, int Length = 0) {
    int L = cstring_length(Source);
    copy_string(Destination, Source, (L >= Length) ? L : Length);
}

void set_string(peko_string* Destination, peko_string* Source) {
    set_string(Destination, Source->Data, Source->AllocSize);
}

void clear_string(peko_string* String) {
    if (!String) {
        return;
    }
    
    if (String->Data) {
        free(String->Data);
    }
    String->Data = nullptr;
    String->Length = 0;
    String->AllocSize = 0;
}

void reallocate_string(peko_string* Source, int AllocSize) {
    peko_string Temp;
    set_string(&Temp, Source->Data, AllocSize);
    clear_string(Source);
    Source->Data = Temp.Data;
    Source->AllocSize = AllocSize;
}

void trim_string(peko_string* Source) {
    if (!Source) {
        return;
    }
    
    if (Source->Length == 0) {
        return;
    }
    
    if (Source->Length == Source->AllocSize) {
        return;
    }
    
    int L = Source->Length;
    reallocate_string(Source, L);
    Source->Length = L;
}

void resize_string(peko_string* Source, int AllocSize) {
    if (AllocSize <= 0) {
        return;
    }
    
    int L = Source->Length;
    reallocate_string(Source, AllocSize);
    Source->Length = L;
    
    if (Source->Length > AllocSize) {
        Source->Length = AllocSize;
        Source->Data[AllocSize-1] = '\0';
    }
}

// Allocates a larger buffer at the Destination if required.
void concat_string(peko_string* Destination, const char* Second) {
    int SecondLength = cstring_length(Second);
    
    // NOTE: Length-1 due to needing the removal of \0 on first string
    int OneBeforeNullTerminator = Destination->Length-1;
    int CombinedLength = OneBeforeNullTerminator + SecondLength;
    if (Destination->AllocSize < CombinedLength) {
        reallocate_string(Destination, CombinedLength);
    }
    
    int Index = OneBeforeNullTerminator;
    for (int i = 0;
         i < SecondLength;
         ++i) {
        Destination->Data[Index++] = Second[i];
    }
    
    Destination->Length = CombinedLength;
}

// Allocates a larger buffer at the Destination if required.
void concat_string(peko_string* Destination, peko_string* Second) {
    concat_string(Destination, Second->Data);
}

// NOTE: Inclusive Start, Exclusive End (5->9 will do indices 5,6,7,8)
// End is NOT the Run of the amount of characters to search, but
// the final index. Returns -1 if it cannot find the Substring.
int search_string(peko_string* Source, char* Substring, int Start = 0, int End = 0) {
    int NotFound = -1;
    
    int Length = cstring_length(Substring);
    if (!Length || Length > Source->Length) {
        return(NotFound);
    }
    
    int CStringFinalIndex = Length-2; // Get rid of \0
    int FinalIndex = (End > 0) ? End : Source->Length;
    
    int Count = 0;
    int Iter = 0;
    for (int i = (Start > 0) ? Start : 0;
         i < FinalIndex;
         ++i) {
        if (Source->Data[i] == Substring[Iter]) {
            for (int Index = i;
                 Index < FinalIndex;
                 ++Index) {
                if (Count == CStringFinalIndex) {
                    return(i);
                }
                
                if (Source->Data[Index] != Substring[Iter]) {
                    Iter = 0;
                    Count = 0;
                    break;
                }
                
                ++Count;
                ++Iter;
            }
        }
    }
    
    return(NotFound);
}

// Substrings in place. Returns a pointer to the substringed Source->Data.
// NOTE: The resulting Substring will be Run+1 in length since it will add
// a null terminator
char* substring(peko_string* Source, int Begin, int Run) {
    if (Source->Length == 0) {
        return("");
    }
    if (Begin >= Source->Length || Begin < 1) {
        Begin = 0;
    }
    if (Run < 1) {
        return ("");
    }
    if (Run+Begin > Source->Length-1) {
        Run = Source->Length-1 - Begin;
    }
    
    char* Temp = (char*) malloc(sizeof(char) * Run+1);
    int Place = 0;
    for (Place;
         Place <= Run;
         ++Place) {
        Temp[Place] = Source->Data[Begin++];
    }
    Temp[Run] = '\0';
    clear_string(Source);
    set_string(Source, Temp);
    free(Temp);
    
    return(Source->Data);
}

#else // C++ style peko_string

struct peko_string {
    char* Data;
    int Length;
    int AllocSize;
    
    // NOTE: To ensure proper use of uninitialized strings for copy_string
    peko_string() : Length(0), AllocSize(0), Data(nullptr) {}
    peko_string(const char* Source, int Size = 0);
    peko_string(peko_string* Source, int Size = 0);
    
    ~peko_string();
    
    void set(const char* Source, int Length = 0);
    void set(peko_string* Source);
    void clear(void);
    void trim(void);
    void resize(int AllocSize);
    void concat(const char* Second);
    void concat(peko_string* Second);
    int search(char* Substring, int Start = 0, int End = 0);
    char* substring(int Begin, int Run);
    
    void operator=(const peko_string& RHS);
    void operator=(const char* RHS);
    
    private:
    void realloc(int AllocSize);
    void copy(const char* Source, int Length);
};


peko_string::peko_string(const char* Source, int Size)
: Data(nullptr), Length(0), AllocSize(0) {
    this->set(Source, Size);
}

// NOTE: Allocates buffer of Size, or Source->AllocSize if not specified
peko_string::peko_string(peko_string* Source, int Size)
: Data(nullptr), Length(0), AllocSize(0) {
    this->set(Source->Data, (Size > 0) ? Size: Source->AllocSize);
}

peko_string::~peko_string() {
    this->clear();
}

void peko_string::operator=(const peko_string& RHS) {
    this->set(RHS.Data);
}

void peko_string::operator=(const char* RHS) {
    this->set(RHS);
}

// If Source is longer than this->AllocSize, resizes this string
void peko_string::copy(const char* Source, int Length) {
    if (this->AllocSize < Length) {
        if (this->Data) {
            free(this->Data);
        }
        this->Data = (char*) malloc(sizeof(char) * Length);
        this->AllocSize = Length;
    }
    int i = 0;
    for (i;
         i <= Length;
         ++i) {
        this->Data[i] = Source[i];
        if (Source[i] == '\0') {
            ++i;
            break;
        }
    }
    this->Length = i;
}

void peko_string::set(const char* Source, int Length) {
    int L = cstring_length(Source);
    this->copy(Source, (L >= Length) ? L : Length);
}

void peko_string::set(peko_string* Source) {
    this->copy(Source->Data, Source->Length);
}

void peko_string::clear() {
    if (this->Data) {
        free(this->Data);
    }
    this->Data = nullptr;
    this->Length = 0;
    this->AllocSize = 0;
}

void peko_string::realloc(int AllocSize) {
    peko_string Temp(this->Data, AllocSize);
    this->clear();
    this->set(Temp.Data, AllocSize);
}

void peko_string::trim(void) {
    if (this->Length == 0) {
        return;
    }
    
    if (this->Length == this->AllocSize) {
        return;
    }
    
    int L = this->Length;
    this->realloc(L);
    this->Length = L;
}

void peko_string::resize(int AllocSize) {
    if (AllocSize <= 0) {
        return;
    }
    
    int L = this->Length;
    this->realloc(AllocSize);
    this->Length = L;
    
    if (this->Length > AllocSize) {
        this->Length = AllocSize;
        this->Data[AllocSize-1] = '\0';
    }
}

// Allocates a larger buffer if required.
void peko_string::concat(const char* Second) {
    int SecondLength = cstring_length(Second);
    
    // NOTE: Length-1 due to needing the removal of \0 on first string
    int OneBeforeNullTerminator = this->Length-1;
    int CombinedLength = OneBeforeNullTerminator + SecondLength;
    if (this->AllocSize < CombinedLength) {
        realloc(CombinedLength);
    }
    
    int Index = OneBeforeNullTerminator;
    for (int i = 0;
         i < SecondLength;
         ++i) {
        this->Data[Index++] = Second[i];
    }
    
    this->Length = CombinedLength;
}

// Allocates a larger buffer if required.
void peko_string::concat(peko_string* Second) {
    this->concat(Second->Data);
}

// NOTE: Inclusive Start, Exclusive End (5->9 will do indices 5,6,7,8)
// End is NOT the Run of the amount of characters to search, but
// the final index.
int peko_string::search(char* Substring, int Start, int End) {
    int NotFound = -1;
    
    int Length = cstring_length(Substring);
    if (!Length || Length > this->Length) {
        return(NotFound);
    }
    
    int CStringFinalIndex = Length-2; // Get rid of \0
    int FinalIndex = (End > 0) ? End : this->Length;
    
    int Count = 0;
    int Iter = 0;
    for (int i = (Start > 0) ? Start : 0;
         i < FinalIndex;
         ++i) {
        if (this->Data[i] == Substring[Iter]) {
            for (int Index = i;
                 Index < FinalIndex;
                 ++Index) {
                if (Count == CStringFinalIndex) {
                    return(i);
                }
                
                if (this->Data[Index] != Substring[Iter]) {
                    Iter = 0;
                    Count = 0;
                    break;
                }
                
                ++Count;
                ++Iter;
            }
        }
    }
    
    return(NotFound);
}

// Substrings in place. Returns a pointer to the substringed this->Data.
// NOTE: The resulting Substring will be Run+1 in length since it will add
// a null terminator
char* peko_string::substring(int Begin, int Run) {
    if (this->Length == 0) {
        return("");
    }
    if (Begin >= this->Length || Begin < 1) {
        Begin = 0;
    }
    if (Run < 1) {
        return("");
    }
    if (Run+Begin > this->Length-1) {
        Run = this->Length-1 - Begin;
    }
    
    char* Temp = (char*) malloc(sizeof(char) * Run+1);
    int Place = 0;
    for (Place;
         Place <= Run;
         ++Place) {
        Temp[Place] = this->Data[Begin++];
    }
    Temp[Run] = '\0';
    this->clear();
    this->set(Temp);
    free(Temp);
    
    return(this->Data);
}

#endif // PEKO_STRING_C

#endif //PEKO_STRING_H
