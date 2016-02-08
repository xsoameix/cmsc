# Function Name Format #

All functions exported for third-party use in wzlibc (see wzlibc.h) follow the same general naming format.

**Format**:
```
[_]WZLib_[Structure]_[Operation](...);
```
> where `[Structure]` is one of the structs defined in wzlibc.h, and `[Operation]` is the specific operation to be performed on the structure.  If a leading underscore `[_]` is present in the function name, then this function is intended only for internal use by wzlibc.

Some Operations appear quite a bit:
|`Init`|Fills the structure with default values (assumes the memory for this structure has already been set to '\0').|
|:-----|:------------------------------------------------------------------------------------------------------------|
|`Parse`|Reads the structure info from file (if necessary), and makes the structure ready for use.                    |
|`Free` or `Delete`|Frees any memory held by the structure that cannot be directly `free`'d.                                     |



---