/**
 * @file   constants.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.02.05
 *
 * @brief  Contains global constant definitions.
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

/** The configuration file name. */
static const char* configFileName = "patternsConfig.xml";
/** Use a timestamp for the log files. */
static bool LOG_USE_TIMESTAMPS = false;
/** The OpenGL major version used. */
static unsigned int PTRN_OPENGL_MAJOR_VERSION = 3;
/** The OpenGL minor version used. */
static unsigned int PTRN_OPENGL_MINOR_VERSION = 3;

/** The font program resource id. */
static const char* fontProgramID = "shader/gui/renderText.vp|shader/gui/renderText.gp|shader/gui/renderText.fp";

/** Uniform buffer block name of the font metrics buffer. */
static const char* fontMetricsUBBName = "fontMetrics";
static const char* orthoProjectionUBBName = "orthoProjection";
static const char* perspectiveProjectionUBBName = "perspectiveTransform";

/** Holds the number of buffers used for dynamic buffering. */
static unsigned int NUM_DYN_BUFFERS = 5;
/** Holds the timeout to wait for asynchronous buffers. */
static GLuint64 ASYNC_TIMEOUT = 3000000;

#endif /* CONSTANTS_H */
