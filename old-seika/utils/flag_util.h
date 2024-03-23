#pragma once

// Contains flag macros...

// Check if 'FLAGS' contains all the flags specified in 'REQUIRED_FLAGS'
#define SKA_FLAG_CONTAINS(FLAGS, REQUIRED_FLAGS) (((FLAGS) & (REQUIRED_FLAGS)) == (REQUIRED_FLAGS))

// Check if 'FLAGS' contains any of the flags specified in 'REQUIRED_FLAGS'
#define HAS_ANY_FLAG(FLAGS, REQUIRED_FLAGS) ((FLAGS) & (REQUIRED_FLAGS))

// Check if 'FLAGS' contains none of the flags specified in 'FORBIDDEN_FLAGS'
#define HAS_NO_FLAG(FLAGS, FORBIDDEN_FLAGS) (((FLAGS) & (FORBIDDEN_FLAGS)) == 0)

// Check if 'FLAGS' contains exactly one of the flags specified in 'REQUIRED_FLAGS'
#define HAS_EXACTLY_ONE_FLAG(FLAGS, REQUIRED_FLAGS) (HAS_ALL_FLAGS(FLAGS, REQUIRED_FLAGS) && ((FLAGS) != 0))

// Check if 'FLAGS' contains none of the flags specified in 'REQUIRED_FLAGS'
#define HAS_NONE_OF_FLAGS(FLAGS, REQUIRED_FLAGS) (((FLAGS) & (REQUIRED_FLAGS)) == 0)
