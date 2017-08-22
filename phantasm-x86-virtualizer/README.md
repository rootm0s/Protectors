# x86 virtualizer
This is a simple virtualization-based obfuscator for x86 code. 

A quick run-down of how it works:
- Each input instruction (native, x86) is assigned one or more bytecode instructions mapped to handlers implemented in the VM.
- Bytecode instruction set is intended to be RISC and very stack-oriented
- Input instructions are thus if needed broken down into more atomical operations. For instance, instructions using scaled addressing (i.e. has an operand in the form [base_reg+index_reg*scalar+displacement]) are converted. The value of the operands of such an instruction are calculated at run-time using a sequence of virtual instructions. Move instructions are all push and pops.
- Many instructions share a common handler. For instance, the bitwise logical operations are all implemented using a single NAND handler (they all set the same flags, just use DeMorgan's laws to change things around). Likewise, CMP can be implemented as a SUB where the result (bar the flags) is discarded.
- Instructions that do not have an implementation in the VM are executed as-is in an executable buffer by doing a temporary context switch out of the VM. Certain instructions cannot be executed in this manner (any instruction changing the IP) and most of these have implementations.
- Jumps and calls need special treatment depending on whether the destination is inside a virtualized region or not.
- The destinations for indirect absolute jumps and calls (i.e. through a register or value at address) are not inferred by the virtualizer and must be manually specified by the user.

## Example usage
### Protecting the program
The VM implementation is either included in the target program by statically linking to it, or by having the virtualizer manually copy it into the executable. The code to be protected is either specified as input arguments or by using the markers defined in the header file. Using the input arguments, the regions to protect are specified by their starting virtual address (in the executable's preferred base) and the length in bytes. Using the markers, they are to be placed within a function: 

```c++
#include "libphant.h"
int main(int argc, const char *argv[])
{
    BeginProtect;
    // code to protect here
    EndProtect;
}
```
Markers need to appear in pairs and may not be nested (could accidentally happen if a function called inside a protected area is instead inlined by the compiler).

See a more complete example [here](example.md) (called "SDK example two" in the source).

## Command-line arguments
```
input_file="path to the input exe"
```
Path to input filename
```
output_file="path to the output exe"
```
Path to output file. Will be created/overwritten. 
```
va_target="virtual address"
```
Inform of a VA used in an absolute jump or call leading inside a virtualized region. Hexadecimal in preferred base, no prefix or suffix.
```
unfold_constants="<0/1>"
```
Simple unfolding of constants (as n=q*d+r)

```
swap_registers="<0/1>"
```
Adds prologue/epilogue code to swap every register in the context structure.

## Needed improvements
Just a few glaring ones
- Control flow obfuscation. Using the VM IP the control flow can easily be mapped out in order to defeat protection against patching etc. with little hassle.
- Obufscation of the VM itself. Handlers are easy to map to each instructions, and the meaning of each handler is easy to figure out.
- Relocatable executable. Relocations are currently not implemented. There are some stubs for this.
- Proper register swapping interleaved with the rest of the code
