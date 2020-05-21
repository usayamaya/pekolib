# pekolib
General purpose single-header C++ library for assorted functionality and types. Some types include both a C-style and C++ style api.

## peko_string
peko_string is a string type that wraps a char* so that the character data is mutable, while maintaining ease of use you may expect from a higher level string library.

```cpp
struct peko_string {
  char* Data;
  int Length;
  int AllocSize;
}
```

Choose between C and C++ style. Defining PEKO_STRING_C before including peko_string.h will set it to use the C api.

```cpp
#define PEKO_STRING_C
#include "peko_string.h"
```

#### C Style example
```cpp
peko_string First;
set_string(&First, "SomeValue"); // First will have an allocated buffer that fits the size of the string automatically

peko_string Second;
set_string(&Second, &First, 100); // Copies First to Second, Second will have 100 character buffer

concat_string(&Second, &First);
resize_string(&First, 50);
trim_string(&Second);            // Reduces the buffer to the string length
search_string(&First, "Val", 2, 8);
substring(&Second, 0, 4);
clear_string(&First);
```

#### C++ Style example
The C++ style allows for setting the data with operator= and through the constructor, it will internally call .set() on itself.
Due to the destructor, char* Data will automatically free() when the peko_string goes out of scope.

```cpp
peko_string First;
First.set("SomeValue");

peko_string Second("Some Other Value");

// This does not reallocate for Second unless First is larger, it merely copies the characters into Second's buffer
Second = First;

First.concat(&Second);
Second.resize(256);
Second.trim();
First.search("Val", 2, 8);
First = Second.substring(5, Second.Length);
First.clear();
```

Since the peko_string.Data is mutable, you can access it like a normal C array as well
```cpp
peko_string Str("Value");
Str.Data[0] = 'W';
char C = Str.Data[3];
```
