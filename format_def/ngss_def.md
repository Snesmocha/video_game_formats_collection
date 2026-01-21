# introduction
ngss or nonserialized generic storage system is a file format and structure defined to be a riged but highly readable and modular way of defining arbatrary files and data in a human readable formst
this format is specifically designed for game development as a similar system to modern json or godot .tscn files but in a more human writable way. its closest cousin is open ddl and minecraft nbt formats as both were used as partial inspiration for the design  

the format defined by ngss is designed to be extremely easy to convert and parse to binary and will map directly to whatever header is given


# basic syntax

array specifier \[\]

must be static


# keywords and macros

## core keywords

### header specific

- enum
- struct

### data specific

- region
  - dynamic
  - large

### macros

these macros may be defined within the header as the following
```c
#define
#include
#pragma 
```

these macros may be defined in the data section

```c
#include "header.ngsh"
#define
#pragma 
```

only one header maay be defined if used in data section 


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

an enum may be defined as the following
```c
#define header

enum selection
{
  A,
  B,
  C
}
```

enums when untyped will automatically be set to i32
when an enum is defined the first value will automatically equal zero and increment if set to an integer or floating data type


enums may be typed with the folowing syntax


```c
#define header

enum selection : f32
{
  A,
  B,
  C
}
```

if an enum is defined as a string, the enum values **MUST** be explicitly assigned



```c
#define header

enum selection : str
{
  A = "hello world",
  B = "abc",
  C - "test",
}
```

## data


if a header is defined, in order to begin a data asection, the following macro must be defined

```c
#define data
```

if no header is defined then an include must occur for defining header info
```c
#include "file.ngsh"
```

### storage

data may be stored anywhere within this format but it is recomended you store data in regions

data declared outside of it will be declared in global regions

core data does not need to be initialized

```c
i32 a 
i32 b
```

a piece of data is allowed to have a default value assigned



```c
i32 a = 3 
i32 b = 5
```

string sizes are to be resolved at compile time

```c
str a = "hello world"
```

if a string needs to be dynamic in size, an additional 32-64 elements may be assigned 


### regions

regions may be defined by the region keyword and a specifier or implicitly defined

regions cannot be recursive

```c
region region_name
{
  str data
  float data
}
```

as specified before, all data must be encapsolated within region

if outside of a regiono specifier, it will be added to a global region

```
f32 data

region store
{
  f32 b
}
```
data will be hoisted to global region



if a region seperataes between data
```
f32 data

region store
{
  f32 b
}
f32 data2

```
f32 data2 will be placed into a second global rather than a singular global

because of this, it is best practice to store as much into regions as needed 


regions may contain additional prefix keywords that will tag the data as needed

```
dynamic region store
{
  f32 b
}

large region store
{
  f32 b
}

```

#### dynamic

must be placed at the end of a file
meant for dynamic file types

more to be specified cause i don't know what else to do

#### large
large specifies that the blob may be a large piece of data, such as possibly a binary blob and should be treated with additional care in import


# serialized mode







