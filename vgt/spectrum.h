#ifndef VGT_SPECTRUM_H
#define VGT_SPECTRUM_H

#include <vgt/types.h>

// Creates an instance of a Spectrum over a volumetric dataset.
Spectrum specCreate(const char* restrict conf);

// Destroys a spectrum structure.
void specDestroy(Spectrum restrict sp);

// Snaps the active threads' endpoints to the spectrum's
// current snap isovalue
void specSnap(Spectrum restrict sp);

// Relaxes the front mesh to improve triangle quality
void specRelax(Spectrum restrict sp);

// Simplifies the front mesh by eliminating small triangles
void specSimplify(Spectrum restrict sp);

// Refines the front mesh by subdividing large triangles
void specRefine(Spectrum restrict sp);

// Projects the front mesh onto the next isosurface, where possible
bool specProject(Spectrum restrict sp);

// Runs marching tets on the volume
// where the projection could not occur
void specSample(Spectrum restrict sp);

// Merges the results of the projection and of the the sampling
void specMerge(Spectrum restrict sp);

void specDisplay(Spectrum restrict sp);

void specStats(Spectrum restrict s, FILE* f);

#endif//VGT_SPECTRUM_H

