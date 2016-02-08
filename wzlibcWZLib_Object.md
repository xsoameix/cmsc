`WZLib_Object` is the ancestor structure for everything in wzlibc.

# Declaration: #
```
typedef struct _struct_WZLib_Object{
        struct _struct_WZLib_Object*    parent;
        char*                           name;
        WZLib_ObjectType                type;
        int                             parsed;
        int                             blockSize;
        int                             checksum;
        unsigned int                    offset;
        unsigned int                    objectID;
        unsigned int                    refCount;
}WZLib_Object;
```
|**Member**|**Description**|
|:---------|:--------------|
|`parent`  |If this object has any kind of parent, it will be pointed to here.|
|`name`    |The name of this object.|
|`type`    |The type of this object (see wzlibcWZLib\_ObjectType).|
|`parsed`  |A boolean that indicates whether this object has been read from file yet.|
|`blockSize`|The number of bytes this object occupies in the file.|
|`checksum`|Add all the bytes together, and you get a checksum! (not implemented yet)|
|`offset`  |The location of this object's data in the file.|
|`objectID`|An integer identifying this object (for use in debugging).|
|`refCount`|A member for reference counting, if you want to use it.|

# Functions #

## WZLib\_Object\_GetSize ##
**Declaration:**<br>
<code>int             WZLib_Object_GetSize(WZLib_Object* obj);</code><br><br>
If this object has children, get the number of children it has.<br>
<h2>WZLib_Object_Get_n</h2>
<b>Declaration:</b><br>
<code>WZLib_Object*   WZLib_Object_Get_n(WZLib_Object* obj,const char* name);</code><br><br>
If this object has children, return the child with the name matching <code>name</code> (case sensitive).<br>
<h2>WZLib_Object_Get_nv</h2>
<b>Declaration:</b><br>
<code>WZLib_Object*   WZLib_Object_Get_nv(WZLib_Object* obj,...);</code><br><br>
If this object has children, crawl down the inheritance list by repeated application of <code>WZLib_Object_Get_n</code>.  Uses varargs; when your list of children is done, terminate with either a <code>""</code> or a <code>NULL</code>.  If there is a discontinuity anywhere down the chain, this function will return NULL.<br>
<h2>WZLib_Object_Get_i</h2>
<b>Declaration:</b><br>
<code>WZLib_Object*   WZLib_Object_Get_i(WZLib_Object* obj,int index);</code><br><br>
If this object has children, return the <code>index</code>'th child.<br>
<h2>WZLib_Object_Get_iv</h2>
<b>Declaration:</b><br>
<code>WZLib_Object*   WZLib_Object_Get_iv(WZLib_Object* obj,...);</code><br><br>
If this object has children, crawl down the inheritance list by repeated application of <code>WZLib_Object_Get_i</code>.  Uses varargs, when your list of children is done, terminate with a <code>-1</code>.  If there is a discontinuity anywhere down the chain, this function will return NULL.