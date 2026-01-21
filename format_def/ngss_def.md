# introduction
ngss or nonserialized generic storage system is a file format and structure defined to be a riged but highly readable and modular way of defining arbatrary files and data in a human readable formst
this format is specifically designed for game development as a similar system to modern json or godot .tscn files but in a more human writable way. its closest cousin is open ddl and minecraft nbt formats as both were used as partial inspiration for the design  

the format defined by ngss is designed to be extremely easy to convert and parse to binary and will map directly to whatever header is given


# basic syntax

# keywords and macros

## core keywords




## macros

these macros may be defined within the header as the following
```
#define
#include
#pragma 
```

these macros may be defined in the data section

```
#define "header.ngsh"
#pragma 
```

only one header maay be defined if used in data section 
include is not supported in this mode 


### pragma operators
TBA

# types 


- str
  - a static fixed sized byte string that is resolved at compile time
- dstr
  - a dynamicly sized byte string that is resolved at compile time 
- char
   - considered to be a singe character, may not be interchangable with u8 and s8   
- file
  - used to import binary blobs directly into the structure as files
- fileptr
  - stores a reference to a file rather than embedding the file itself 
- binary
  - a little endian blob that may be defined by either base64 or direct binary if nessisary  
- wstr
  - a static fixed sized short string that is resolved at compile time
- wchar
  - considered to be a single 2 byte character, not interchangable with s16 and u16


|   | u | s | f |
|---|---|---|---|
| 8 | ✔️|✔️| ❌| 
| 16 | ✔️|✔️| ✔️| 
| 32 | ✔️|✔️| ✔️| 
| 64 | ✔️|✔️| ✔️| 


# nonserialized mode 

## header
ngss files may optionally start with a header defined by the following

```c
#define header

```

at compiletime, the macro will split the file into the according serialized map of data the data structures stored iwthin the file

data cannot exist within header space, anything called within header must be a defined struct or enum

the header may include other ngsh files defined as the following

```c
#define header

#include "file.ngsh
```

this include syntax is only valid for specifically ngsh files, other files will not be valid


### struct and enum

ngss headers are used to define c like structs and c++ like enums with the following syntax

```c
#define header

struct data
{
  i32 data1
  f32 data2
}
```

these may only exist within the header and cannot be defined in data section
they are only as both syntactical sugar for memory placement and for interfacing with programming language structs directly






