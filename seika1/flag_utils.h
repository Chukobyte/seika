#pragma once

// Contains flag macros...

// Check if 'FLAGS' contains all the flags specified in 'REQUIRED_FLAGS'
#define SKA_FLAG_CONTAINS(FLAGS, REQUIRED_FLAGS) (((FLAGS) & (REQUIRED_FLAGS)) == (REQUIRED_FLAGS))

// Check if 'FLAG' contains any of the flags specified in 'REQUIRED_FLAGS'
#define SKA_HAS_FLAG(FLAG, REQUIRED_FLAGS) ((FLAG) & (REQUIRED_FLAGS))

// Check if 'FLAGS' contains none of the flags specified in 'FORBIDDEN_FLAGS'
#define SKA_HAS_NO_FLAG(FLAGS, FORBIDDEN_FLAGS) (((FLAGS) & (FORBIDDEN_FLAGS)) == 0)

// Check if 'FLAGS' contains exactly one of the flags specified in 'REQUIRED_FLAGS'
#define SKA_HAS_EXACTLY_ONE_FLAG(FLAGS, REQUIRED_FLAGS) (HAS_ALL_FLAGS(FLAGS, REQUIRED_FLAGS) && ((FLAGS) != 0))

// Check if 'FLAGS' contains none of the flags specified in 'REQUIRED_FLAGS'
#define SKA_HAS_NONE_OF_FLAGS(FLAGS, REQUIRED_FLAGS) (((FLAGS) & (REQUIRED_FLAGS)) == 0)

// Add 'NEW_FLAGS' to 'FLAGS'
#define SKA_ADD_FLAGS(FLAGS, NEW_FLAGS) ((FLAGS) |= (NEW_FLAGS))

// Remove 'REMOVE_FLAGS' from 'FLAGS'
#define SKA_REMOVE_FLAGS(FLAGS, REMOVE_FLAGS) ((FLAGS) &= ~(REMOVE_FLAGS))
