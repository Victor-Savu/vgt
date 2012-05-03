#ifndef VGT_VOLUMETRIC_DATA_H
#define VGT_VOLUMETRIC_DATA_H

#include <vgt/types.h>



/*
   Allocates memory for a struct VolumetricData and fills the contents according to
   the data from @filename. If @filename is a null pointer, the structure is
   filled with 0 values.

   The method returns a pointer to the struct VolumetricData on success and a null
   pointer on error.
*/
VolumetricData vdCreate(const char* filename);

VolumetricData vdCopy(VolumetricData v);

/*
   Clears the contents of a struct VolumetricData pointed by @v while freeing the
   memory from every field. The structure is filled with 0 values.
*/
void vdClear(VolumetricData v);

/*
   Reads the configuration file @filename and initialises the struct VolumetricData
   pointed by @f. The structure is cleared before the process starts using
   @vdClear.

   In case of error, the method returns 0, otherwise it returns 1.
*/
bool vdRead(VolumetricData v, FILE* fin, const char* filename);

/*
   Clears the field and releases the allocated memory.
*/
void vdDestroy(VolumetricData v);

#endif//VGT_VOLUMETRIC_DATA_H
