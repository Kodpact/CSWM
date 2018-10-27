#ifndef INCLUDE_FORCETYPE_H
#define INCLUDE_FORCETYPE_H

enum FORCE_TYPE
{
	force_exactfile,					// File on client must exactly match server's file
	force_model_samebounds,				// For model files only, the geometry must fit in the same bbox
	force_model_specifybounds,			// For model files only, the geometry must fit in the specified bbox
	force_model_specifybounds_if_avail,	// For Steam model files only, the geometry must fit in the specified bbox (if the file is available)
};

#endif